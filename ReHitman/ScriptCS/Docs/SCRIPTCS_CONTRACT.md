# ScriptCS — Module Layout & Script-DLL Binary Contract

Scope: `ReHitman/ScriptCS/` (per-script DLL workspace, C-language build tree)
and the **PC_Hideout** reference binary `Hideout.dll`
(`.../Games/Scriptcs/_gamerelease/Hideout.dll`) — the compiled Hideout-level
script module the engine attaches at runtime.

> **Instance binding:** `ScriptCS/Hideout/` ⇄ **PC_Hideout** IDA instance.
> All reversing, verification, and diffing of the `Hideout` target goes
> through PC_Hideout. Additional per-script directories added later follow the
> same rule: each binds to its own `Scriptcs/_gamerelease/<module>.dll` IDA
> instance (create/register it before reversing that script).

## Module status & layout

`ScriptCS` is an early scaffold wired into the build; sources are skeleton/
empty. Unlike the C++ engine tree, it is a **C project** (`project(...
LANGUAGES C)`) pinning the shared CRT via
`CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"`.

```
ScriptCS/
├── CMakeLists.txt            # project LANGUAGES C; MultiThreadedDLL; adds ScriptRuntime, AllLevels, Hideout
├── ScriptRuntime/
│   ├── CMakeLists.txt        # add_library(ScriptRuntime STATIC source/ScriptRuntime.c)
│   ├── include/ScriptRuntime/
│   │   └── ScriptRuntime.h  # canonical C-facing mirrors of the Glacier script metadata
│   │                         #   (SCRIPTCREATOR, STATECONTROLLER, FUNCTIONCONTROLLER,
│   │                         #    SAVEGAMESTATICS, SCRIPTIMPORT, SCRIPTFUNCTIONS,
│   │                         #    INTERNALSCRIPTFUNCTIONS, TODO_PTR)
│   └── source/ScriptRuntime.c
├── AllLevels/
│   ├── CMakeLists.txt        # add_library(AllLevels STATIC source/AllLevels.c); links ScriptRuntime
│   ├── include/AllLevels/
│   │   ├── AllLevels.h       # umbrella: includes ScriptCreator.h + every Alllevels_*.h
│   │   ├── ScriptCreator.h   # shim re-exporting ScriptRuntime/ScriptRuntime.h
│   │   └── Alllevels_*.h     # per-script extern SCRIPTCREATOR declarations (11 scripts)
│   └── source/AllLevels.c    # SCRIPTCREATOR definitions (names/sizes/state/parent)
├── Hideout/
│   ├── CMakeLists.txt        # add_library(Hideout SHARED source/DllMain.c source/Hideout.c
│   │                         #   source/Scripts.c Hideout.def); links ScriptDllInterface AllLevels ScriptRuntime
│   ├── Hideout.def           # ordinal exports SF@1 ISF@2 Scripts@3 ScriptImports@4
│   ├── include/Hideout/
│   │   ├── Hideout.h         # umbrella: includes ScriptCreator.h + every Hideout_*.h
│   │   └── Hideout_*.h       # per-script extern SCRIPTCREATOR declarations (3 scripts)
│   └── source/
│       ├── DllMain.c         # returns TRUE for all reasons (shape-compatible with original)
│       ├── Hideout.c         # the 3 level-specific SCRIPTCREATOR definitions
│       └── Scripts.c         # contract tables: SF, ISF, Scripts list, ScriptImports[0x2CC]
└── Docs/                     # this memory bank
```

## AllLevels shared scripts (reconstructed from PC_Hideout)

The `Alllevels_*` creators (the shared-script subset of the Hideout.dll
`Scripts` table) are declared in `AllLevels/include/AllLevels/` and defined in
`AllLevels/source/AllLevels.c` with their recovered `SCRIPTCREATOR` metadata
(name, script/state variable sizes, initial state controller, parent creator).
Per the ScriptCS conventions (see MEMORY_BANK.md), unresolved pointer fields
(`m_pStateController`, `ProcessMessage`, `Initialize`, `Imports`, `Unpack*`,
`m_pImports`, ...) are written as `TODO_PTR` (`#define TODO_PTR NULL`) pending
per-script body reversal — `grep -r TODO_PTR` lists what is still missing.
The PC creator addresses below are reference anchors only (Docs-only).

