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
| 1 | `0x1003E5C0` | Alllevels_Baseboid | **DONE** (fully named) |
| 2 | `0x1003F228` | Alllevels_Bird | **PARTIAL** |
| 3 | `0x10043BB0` | Alllevels_Rat | **IN PROGRESS** |
| 4 | `0x10044488` | Hideout_Levelcontrol | not started (out of scope) |
| 5 | `0x100440D8` | Hideout_Canary | not started (out of scope) |
| 6 | `0x10044380` | Hideout_Happyrat | not started (out of scope) |
| 7 | `0x100433F8` | Alllevels_Levelcontrol | not started |
| 8 | `0x10043658` | Alllevels_Perceptionconverter | not started |
| 9 | `0x1003ECB0` | Alllevels_Basefunc | not started |
| 10 | `0x10043D28` | Alllevels_Vehicles_Car | not started |
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

### Alllevels_Baseboid — FULLY NAMED

| Address | Name |
| --- | --- |
| 0x1000263F | `Alllevels_Baseboid_RUN` |
| 0x100029D1 | `Alllevels_Baseboid_PROCESSMESSAGE` |
| 0x1000270D | `Alllevels_Baseboid_Initialize` |
| 0x1000274F | `Alllevels_Baseboid_Playanimwait` |
| 0x100027E6 | `Alllevels_Baseboid_Playanimsegment` |
| 0x10002858 | `Alllevels_Baseboid_Playanimsegmentwait` |
| 0x100028EF | `Alllevels_Baseboid_Playaniminterpolatedwait` |
| 0x10002719 | ENTER — **not renamed** (not a function in IDA) |
| nullsub_1 | Imports/UnpackResources — commented as empty stub |

### Alllevels_Bird — PARTIAL

| Address | Name |
| --- | --- |
| 0x1000A507 | `Alllevels_Bird_RUN` |
| 0x1000A6BF | `Alllevels_Bird_ENTER` |
| 0x1003A5B3 | `Alllevels_Bird_IMPORTS` (contains entry 0x1003A603) |
| — | ProcessMessage = 0 (inherits from Baseboid) |
| — | Initialize = nullsub (empty stub) |
| — | Other state functions NOT yet renamed |

### Alllevels_Rat — IN PROGRESS

SCRIPTCREATOR at `0x10043BB0`:

| Field | PC address | Name |
| --- | --- | --- |
| ProcessMessage | 0x1003B752 | `Alllevels_Rat_PROCESSMESSAGE` (**renamed**) |
| Initialize | nullsub_1 | empty stub (do not rename) |
| Imports | 0x1003A915 | `Alllevels_Rat_IMPORTS` (**renamed**) |
| StaticImports/Unpack/UnpackStatic | nullsub_1 | empty stub |

STATECONTROLLER at `0x10043B0C` → FUNCTIONCONTROLLER list at `0x10043AF4`:
- `m_pRun` → `0x1003A819` → `Alllevels_Rat_RUN` (**pending**)
- `m_pEnter` → `0x1003A936` → `Alllevels_Rat_ENTER` (**pending**)

iOS refs: `_Alllevels_Rat_PROCESSMESSAGE` (0x100F57B44),
`_Alllevels_Rat_IMPORTS` (0x100F57C3C), `_Alllevels_Rat_RUN` (0x100F57998),
`_Alllevels_Rat_ENTER` (0x100F57C74).

---

## 5. SCRIPTCREATOR data collected for remaining scripts (not yet renamed)

### Alllevels_Levelcontrol — creator `0x100433F8`, statecontroller `0x100433D0`
- ProcessMessage = 0x10037646
- Initialize = nullsub (empty stub)
- Imports = nullsub (empty stub)
- iOS: `_Alllevels_Levelcontrol_PROCESSMESSAGE` (0x10106E858),
  `_Alllevels_Levelcontrol_RUN` (0x10106E6AC),
  `_Alllevels_Levelcontrol_ENTER` (0x10106E874)

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

## 7. Key conventions / gotchas

- `nullsub_1` (0x1003C93C) = shared empty stub — **do not rename**, comment
  as empty stub.
- ENTER functions not recognized as IDA functions cannot be renamed via
  `hyper_rename_function`.
- iOS SCRIPTCREATOR is 64-bit; match by behavior/position, not fields.
- Never hardcode addresses in `.h`/`.c`; use `TODO_PTR` until reversed.
- Method names come from iOS; PC is the layout/ABI reference.
