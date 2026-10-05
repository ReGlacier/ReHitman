# PC_Hideout Context — Script Creator/State-Function Reversing

Authoritative working context for the ongoing task of reversing the
`Scripts` export of `Hideout.dll` (IDA instance **PC_Hideout**) and
registering/naming its script creator + state functions into the
`ScriptCS/AllLevels/` project.

Data-source policy for this module (from MEMORY_BANK.md):

- **PC_Hideout** (`.../Games/Scriptcs/_gamerelease/Hideout.dll`) is the
  reversing reference bound to `ScriptCS/Hideout/`.
- **iOS** (`HitmanBloodMoney_IOS`) carries rich debug names for the shared
  scripts — it is the *naming* source. PC is the *layout/ABI* source.
- Method names come from iOS (e.g. `_Alllevels_Perceptionconverter_RUN`).
- **Never hardcode raw addresses in `.h`/`.c` files.** Unresolved pointers
  are `TODO_PTR` (== NULL). Addresses may appear in `Docs/` only.

---

## 1. The `Scripts` table in PC_Hideout

Located at `0x10045A10`:

- `[0] = 14` (count), creators on `[1..14]`, then NULL.
- Index order (1..14):

| # | Creator address | Script | Status |
| --- | --- | --- | --- |
| 1 | `0x1003E5C0` | Alllevels_Baseboid | **DONE** — named (re-synced 2026-10-05, §10) + fully ported (`AllLevels/source/Alllevels_Baseboid.c`) |
| 2 | `0x1003F228` | Alllevels_Bird | **PARTIAL** — RUN/IMPORTS/ENTER named; ENTER ported (`AllLevels/source/Alllevels_Bird.c`), RUN+states TODO |
| 3 | `0x10043BB0` | Alllevels_Rat | **PARTIAL** — RUN/ENTER/PM/IMPORTS named, no C code yet |
| 4 | `0x10044488` | Hideout_Levelcontrol | **PORTED** — named + C port (`Hideout/source/Hideout_Levelcontrol.c`) |
| 5 | `0x100440D8` | Hideout_Canary | **PORTED** — named + C port (`Hideout/source/Hideout_Canary.c`) |
| 6 | `0x10044380` | Hideout_Happyrat | **PORTED** — named + C port (`Hideout/source/Hideout_Happyrat.c`) |
| 7 | `0x100433F8` | Alllevels_Levelcontrol | names not started; parent of #4 |
| 8 | `0x10043658` | Alllevels_Perceptionconverter | names not started; root of the NPC chain (#9 → #11…#14) |
| 9 | `0x1003ECB0` | Alllevels_Basefunc | names not started; parent of #3/#11 |
| 10 | `0x10043D28` | Alllevels_Vehicles_Car | not started (no Hideout_* child in this DLL) |
| 11 | `0x10041A40` | Alllevels_Human | not started |
| 12 | `0x1003FC90` | Alllevels_Civilian | not started |
| 13 | `0x1003E488` | Alllevels_Armed | not started |
| 14 | `0x10040818` | Alllevels_Guard | not started |

> Scope: only the 11 `Alllevels_*` shared scripts. The three `Hideout_*`
> scripts are intentionally left untouched for now.

---

## 2. PC SCRIPTCREATOR struct layout (14 × 4 bytes = 0x38)

| +offset | Field | Type |
| --- | --- | --- |
| +0x00 | m_pName | const char* |
| +0x04 | m_lScriptVariablesSize | int32 |
| +0x08 | m_lStateVariablesSize | int32 |
| +0x0C | m_pStateController | const STATECONTROLLER* |
| +0x10 | m_pParentCreator | const SCRIPTCREATOR* |
| +0x14 | m_pStatesVirtualTable | const void* |
| +0x18 | ProcessMessage | ProcessMessage_t |
| +0x1C | Initialize | VoidFunction_t |
| +0x20 | m_pSaveGameStatics | const SAVEGAMESTATICS* |
| +0x24 | Imports | VoidFunction_t |
| +0x28 | StaticImports | VoidFunction_t |
| +0x2C | UnpackResources | VoidFunction_t |
| +0x30 | UnpackStaticResources | VoidFunction_t |
| +0x34 | m_pImports | const SCRIPTIMPORT* |

### STATECONTROLLER (one per state)

| +offset | Field |
| --- | --- |
| +0x00 | m_pRun (const FUNCTIONCONTROLLER*) |
| +0x04 | m_pEnter (const FUNCTIONCONTROLLER*) |
| +0x08 | m_pDestroy (const FUNCTIONCONTROLLER*) |
| +0x0C | ProcessMessage |
| +0x10 | m_pFunctionsVirtualTable |
| +0x14 | m_lLevel (uint16) |
| +0x16 | m_lScriptLevel (uint16) |
| +0x18 | m_pParent (const STATECONTROLLER*) |
| +0x1C | m_pName (const char*) |
| +0x20 | m_lStringOffsets (uint16*) |

### FUNCTIONCONTROLLER (0x10, one per callable)

| +offset | Field |
| --- | --- |
| +0x00 | m_pEntryPoint (EntryPoint_t) |
| +0x04 | m_lInputSize (uint16) |
| +0x06 | m_lDataSize (uint16) |
| +0x08 | m_pName (const char*) |
| +0x0C | m_lStringOffsets (uint16*) |

> **iOS SCRIPTCREATOR is 64-bit (ARM)** — pointer fields are 8 bytes, so the
> layout differs from PC. Field-for-field comparison does **not** work;
> matching is done by *behavior / position* of the FUNCTIONCONTROLLER lists.

---

## 3. Method-naming workflow

For each script:

1. Read the SCRIPTCREATOR struct at its creator address (pointer table,
   count 15) to get the function-pointer fields:
   - `ProcessMessage`, `Initialize`, `Imports`, `StaticImports`,
     `UnpackResources`, `UnpackStaticResources`.
   - Note: `nullsub_1` (`0x1003C93C`) is a **shared empty stub** used for
     Imports/Unpack*/StaticImports across many scripts — **do not rename**,
     only comment it as an empty stub.
2. Read the STATECONTROLLER (pointed to by `+0x0C`) to find the
   FUNCTIONCONTROLLER list for `RUN` and `ENTER`:
   - `m_pRun` → FUNCTIONCONTROLLER whose `m_pEntryPoint` is the RUN function.
   - `m_pEnter` → FUNCTIONCONTROLLER whose `m_pEntryPoint` is the ENTER function.
3. Confirm the iOS name exists (`_<Script>_RUN`, `_<Script>_ENTER`,
   `_<Script>_PROCESSMESSAGE`, `_<Script>_IMPORTS`, ...).
4. Rename in PC_Hideout with `hyper_rename_function` using
   `expected_name` = current name (compare-and-set).
5. State functions (Playanim*, Movetopos, ...) come from the FUNCTIONCONTROLLER
   list(s) after the SCRIPTCREATOR; match to iOS names by order / behavior.

> **ENTER caveat:** some entry points (e.g. `0x10002719` for Baseboid ENTER)
> are not recognized by IDA as functions (`loc_*`), so
> `hyper_rename_function` returns "no function contains address". These cannot
> be renamed as functions; document and skip (or create the function in IDA).

---

## 4. Accomplished so far

### Alllevels_Baseboid — FULLY NAMED + PORTED (2026-10-05, §10)

> **IDB note (2026-10-05):** the first-session renames of Baseboid/Bird/Rat
> were **lost** when the DB instance was re-opened — re-applied by §10; save
> the database before restarting the MCP instance.

All eight entries renamed with the current convention (leading `_` + iOS
method case) and ported; the exact list is the §10 table. Creator
`0x1003E5C0` fields are filled in `AllLevels/source/AllLevels.c`
(SC/FC anchors + savegame statics at `0x1003E5B0`), state code in
`AllLevels/source/Alllevels_Baseboid.c`.

### Alllevels_Bird — PARTIAL (named: RUN/ENTER/IMPORTS; ENTER ported)

| Address | Name |
| --- | --- |
| 0x1000A507 | `_Alllevels_Bird_RUN` (renamed, **not ported**) |
| 0x1000A6BF | `_Alllevels_Bird_ENTER` (renamed + ported, `AllLevels/source/Alllevels_Bird.c`) |
| 0x1003A5B3 | `_Alllevels_Bird_IMPORTS` (renamed, not ported; contains entry 0x1003A603) |
| — | ProcessMessage = 0 (inherits from Baseboid) |
| — | Initialize = nullsub (empty stub) |
| — | State functions (Airborne/Grounded/Hurt/Dead …) NOT yet enumerated |

### Alllevels_Rat — NAMED (no C code yet)

SCRIPTCREATOR at `0x10043BB0` (parent chain: Rat → Basefunc →
Perceptionconverter — verified in `AllLevels.c`):

| Field | PC address | Name |
| --- | --- | --- |
| ProcessMessage | 0x1003B752 | `_Alllevels_Rat_PROCESSMESSAGE` (renamed) |
| Initialize | nullsub_1 | empty stub (do not rename) |
| Imports | 0x1003A915 | `_Alllevels_Rat_IMPORTS` (renamed) |
| StaticImports/Unpack/UnpackStatic | nullsub_1 | empty stub |

STATECONTROLLER at `0x10043B0C` → FUNCTIONCONTROLLER list at `0x10043AF4`:
- `m_pRun` → `0x1003A819` → `_Alllevels_Rat_RUN` (renamed, not ported)
- `m_pEnter` → `0x1003A936` → `_Alllevels_Rat_ENTER` (renamed, not ported)

The remaining Rat states/methods (the `Alllevels_Rat_*` anchors referenced by
`Hideout_Happyrat`, incl. the FC at `0x100439AC`/`0x10043BFC` and
`sub_1003C95E`) still need the §3 loop.

### Alllevels_Levelcontrol — FULLY NAMED + PORTED (2026-10-05)

Creator `0x100433F8` (root script: no parent, no statesVT, no imports) is
filled in `AllLevels/source/AllLevels.c`; FC/SC records in
`AllLevels/source/AllLevels_LevelcontrolParentData.c`; script code in
`AllLevels/source/Alllevels_Levelcontrol.c`.

| Address | Name | Status |
| --- | --- | --- |
| 0x100372C2 | `_Alllevels_Levelcontrol_RUN` (renamed + ported) | done |
| 0x100373BE | `_Alllevels_Levelcontrol_ENTER` (renamed + ported) — writes -1 to script-var +0x00 | done |
| 0x100373F4 | `_Alllevels_Levelcontrol_Idle_RUN` (renamed + ported) | done |
| 0x10037452 | `_Alllevels_Levelcontrol_Missioncompleted` (renamed + ported; import slot 287) | done |
| 0x10037483 | `_Alllevels_Levelcontrol_Missionfailed` (renamed + ported; slot 288) | done |
| 0x100374B8 | `_Alllevels_Levelcontrol_Characterkilled` (renamed + ported; slot 289; PC's 4th state FC reuses this same entry) | done |
| 0x10037646 | `_Alllevels_Levelcontrol_PROCESSMESSAGE` (renamed + ported; dispatcher: only 0x0857 is handled, root script so no parent walk) | done |
| 0x100374ED | `_Alllevels_Levelcontrol_Message_0857` (user created the function; renamed + ported) | done |

State/data map: root SC `0x100433D0` (RUN FC `0x100433B8`, ENTER FC
`0x100433C4`, stringblob = SGST_END at `0x100433F0`); `Idle` SC `0x10043444`
(level 2, parent = root; RUN FC `0x10043438`; the state's FC table follows at
`0x10043464`: Missioncompleted in=0x14, Missionfailed in=0x18
(stringoffsets=`0x100459E4`), Characterkilled in=0x1C, 4th slot = same
Characterkilled entry, in=0x1C; `sub_10037452/0x10037483/0x100374B8` between
FCs are just the embedded debug strings).

Decoded 0x0857 payload (`_Alllevels_Levelcontrol_Message_0857`, 0x100374ED):
`{ ZREF first[8]; ZREF second[8]; int32 mode+0x40 }` — for each pair (loop
`i < 8`, stop at a null `first`): `SF.FindScriptStateByRef(first,
"alllevels.human")` + `SF.GetAlienScriptState` (null result → `SF.Sleep(-2)`
and abort); skip if the human's script-vars byte `+0x17D` bit 0 is set or its
int `+0x68` ≥ 3 (Alllevels_Human vars unreversed — raw offsets in the port);
otherwise reuse/create the ONE permanent line at our script-var `+0x00` via
import slot 594 `Debugfunctions__Displaypermanentscriptline(first, second,
0x00FF0000)` / slot 596 `Debugfunctions__Modifypermanentscriptline(handle,
...)`, then `SF.SendCommand(first, 0x085B, &second, m_rThis)` and return when
`mode == 2`, else send `0x0859` and return when `mode == 0` (otherwise walk
to the next pair). Script-var `+0x00` is that line handle; ENTER stamps -1.

---

## 5. SCRIPTCREATOR data collected for the remaining scripts

> The function-pointer addresses below are from the first pass and were
> **never renamed in the current DB** (the renames were lost, §4 note — the
> §10 batch only restored Baseboid/Bird/Rat). Use them as the starting point
> for the §3 loop; verify the current name with `hyper_search_functions`
> before each `rename_function`.

### Alllevels_Levelcontrol — DONE (see §4: ported 2026-10-05, only the
PROCESSMESSAGE 0x0857 handler body at `loc_100374ED` is still unreversed)
- ProcessMessage = 0x10037646 (dispatcher stub; create function in DB)
- Initialize = nullsub (empty stub)

### Alllevels_Perceptionconverter — creator `0x10043658`, statecontroller `0x100435C8`
- ProcessMessage = 0x1003A742
- Initialize = 0x10037751
- Imports = 0x10037789
- iOS: `_Alllevels_Perceptionconverter_PROCESSMESSAGE` (0x100F35E60),
  `_Alllevels_Perceptionconverter_IMPORTS` (0x100F36300),
  `_Alllevels_Perceptionconverter_RUN` (0x100F35CB4),
  `_Alllevels_Perceptionconverter_ENTER` (0x100F36504)

### Alllevels_Basefunc — creator `0x1003ECB0`, statecontroller `0x1003EA1C`
- ProcessMessage = 0x1000A1FF
- Initialize = 0x10002ADC
- Imports = 0x10002B7F
- iOS: `_Alllevels_Basefunc_PROCESSMESSAGE` (0x100FB316C),
  `_Alllevels_Basefunc_IMPORTS` (0x100FB37DC),
  `_Alllevels_Basefunc_RUN` (0x100FB2FC0),
  `_Alllevels_Basefunc_ENTER` (0x100FB3824)

#### Basefunc data-layout correction (2026-10-05)

`AllLevels/source/Alllevels_Basefunc.c` now accesses statics and script
variables through `BasefuncStatics` and `BasefuncScriptVariables`, not
pointer arithmetic. Neutral field names identify offsets until their
semantics are verified; these are local script-data mirrors, not Glacier
class layouts.

- `BasefuncStatics`: size `0x58`, maps PC globals `0x1004688C..0x100468E3`.
  INITIALIZE matches `0x10002ADC`; the registered field at `0x100468C4`
  is deliberately not initialized by that routine.
- Savegame registration at `0x1003EBF8` contains **22 separate fields plus
  SGST_END**, not one raw block. Field order and sizes are preserved;
  `0x00000200` encodes type 0, size 2 (not size `0x200`). Padding is not saved.
- `BasefuncScriptVariables`: size `0x17C`; parent-owned `0xEC` bytes remain
  opaque. The two four-byte imports at `+0x148` and `+0x178` use member
  addresses and `sizeof`.
- The ENTER initialization stores at `+0x10C/+0x110/+0x114/+0x118` are
  `200.0f/0.9f/0.0f/-200.0f`. The word store at `+0xED` is represented by
  byte fields: preserve bits 6/7 of `+0xED`, set its low nibble, clear `+0xEE`.
  No broad memset: imports, parent data, and untouched fields must survive.
- Compile-time size and offset checks use typedef-array assertions,
  compatible with this target's MSVC C mode (which rejects `_Static_assert`).

**Status remains PARTIAL.** ENTER still lacks the parent/callee continuation
and engine calls from PC `0x10002BAF`; its initialization currently runs at
resume 0 rather than PC resume 2. RUN continuation indices, function/state
VTs, and PROCESSMESSAGE also require completion. A successful build does
not establish behavior parity with the original script.

### Alllevels_Vehicles_Car — creator `0x10043D28`, statecontroller `0x10043CF8`
- ProcessMessage = 0x1003C425
- Initialize = nullsub (empty stub)
- Imports = 0x1003B890
- iOS: `_Alllevels_Vehicles_Car_PROCESSMESSAGE` (0x10103A2E0),
  `_Alllevels_Vehicles_Car_IMPORTS` (0x10103A538),
  `_Alllevels_Vehicles_Car_RUN` (0x10103A134),
  `_Alllevels_Vehicles_Car_ENTER` (0x10103A57C)

### Alllevels_Human — creator `0x10041A40`, statecontroller `0x100413D4`
- ProcessMessage = 0x100379F7
- Initialize = 0x1001BB88
- Imports = 0x1001BE0B
- iOS: `_Alllevels_Human_PROCESSMESSAGE` (0x1010094CC),
  `_Alllevels_Human_IMPORTS` (0x1010099C0),
  `_Alllevels_Human_RUN` (0x101009320),
  `_Alllevels_Human_ENTER` (0x1010099F8)

### Alllevels_Civilian — creator `0x1003FC90`, statecontroller `0x1003FACC`
- ProcessMessage = 0x10012A73
- Initialize = 0x1000D224
- Imports = 0x1000D240
- iOS: `_Alllevels_Civilian_PROCESSMESSAGE` (0x101000BDC),
  `_Alllevels_Civilian_IMPORTS` (0x101000CB0),
  `_Alllevels_Civilian_RUN` (0x101000A30),
  `_Alllevels_Civilian_ENTER` (0x101000D14)

### Alllevels_Armed — creator `0x1003E488`, statecontroller `0x1003E384`
- ProcessMessage = 0 (inherits from parent)
- Initialize = nullsub (empty stub)
- Imports = 0x100010FC
- iOS: `_Alllevels_Armed_RUN` (0x1010D5380),
  `_Alllevels_Armed_IMPORTS` (0x1010D552C),
  `_Alllevels_Armed_ENTER` (0x1010D55A0)
- **Caution:** iOS also has `_Alllevels_Armedbartender_*` — a *separate*
  script not in the PC_Hideout AllLevels list; do not confuse it with `Armed`.

### Alllevels_Guard — creator `0x10040818`, statecontroller `0x10040570`
- ProcessMessage = 0x1001B7C1
- Initialize = 0x10012C79
- Imports = 0x10012C98
- iOS: `_Alllevels_Guard_PROCESSMESSAGE` (0x100F7B544),
  `_Alllevels_Guard_IMPORTS` (0x100F7B80C),
  `_Alllevels_Guard_RUN` (0x100F7B398),
  `_Alllevels_Guard_ENTER` (0x100F7B8C8)

---

## 6. Remaining tasks

1. **Rename SCRIPTCREATOR-level functions** for the remaining scripts
   (Rat RUN/ENTER pending; then Levelcontrol, Perceptionconverter, Basefunc,
   Vehicles_Car, Human, Civilian, Armed, Guard). For each: verify current name
   via `hyper_find_references`, then `hyper_rename_function` with
   `expected_name`.
2. **Rename state functions** for each script (FUNCTIONCONTROLLER lists,
   matching iOS names by order/behavior).
3. **Add declarations + TODO stubs** to the project:
   - `AllLevels/include/AllLevels/Alllevels_<Script>.h` — declarations.
   - `AllLevels/source/AllLevels.c` (or new source files) — bodies begin with
     `// TODO: Finish me`.
4. **Build/verify** compilation (CMake, `build` dir, Debug config).
5. Consider a **PROGRESS.md** to track the large multi-script effort.

## 7. Compiled script runtime (support section)

Every ScriptCS DLL embeds a support section emitted by the script compiler.
In PC_Hideout it occupies `0x1003D06F..0x1003D95C`; it is now IDA-named and
ported 1:1 into `ScriptRuntime/source/ScriptSupport.c`. The conventions below
are what the `Hideout_*` C ports follow.

### PC support-section addresses → names (all renamed)

| PC address | IDA name | C port |
| --- | --- | --- |
| `0x1003D06F` | `AS_CountStateTransitions` | `AS_CountStateTransitions` |
| `0x1003D0E2` | `AS_ResizeFrameVars` | `AS_ResizeFrameVars` |
| `0x1003D14B` | (while-loop yield helper, unnamed) | file-local `LC_WhileYield` in `Hideout_Levelcontrol.c` |
| `0x1003D188` | `AS_UnlinkAsyncCall` | `AS_UnlinkAsyncCall` |
| `0x1003D216` | `AS_ExitCall` | `AS_ExitCall` (returns -3.0) |
| `0x1003D27C` | `AS_GetActiveFrame` | `AS_GetFrame` |
| `0x1003D299` | `AS_InstallCall` | `AS_InstallCall` |
| `0x1003D460` | (forced async call, not named by iOS) | `AS_InstallForcedAsyncCall` |
| `0x1003D581` | `AS_SwitchState` | `AS_SwitchState` |
| `0x1003D64C` | `AS_SwitchStateDriver` | `AS_SwitchStateDriver` |
| `0x1003D8C5` | `InstallSwitchState` | `AS_InstallSwitchState` |
| `0x1003C93C` | `nullsub_1` shared empty void stub | `AS_EmptyVoid` |
| `0x1000721C` | shared empty entry stub | `AS_EmptyEntryPoint` |

### Frame model

- A compiled entry is `float f(ScriptState*)`. The active frame is the LVE
  returned by `AS_GetFrame`: `pState->m_pVariables`, except when
  `ZSC_FLAG_ASYNC_ACTIVE` is set without `ZSC_FLAG_ASYNC_WAITING` — then it
  is `pState->m_pAsyncCall->m_pLVE`.
- `LocalVarEntry` is 0x14; frame locals live at `(char*)lve + 0x14`.
- `lve->m_lFunctionIndex` is the resume case (a compiler-assigned state
  machine label); `0x8000` marks the exiting variant, `m_lExitFunctionIndex
  = 0x7FFF` marks "no pending exit", `0xFFFF` belongs to the switch driver.
- Deeper frames are allocated with `SF.Alloc(size, __FILE__, __LINE__)`,
  linked via `m_pNextVariables`, and their size recorded in
  `m_lNextVariablesSize` — then freed at the matching resume case.

### Async / continuation protocol

- `AS_InstallCall(state, frame, nextCase, calleeFC)` writes `nextCase` into
  the current resume index (unless the exit-flag variant), attaches
  `calleeFC` to the continuation LVE, links it, runs `SF.RunNoBreak`, and
  zeroes `FC->m_lDataSize` bytes past the callee's `m_lInputSize`.
- The caller then returns `-3.0` (`SC_RET_CONTINUE_AFTER_TIMEOUT`); the
  scheduler resumes the child frame; when the child finishes,
  `AS_ExitCall` unwinds and the parent re-enters at `nextCase`.
- `AS_ExitCall` at the end of an ENTER body is the standard tail sequence
  after `SF.Free(m_pNextVariables)`.

### SwitchState

- Root `RUN` bodies end with: alloc a 0x24 block, cast to
  `SwitchStateStruct`, write the target `STATECONTROLLER*` into `stateController`,
  then `AS_InstallSwitchState(..., 6, &DoSwitchState_FC)` and `return 0.0f`.
  PC shares one `DoSwitchState` FC pair per DLL (`0x10044520` /
  `0x1004452C`); their bodies exist in iOS as `_DoSwitchState` /
  `_DoSwitchStateDestroy` but are not yet ported, so the C records expose
  the entries as TODO.

### Creator-/state-level records in compiled data

- `FUNCTIONCONTROLLER` = `{entry, inputSize(u16), dataSize(u16), name, stringOffsets}`
  (0x10 on PC). Input size = LVE header + locals of the callee frame.
- STATECONTROLLER and SCRIPTCREATOR commonly share one `m_lStringOffsets`
  u16 blob — it doubles as the creator `m_pImports` array; its trailing
  `0x0005,0,0` is also the `SAVEGAMESTATICS` SGST_END terminator the
  `m_pSaveGameStatics` points to (see the Levelcontrol/Canary ports).
- State- and creator-level `ProcessMessage` are 3-arg
  (`ScriptState*, msgId, arg`); the frozen `ProcessMessage_t` typedef is
  2-arg, so ports declare `StateProcessMessage_t` and cast on store.
- `IMPORTS` bodies = parent creator's `Imports` first (shared empty stub in
  most Alllevels parents), then `SF.Input(scriptvars + offset, size)` per
  imported script variable, in PC order.

