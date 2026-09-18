# ScriptCS Memory Bank — Master Index

Authoritative context base for all script-side (ScriptCS) work in ReHitman.
Every future task touching `ReHitman/ScriptCS/` or the compiled-script boundary
must start here and pull in the linked documents as needed.

Data-source policy for this module: the **PC_Hideout IDA instance**
(`Scriptcs/_gamerelease/Hideout.dll`) is the reversing reference bound to the
`ScriptCS/Hideout/` project; future per-script directories bind to their own
`Scriptcs/_gamerelease/<module>.dll` IDA instances the same way. The
`ScriptInterface` module is the authoritative source of script-callable
implementation specs; the engine docs are the authoritative source of host
mechanics.

## Scope

| Area | Path | Doc |
| --- | --- | --- |
| Script engine (host side) | `ReHitman/Glacier/source/ScriptEngine/`, `ReHitman/Glacier/include/Glacier/ScriptEngine/` | [SCRIPT_ENGINE.md](SCRIPT_ENGINE.md) |
| Script interface (game API, 716 slots) | `ReHitman/ScriptInterface/` | [SCRIPT_INTERFACE.md](SCRIPT_INTERFACE.md) |
| Script DLL workspace (per-script projects) | `ReHitman/ScriptCS/AllLevels/`, `ReHitman/ScriptCS/Hideout/`, `ReHitman/ScriptCS/ScriptRuntime/` | [SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md) |
| ScriptRuntime shared types | `ReHitman/ScriptCS/ScriptRuntime/include/ScriptRuntime/ScriptRuntime.h` (+ `ZScriptImportTable.h`) | [SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md) — canonical C-facing mirrors of the Glacier script metadata + `SCRIPTFUNCTIONS`/`INTERNALSCRIPTFUNCTIONS` + `SCRIPTIMPORTSTABLE` |
| AllLevels shared scripts (11 `Alllevels_*` SCRIPTCREATORs) | `ReHitman/ScriptCS/AllLevels/include/AllLevels/` + `source/AllLevels.c` | [SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md) (reconstructed from PC_Hideout) |
| Hideout.dll reference (IDA) | **PC_Hideout** instance — `.../Games/Scriptcs/_gamerelease/Hideout.dll` | [SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md) (verification anchor for `ScriptCS/Hideout/`) |

Verified anchors (2026-09-17):

- PC_Hideout instance exists and holds `Hideout.dll`
  (`.../Games/Scriptcs/_gamerelease/Hideout.dll`); it is the designated
  reversing reference for `ScriptCS/Hideout/`.
- Its PE exports are exactly the four engine ordinals `SF=1`, `ISF=2`,
  `Scripts=3`, `ScriptImports=4` plus `DllEntryPoint`.
- `ScriptInterfacesTable` compiles to exactly `0x2CC` pointers
  (`sizeof == sizeof(void*) * 0x2CC`), slot 0 = `Zcar__Moveto`,
  slot 715 = `Pathfinder__Iswallbetweenpoints`.

Verified in the rebuilt `ScriptCS/Hideout` target (2026-09-17):

- `Hideout.dll` exports exactly the four engine ordinals `SF=1`, `ISF=2`,
  `Scripts=3`, `ScriptImports=4` (dumpbin-verified). `SF`/`ScriptImports`
  land in writable `.data` (runtime-patched); `Scripts` in read-only `.rdata`.
- The `Scripts` table lists 14 creators in PC_Hideout order (11 `Alllevels_*`
  shared + 3 level-specific `Hideout_*`), `[0] = 14`, null-terminated.
- Common runtime type descriptions (`SCRIPTCREATOR`, `STATECONTROLLER`,
  `FUNCTIONCONTROLLER`, `SAVEGAMESTATICS`, `SCRIPTIMPORT`, `SCRIPTFUNCTIONS`,
  `INTERNALSCRIPTFUNCTIONS`, `TODO_PTR`) live in the `ScriptRuntime` module;
  `AllLevels/ScriptCreator.h` is a shim re-exporting them.
- `ScriptImports` is a typed `SCRIPTIMPORTSTABLE` (0x2CC function pointers,
  PC slot order) defined in `ScriptRuntime/include/ScriptRuntime/ZScriptImportTable.h`
  — the C port of the Glacier `ZScriptImportTable.h` typed import table.

## Reading order

1. [SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md) — what `ScriptCS` must emit
   (ordinals, tables, `DllMain`) and how the game binds it.
