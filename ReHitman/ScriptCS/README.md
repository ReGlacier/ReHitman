# ScriptCS — Script-DLL Workspace

`ScriptCS` is the C-language workspace that builds the compiled scene-script
modules the engine attaches at runtime. Each scene module is a 32-bit Windows
DLL (`scriptcs/_gamerelease/<module>.dll`) carrying the script metadata and
function bodies the game's script engine loads and runs.

The module is split into three CMake targets:

```
ScriptCS/
├── CMakeLists.txt            # C project; MultiThreadedDLL; adds the three subdirs
├── ScriptRuntime/            # common runtime type descriptions
├── AllLevels/                # shared "Alllevels_*" scripts (linked into every scene DLL)
├── Hideout/                  # the level-specific Hideout.dll target
└── Docs/                     # reversing memory bank (authoritative contracts)
```

## ScriptRuntime

A static library holding the **common script-DLL runtime type descriptions** —
the C-facing mirrors of the Glacier script metadata structures. Every scene
DLL and the shared-script layer include this header.

`ScriptRuntime/include/ScriptRuntime/ScriptRuntime.h` defines:

- `SCRIPTCREATOR`, `STATECONTROLLER`, `FUNCTIONCONTROLLER`,
  `SAVEGAMESTATICS`, `SCRIPTIMPORT` — the compiled-script metadata emitted by
  the script compiler and read by the engine.
- `SCRIPTFUNCTIONS` (0x6C) and `INTERNALSCRIPTFUNCTIONS` — the engine→script
  function table (ordinal 1) and the running-thread link (ordinal 2).
- `TODO_PTR` — the placeholder for any pointer still resolving into
  unreversed script code (`#define TODO_PTR NULL`).

Field order is frozen to match the Glacier engine structs; never reorder.

## AllLevels

A static library of the **shared "Alllevels_*" scripts** — the common script
set linked into every scene module. It declares and defines the 11 shared
`SCRIPTCREATOR`s (`Alllevels_Baseboid`, `Alllevels_Bird`, `Alllevels_Rat`,
`Alllevels_Levelcontrol`, `Alllevels_Perceptionconverter`,
`Alllevels_Basefunc`, `Alllevels_Vehicles_Car`, `Alllevels_Human`,
`Alllevels_Civilian`, `Alllevels_Armed`, `Alllevels_Guard`).

Each header declares one script's `SCRIPTCREATOR`; `source/AllLevels.c`
defines them with the recovered metadata (name, script/state variable sizes,
initial state controller, parent creator). Unresolved function-pointer fields
are `TODO_PTR` pending per-script body reversal.

`AllLevels/ScriptCreator.h` is a compatibility shim that re-exports the type
definitions from `ScriptRuntime` — existing includes keep working.

## Hideout

The **level-specific Hideout.dll target** (`SHARED`). It is the real
`Hideout.dll` the engine attaches for the Hideout level.

- `Hideout.def` — declares the four engine ordinal exports
  `SF @1`, `ISF @2`, `Scripts @3`, `ScriptImports @4`.
- `source/Scripts.c` — defines the contract tables: the zero-filled
  `SCRIPTFUNCTIONS`/`INTERNALSCRIPTFUNCTIONS` instances, the `Scripts`
  creator list (14 creators in PC_Hideout order, `[0] = 14`,
  null-terminated), and the `void*[0x2CC]` `ScriptImports` import block.
- `source/Hideout.c` — defines the three level-specific `SCRIPTCREATOR`s:
  `Hideout_Levelcontrol`, `Hideout_Canary`, `Hideout_Happyrat`.
- `source/DllMain.c` — `DllMain` returning `TRUE` for all reasons
  (shape-compatible with the original).

### Binary contract

The engine attaches the DLL as follows: allocate the script heap → fill the
`SF` table → clear `ISF` and point the engine's own `ISF` at it → read the
`Scripts` creator list → overwrite the `ScriptImports` block with the 716
`ScriptInterfaces` → run every creator's `Initialize()`. `SF` and
`ScriptImports` are runtime-patched, so they must live in writable `.data`;
`Scripts` is read-only.

## Docs

The `Docs/` directory is the reversing memory bank. Start with
`MEMORY_BANK.md`, then read `SCRIPTCS_CONTRACT.md` (module layout + binary
contract), `SCRIPT_ENGINE.md` (host-side engine mechanics), and
`SCRIPT_INTERFACE.md` (the 716 script-callable functions).