---

## 8. Hideout_* ports — inventory & remaining TODOs (2026-10-05)

All three level scripts are named in PC_Hideout (iOS names) and ported:

| Script | IDA names (all renamed) | C port |
| --- | --- | --- |
| Canary | `_Hideout_Hideout_Canary_{RUN,IMPORTS,Ambient_RUN,PROCESSMESSAGE,ENTER}` | `Hideout/source/Hideout_Canary.c` |
| Levelcontrol | `_Hideout_Hideout_Levelcontrol_{RUN,IMPORTS,Checkitems,ENTER,Idle_RUN,PROCESSMESSAGE}` | `Hideout/source/Hideout_Levelcontrol.c` |
| Happyrat | `_Hideout_Hideout_Happyrat_{RUN,STATICINITIALIZERS,IMPORTS,Happyfunness_RUN,Dodeadstuff,PROCESSMESSAGE,ENTER}` | `Hideout/source/Hideout_Happyrat.c` |

Known semantic summaries: Canary copies the feed-point position and plays a
random idle animation via Baseboid `Playanimwait`; Levelcontrol runs a
Zlist item check twice over two imported lists then sleeps in Idle (PM
catches `0x0B3D` → `Silevelcontrol__Missioncompleted`); Happyrat chains the
Rat parent, has Idle/Happyfunness states, `Dodeadstuff`, and a real
`STATICINITIALIZERS` (itsInitialize slot).

