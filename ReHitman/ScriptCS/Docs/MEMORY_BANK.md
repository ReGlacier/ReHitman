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
| Script DLL workspace (per-script projects) | `ReHitman/ScriptCS/AllLevels/`, `ReHitman/ScriptCS/Hideout/` | [SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md) |
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

## Reading order

1. [SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md) — what `ScriptCS` must emit
   (ordinals, tables, `DllMain`) and how the game binds it.
2. [SCRIPT_ENGINE.md](SCRIPT_ENGINE.md) — attach, hosting, threading,
   scheduling, messages, memory, save/load mechanics on the engine side.
3. [SCRIPT_INTERFACE.md](SCRIPT_INTERFACE.md) — the 716 script-callable
   functions, per-domain grouping, signature rules, implementation status.

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