| Script | PC creator (ref) | ScriptVars | StateVars | Parent |
| --- | --- | --- | --- | --- |
| `Alllevels_Baseboid` | 0x1003E5C0 | 0x0C | 0x00 | — |
| `Alllevels_Bird` | 0x1003F228 | 0x78 | 0x1C | Alllevels_Baseboid |
| `Alllevels_Rat` | 0x10043BB0 | 0x190 | 0x24 | Alllevels_Basefunc |
| `Alllevels_Levelcontrol` | 0x100433F8 | 0x04 | 0x00 | — |
| `Alllevels_Perceptionconverter` | 0x10043658 | 0xEC | 0x00 | — |
| `Alllevels_Basefunc` | 0x1003ECB0 | 0x17C | 0x24 | Alllevels_Perceptionconverter |
| `Alllevels_Vehicles_Car` | 0x10043D28 | 0x54 | 0x00 | — |
| `Alllevels_Human` | 0x10041A40 | 0x270 | 0x3C | Alllevels_Basefunc |
| `Alllevels_Civilian` | 0x1003FC90 | 0x290 | 0x3C | Alllevels_Human |
| `Alllevels_Armed` | 0x1003E488 | 0x2A0 | 0x3C | Alllevels_Civilian |
| `Alllevels_Guard` | 0x10040818 | 0x2DC | 0x3C | Alllevels_Civilian |

The level-specific `Hideout_*` scripts (`Hideout_Levelcontrol`,
`Hideout_Canary`, `Hideout_Happyrat`) stay in `ScriptCS/Hideout/`. Their
recovered `SCRIPTCREATOR` metadata (name, sizes, initial state, parent) is
defined in `Hideout/source/Hideout.c`; unresolved pointer fields are
`TODO_PTR` pending body reversal, matching the AllLevels scope.

CMake wiring:

- `ReHitman/CMakeLists.txt` adds `ScriptCS` after `Glacier`, `BloodMoney`,
  `ScriptInterface`, `ReHitman`.
- `ScriptCS/CMakeLists.txt`: `project(ReHitman LANGUAGES C)` +
  `set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")`; subdirs:
  `ScriptRuntime`, `AllLevels`, then `Hideout`.
- `ScriptRuntime`: STATIC lib `ScriptRuntime`, PUBLIC include `include/`,
  links `ScriptDllInterface` — canonical home of the common script-DLL runtime
  type descriptions (SCRIPTCREATOR family + SCRIPTFUNCTIONS/ISF + TODO_PTR).
- `AllLevels`: STATIC lib `AllLevels`, PUBLIC include `include/`, links
  `ScriptDllInterface` and `ScriptRuntime` — the shared-script-code layer
  intended to be linked into every scene DLL.
- `Hideout`: SHARED lib `Hideout` (this is the real `Hideout.dll` target),
  PUBLIC include `include/`, links `ScriptDllInterface` (signatures only —
  never link game-side `ScriptInterface` implementations), `AllLevels`, and
  `ScriptRuntime`.

Binding rule for the `Hideout` target: every contract check (exports, table
contents, creator metadata shapes, generated function bodies) is verified
against the PC_Hideout database (verified 2026-06, reconfirmed 2026-09-17 —
see anchors below). When new scripts are added (`ReHitman/ScriptCS/<Module>/`),
bind each to its own PC IDA instance the same way and record the mapping in
[MEMORY_BANK.md](MEMORY_BANK.md).

Current state (2026-09-17): the `Hideout` SHARED target now emits the four
ordinal exports (`SF`, `ISF`, `Scripts`, `ScriptImports`) via `Hideout.def`
and `source/Scripts.c`; the three level-specific `SCRIPTCREATOR`s are defined
in `Hideout.c`. Per-script state/function bodies remain `TODO_PTR` and are the
main pending work.

## Binary contract the DLL must satisfy

Verified in PC_Hideout (2026-09-17). Export table (ordinal → name → RVA):

| Ordinal | Name | RVA | Content |
| --- | --- | --- | --- |
| 1 | `SF` | `0x10045A80` | `SCRIPTFUNCTIONS` (0x6C), zero-filled in the image — engine writes all slots at `AttachSceneScripts` |
| 2 | `ISF` | `0x10045AEC` | `INTERNALSCRIPTFUNCTIONS` — `RunningThread`; engine clears it then points its own `ISF` at this field |
| 3 | `Scripts` | `0x10045A10` | `SCRIPTCREATOR**`: `[0] = count` (`0x3B4` = 948 in Hideout.dll), creators at `[1..]`, null-terminated |
| 4 | `ScriptImports` | `0x10045B00` | `void*[0x2CC]` import block; zero-filled in the image; engine overwrites with `ScriptInterfaces` at attach |