Remaining gaps (all marked `TODO_PTR` + `// TODO:` at the call sites):

- Parent-chain *function bodies* are unreversed: `Alllevels_Bird_RUN` and its
  state machine, `Alllevels_Baseboid_{RUN,Playanimsegment,Playanimsegmentwait,Playaniminterpolatedwait,...}`,
  `Alllevels_Rat_*`, `Alllevels_Levelcontrol_ENTER`. (Baseboid ENTER /
  PlayAnimWait and Bird ENTER are now ported — see §10; the FC/SC chains are
  in `AllLevels/source/AllLevels.c`, other anchors in
  `AllLevels_LevelcontrolParentData.c`, `AlllevelsRatParentData.c`.)
- The shared `DoSwitchState` / `DoSwitchStateDestroy` FC entries (PC FC
  records exist; iOS bodies only).
- Bird/Canary `FunctionsVT`s beyond the RUN/ENTER pair, and name strings for
  states (`m_pName`) — only observed as string blobs in PC.

Build: `cmake --build build --config Debug --target Hideout` produces
`Hideout.dll` from these sources.

---

## 9. Key conventions / gotchas

- `nullsub_1` (0x1003C93C) = shared empty stub — **do not rename**, comment
  as empty stub.
- Some entry points (e.g. `0x10002719` Baseboid ENTER) are not IDA functions;
  **ask the user to create the function in IDA** before renaming/ porting —
  `hyper_rename_function` on a raw address fails.