2. [SCRIPT_ENGINE.md](SCRIPT_ENGINE.md) — attach, hosting, threading,
   scheduling, messages, memory, save/load mechanics on the engine side.
3. [SCRIPT_INTERFACE.md](SCRIPT_INTERFACE.md) — the 716 script-callable
   functions, per-domain grouping, signature rules, implementation status.

## ScriptCS reversing conventions (apply to all `ScriptCS/*` work)

These rules govern how reconstructed script metadata is written into
`ScriptCS` headers/sources. They keep the C tree buildable and greppable while
per-script bodies are still being reversed.

1. **Never hardcode raw addresses in headers or sources.** A PC_Hideout (or
   iOS) address like `0x1003E590` is a *reference* anchor, not source content.
   It may appear in `Docs/` (e.g. the SCRIPTCS_CONTRACT table) but never in
   `.h`/`.c` files. Pointers that are not yet resolved must be written as
   `TODO_PTR`.
2. **`TODO_PTR` is the placeholder for any unresolved pointer.**
   `#define TODO_PTR NULL` (defined once in
   `ScriptRuntime/include/ScriptRuntime/ScriptRuntime.h`). Every field
   that still points into unreversed script code — `m_pStateController`,
   `m_pParentCreator`, `m_pStatesVirtualTable`, `ProcessMessage`,
   `Initialize`, `m_pSaveGameStatics`, `Imports`, `StaticImports`,
   `UnpackResources`, `UnpackStaticResources`, `m_pImports` — is set to
   `TODO_PTR` until its target is reversed. After a batch, `grep -r TODO_PTR`
   lists exactly what is still missing.
3. **Declare every structure; never reference an address as a value.**
   The C-facing mirrors (`SCRIPTCREATOR`, `STATECONTROLLER`,
   `FUNCTIONCONTROLLER`, `SAVEGAMESTATICS`, `SCRIPTIMPORT`, `SCRIPTFUNCTIONS`,
   `INTERNALSCRIPTFUNCTIONS`) must be declared as structs in
   `ScriptRuntime/include/ScriptRuntime/ScriptRuntime.h`.
   `AllLevels/include/AllLevels/ScriptCreator.h` is a compatibility shim that
   re-exports them. Cross-references between the structs use the struct
   pointers, never integer addresses.
4. **Method naming comes from the iOS instance.** The iOS build carries rich
   debug names for the shared scripts, e.g. `_Alllevels_Perceptionconverter_RUN`,
   `_Alllevels_Perceptionconverter_PROCESSMESSAGE`,
   `_Alllevels_Perceptionconverter_Savecurrentstatecollection`. Use those names
   when declaring/referencing script states, functions, and entry points. PC is
   the layout/ABI reference; iOS supplies the meaningful names.
5. **SCRIPTCREATOR-only scope** (current): headers carry name + variable sizes +
   initial state + parent. Function-pointer fields stay `TODO_PTR` until the
   corresponding script body is reversed (see SCRIPTCS_CONTRACT.md).

## Cross-module invariants (never violate)

1. **Slot order is the ABI.** `SI::ScriptInterfacesTable` (716 entries) and the
   DLL's slot-4 `ScriptImports` block are indexed by position. Never reorder,
   insert, or remove.
2. **Signatures are byte-exact.** Header declarations in
   `ScriptInterface/include/SI/SI_*.h` define the script-side prototype;
   the table copies pointers with no type check. Declaration, definition, and
   table field types must stay in sync.
3. **No host-heap runtime memory for thread images.** Script states/frames
   live in the engine's 512 KB `g_ScriptMemory` pool via
   `ScriptEngine::Alloc/Free` — relevant when ScriptCS code ever allocates.
4. **Engine structs are frozen.** `ScriptState`, `LocalVarEntry`,
   `SCRIPTCREATOR`, `STATECONTROLLER`, `FUNCTIONCONTROLLER`,
   `SCRIPTFUNCTIONS`, `ZScriptC` are `RE_VERIFY_SIZE`-checked; do not change
   field order.
5. **ScriptDLL side is 32-bit Windows x86.** The `ScriptCS` projects build C
   targets against the C++ SI headers (C++11/CLI headers) — the `AllLevels`
   project pins `CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"` for the
   subdir tree. `Hideout` is a SHARED Win32 DLL; its import-table slot-4
   contract is filled by the engine at attach (see SCRIPTCS_CONTRACT.md).