Plus ordinary `DllEntryPoint`. A `.rdata` pointer block (`off_10044738`)
references `{SF, ISF, Scripts, ScriptImports}` in ordinal order (linker
`/EXPORT`-related structure; informational).

`DllMain` contract: the original returns TRUE for all reasons (current stub is
shape-compatible). Attach order (engine side) is: heap block → fill SF → clear
ISF → grab Scripts → overwrite ScriptImports → run every creator's
`Initialize()` (`SCRIPTCREATOR +0x1C`). Script code must tolerate `SF`/`ISF`
slots being invalid until then (nothing runs before attach).

## ScriptImports slot usage (DLL side)

Compiled script code calls `ScriptImports[i](args...)` — by **index**, against
the signatures declared in `ScriptInterface/include/SI/SI_*.h`. Slot order is
frozen; see SCRIPT_INTERFACE.md for groups, anchors and typing rules. The DLL
is 32-bit Windows code; the C targets compile with the SI headers only (C++
headers consumed from C — keep any C-facing wrappers self-contained) — never
link the game-side `ScriptInterface` implementations into the DLL.

## Compiled metadata the DLL must emit

For every script, `.data` holds the compiler output described in
SCRIPT_ENGINE.md ("Compiled metadata"); the runtime never modifies
`SCRIPTCREATOR`/`STATECONTROLLER`/`FUNCTIONCONTROLLER` bodies, but every data
block carries `m_lStringOffsets` fixup tables so save/load can convert
pointers. Key contracts:

- `SCRIPTCREATOR`: name, script/state variable block sizes, initial state,
  parent-creator inheritance, states vtable, `ProcessMessage`, `Initialize`
  (runs once at attach), `SAVEGAMESTATICS` descriptor, `Imports`/`StaticImports`
  /`UnpackResources`/`UnpackStaticResources`, `SIT`-coded import table.
- `FUNCTIONCONTROLLER::m_pEntryPoint`: `float (__fastcall? no — cdecl float fn(ScriptState*))`
  entry following the float yield protocol. **Every** generated state/function
  body must return per protocol: `-7` yield, `-6` set-priority (side channel
  `m_SpecialScriptReturnType.m_lPriority`), `-5` continue, `-4` fork
  (`m_pForkStateController`), `-3` continue-after-timeout (`ForkStateFree`
  cleanup frames), `-2` terminate, `>= 0` sleep seconds.
- Call frames: `LocalVarEntry` 0x14 header + locals, size
  `max(m_lDataSize, 0x14)`; async calls keep `AsyncCall_Struct` nodes;
  message queues keep `MessageCue` nodes with inline payloads.
- `ProcessMessage(const ZMSGID, void*)` handlers: script-level on creator,
  state-level on `STATECONTROLLER`; ids ≥ `0x800`.
- State machine data: `SwitchStateStruct` (0x24) is the compiled switch-state
  frame; `m_lLevel`/`m_lScriptLevel` encode the state hierarchy.
- `SAVEGAMESTATICS` entries (`{type:8, size:24, addr}`) describe per-script
  saved statics: `SGST_RAW/STRING/THREAD/STATEREF/EVENTREF`.
- Script-visible enum constants: compiled into the DLL. `GetZDefine(name,
  buffer, size)` zeroes the buffer on PC (exe does not carry the registry), so
  any `ZDEFINE_*` value used by scripts must be materialized inside the DLL
  build — mirror values from `Common.h` / `Levels/Base.h` / BloodMoney headers.

## Thread/message runtime expectations (what ScriptCS code may rely on)

Detailed in SCRIPT_ENGINE.md; the DLL-visible essentials:

1. `g_pZSC`/`s_CurrentRootScriptCRef`/priority are engine-managed; DLL code
   only sees the `ScriptState*` argument and the SF table.
2. `Sleep(t)` suspends the calling thread; `ScriptEngine::Sleep` reaches the
   running thread via `g_pZSC`, so DLL calls must happen while its thread is
   running.