- iOS SCRIPTCREATOR is 64-bit; match by behavior/position, not fields.
- Never hardcode addresses in `.h`/`.c`; use `TODO_PTR` until reversed.
- Method names come from iOS; PC is the layout/ABI reference.

---

## 10. Second pass 2026-10-05 — Baseboid complete, Bird ENTER, ID-sync

### IDA (HBMScripts_PC) renames now applied (compare-and-set)

Names follow the user rule (leading `_`, then script, then method, iOS-derived):

| PC address | IDA name | Status |
| --- | --- | --- |
| `0x1000263F` | `_Alllevels_Baseboid_RUN` | ported |
| `0x1000270D` | `_Alllevels_Baseboid_STATICINITIALIZERS` | ported (creator `Initialize` slot) |
| `0x10002719` | `_Alllevels_Baseboid_ENTER` | ported («not a function» → user created it in IDA) |
| `0x1000274F` | `_Alllevels_Baseboid_PlayAnimWait` | ported (renamed with correct case) |
| `0x100027E6` | `_Alllevels_Baseboid_PlayAnimSegment` | ported |
| `0x10002858` | `_Alllevels_Baseboid_PlayAnimSegmentWait` | ported |
| `0x100028EF` | `_Alllevels_Baseboid_PlayAnimInterpolatedWait` | ported |
| `0x100029D1` | `_Alllevels_Baseboid_PROCESSMESSAGE` | ported |