3. Message dispatch happens in the owning event's `SchedUpdate`; DLL
   `ProcessMessage` may be re-entered — guard with the documented flags
   (`ZSC_FLAG_HANDLING_MESSAGE`), never new ad-hoc guards.
4. All runtime allocation from the script heap via `SF.Alloc/AllocNM` (+
   `Free/FreeNM`); message-scoped allocations use the nested 4 KB pool while
   `g_pMessageAllocator` is active. Blocks are zero-initialized.
5. Script-code pointers: `GetOffsetInScriptCode`/`GetAddressInScriptCode`
   round-trip via `g_pScripts` with the `0x80000000` tag; the
   `ForkStateFree` singleton round-trips as `0x7FFFFFFF`. DLL module base ==
   `GetScriptBaseAddress()`. Never relocate that singleton.
6. Timeouts: threads exceeding ~5000 ticks/pass are force-yielded.
7. `Input(pData, lSize)` consumes the sequential PostLoad data block
   (`g_pZScriptCDataBlock`) — the `Imports()` function generated for each
   script must consume its `SCRIPTIMPORTS` table (`SIT_BYTE..SIT_END`)
   in exactly the order the data was written by the streamer.

## PC_Hideout inspection notes (bound reference for `ScriptCS/Hideout/`)

- Base image `0x10000000`; all four contract tables live in `.data`
  (`0x10045Axx` range); creator data begins `0x1003E5C0` (SCRIPTCREATOR-shaped
  entry: `.name` string ptr, sizes, state-controller ptr, initializer chain,
  empty `Imports`/`Unpack` stubs — matches `SCRIPTCREATOR +0x00..+0x34`).
- Creator-list head: `Scripts[0] = 0x3B4`; creator entries `Scripts[1..]`
  (e.g. `0x1003E5C0`, `0x1003F228`, `0x10043BB0`, `0x10044488`, `0x100440D8`,
  `0x10044380`, `0x100433F8`, ...).
- `SF`/`ScriptImports` blocks are zero in the standalone image — they are
  runtime-patched; any diff/replacement must keep them writable
  (`.data`, not `.rdata`).
- Message ids: shared script vocabulary; level-specific ids (e.g.
  `MSG_LevelControl_HitmanDied = 0x081D`) come from the common trie compiled
  into the exe, DLL `ProcessMessage` compares raw ids.
- Workflow: reverse Hideout script entry points / creator metadata from
  PC_Hideout, emit the reconstructed data + function bodies into
  `ScriptCS/Hideout/source/Hideout.c` (per-script sources), shared helpers
  into `AllLevels`; verify results by re-anchoring against PC_Hideout.

## Roadmap contracts (when ScriptCS becomes real)

1. `AllLevels` and per-script targets stay header/signature-compatible with
   `ScriptDllInterface` (`SI/SI_*.h`) — never add SI implementations there.
2. The `Hideout` SHARED target now has ordinal exports 1–4 with the exact
   content types above (`SF`, `ISF`, `Scripts`, `ScriptImports`) via
   `Hideout.def` + `source/Scripts.c`. Remaining: per-script `SCRIPTCREATOR`
   data (defined, bodies pending) + generated function bodies honoring the
   float protocol; `DllMain` preserving CRT init handling (compare PC_Hideout
   `DllEntryPoint` which wraps `_CRT_INIT`).
3. The DLL is loaded from `scriptcs/_gamerelease/<module>.dll` relative to the
   game working directory (scene COM value `ScriptCModule`), or the
   `OverrideScriptPath` registry-style override — so the `Hideout` target must
   produce `Hideout.dll` under that path layout at runtime. The CI artifact
   step copies the built `Hideout.dll` into `Scriptcs/_gamerelease/` so the
   archive carries it at the expected path.
4. Keep the binary slot/name/order contract documented here in sync with
   `SI.h`/`SI.cpp`; the engine copies the table blindly and compiled script
   callbacks break silently on any mismatch.
5. **New scripts:** create `ReHitman/ScriptCS/<Module>/` with the same
   layout as `Hideout/` (CMakeLists `add_library(<Module> SHARED
   source/DllMain.c source/<Module>.c)`, PUBLIC include dir, link
   `ScriptDllInterface`), register the module's
   `Scriptcs/_gamerelease/<Module>.dll` as its IDA instance, and document the
   binding here and in [MEMORY_BANK.md](MEMORY_BANK.md).