Names follow the user rule (leading `_`, then script, then method, iOS-derived).

### C ports

- `AllLevels/source/Alllevels_Baseboid.c` — `Alllevels_Baseboid_ENTER`,
  `Alllevels_Baseboid_PlayAnimWait`.
- `AllLevels/source/Alllevels_Bird.c` — `Alllevels_Bird_ENTER`.
- `AllLevels.c` FC records `Alllevels_Baseboid_{Enter,Playanimwait}…` and
  `Alllevels_Bird_Enter…` now carry real entries (remaining TODO: Baseboid/Bird
  RUN and the rest).
- `AllLevels/CMakeLists.txt` lists both new sources; `Hideout` builds and links.

### Decoded semantics

**BaseboidScriptVars (0x0C, inferred from the ported pair only)**

| Offset | Meaning |
| --- | --- |
| +0x00 | byte, bit `0x02` = "segment running/cancel": ENTER sets it; PlayAnimWait clears it before starting and watches it afterwards |
| +0x08 | int32 — result/handle of `Zlink__PlayAnimSegment` (0 ⇒ nothing playing → PlayAnimWait unwinds) |

**PlayAnimWait protocol** — installs nothing: it is *itself* the FC that a
parent's `AS_InstallCall` targets (e.g. Canary Ambient writes the animation
handle into frame local `+0x14` first, `m_lInputSize 0x18`). Resume cases:
0 = start (set state-flag guard bit `0x0001`, exit-index 0x7FFF, clear the
cancel bit, PlayAnimSegment(anim, segment 1, from 0.0, end -1.0, blend 1.0),
clear the guard bit again, exit-call if the segment handle is 0); 1 = the same
start block but *without* first setting the guard bit / exit-index; 2 = wait:
exit-call when the cancel bit is set again **or**
`ZSC_FLAG_HANDLING_MESSAGE`, else set index 2 and
`return -7.0f` (SC_RET_RETRY_NEXT_PASS); default = exit-call unconditionally.
The guard bit `0x0001` is unnamed in `ScriptFlags.h` (only appears inside
`ZSC_ASYNC_RESUME_CLEAR_MASK`/`ZSC_LOCAL_ASYNC_BLOCK_MASK`); it wraps the
synchronous import call and `AS_ExitCall` clears it — kept as a raw bit with
a comment in the C port.

**Bird ENTER** (0x1000A6BF) — after the parent-chain step (parent ENTER FC +
`nextCase 2`), the case-2 block: state byte `+0x0C |= 0x06`; zeroes `+0x10`,
`+0x3C`; writes float `380.0` at `+0x14`;
`Zhm3Boid__SetVisionRangeAndAngle(rThis, float(+0x28) * 1.5, 270.0)`; clears
bit `0x02` of `+0x0C` when `+0x40 == 0`; resolves the eleven `/Movement/*`
animation handles (`Zlink__GetAnim`) into `+0x50..+0x64`; if HeadShot is 0 it
falls back to Die_01. BirdScriptVars layout is documented in the C file
(opaque padding for everything the ported function does not touch).

### Import slot verification

Slot = `(address - ScriptImports base 0x10045B00) / 4`, cross-checked against
the field order of `SCRIPTIMPORTSTABLE` in
`ScriptRuntime/include/ScriptRuntime/ZScriptImportTable.h`:

| Address | Slot | Field |
| --- | --- | --- |
| `0x10045B40` | 16 | `Zlink__GetAnim` |
| `0x10045B4C` | 19 | `Zlink__PlayAnimSegment` |
| `0x100462D4` | 501 | `Zhm3Boid__SetVisionRangeAndAngle` |
| `0x10045AB8` | SF +0x38 | `SF.Alloc` (SF block base `0x10045A80`) |
| `0x10045AC0` | SF +0x40 | `SF.Free` |

This makes the table-based call mapping (no raw addresses in `.c`) exact for
new script ports.

---

## 11. Plan for the remaining `Alllevels_*` (order matters)

All 11 shared scripts are in the *same* `Hideout.dll`, and the three
`Hideout_*` scripts already ported depend on these parents at runtime — a
`TODO_PTR` in a parent FC chain means the child chain is not executable. The
order below is the dependency order (parents first); inside one script the
routine is always: (1) names in IDA (+ ask the user to turn any `loc_`
address into a function), (2) anchor records (creator/SC/FC) in
`AllLevels/source/AllLevels.c` from the PC data, (3) per-script C file, (4)
fill entries, (5) build + Docs.

| Order | Script | Why next | Notes |
| --- | --- | --- | --- |
| 0 | Baseboid | done | root; its two-channel animation protocol is in §10 |
| 1 | **Bird** | parent of Canary, everything Baseboid-based | needs its full state/FC tables (the `Alllevels_Bird_*` anchors in `AllLevels.c` are half-empty), 10+ methods; then Canary's chain really runs |
| 2 | **Levelcontrol** | parent of Hideout_Levelcontrol | **fully done 2026-10-05** (§4): chain green including the 0x0857 message handler |
| 3 | **PerceptionConverter** | root of the NPC chain (parent of Basefunc) | must precede Basefunc/Rat; its own state tables first |
| 4 | **Basefunc** | parent of Rat **and** Human (big common NPC core, vars 0x17C) | after #3; Rat/Human inherit its methods |
| 5 | **Rat** | parent of Happyrat | anchors and names already in the repo (§4); do after its parents |
| 6 | Human → Civilian → Armed/Guard | NPC chain leaves, no `Hideout_*` child | only when another scene DLL needs them |
| 7 | Vehicles_Car | no child in this DLL | last; parent creator still unknown (`TODO_PTR` in `AllLevels.c` + §5) |

Practical rules for this part of the work:

- **Naming first, port second.** With names in place, every follow-up
  decompile is self-describing; re-run the §3 loop per script.
- After a batch of renames, **save the IDB** to disk — the first pass lost
  all names when the server instance was re-opened (§4 note).
- Do not re-create parent records in child files; the single source of truth
  for shared chains is `AllLevels.c` + the per-script files.
- `AS_InstallCall`/`AS_InstallSwitchState` on a `TODO_PTR` entry stays
  un-executable by design; that is the marker of “chain not finished yet”
  (`grep -r TODO_PTR` ≡ work list).
