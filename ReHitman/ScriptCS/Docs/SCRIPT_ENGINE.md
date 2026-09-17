# Script Engine — Architecture & Contracts

Source of truth: `ReHitman/Glacier/include/Glacier/ScriptEngine/*.h` and
`ReHitman/Glacier/source/ScriptEngine/*.cpp`. This is the *host* (game-exe)
half of the ScriptC system. The DLL half is documented in
[SCRIPTCS_CONTRACT.md](SCRIPTCS_CONTRACT.md); the callable API in
[SCRIPT_INTERFACE.md](SCRIPT_INTERFACE.md). The directory also contains a
detailed upstream `README.md`; this document pins the facts that ScriptCS work
depends on.

## Component map

| Class / unit | File(s) | Role |
| --- | --- | --- |
| `ScriptEngine` (static API) | `ScriptEngine.h/.cpp` | DLL attach/detach, script heap, thread control, message id resolution, script-code address mapping, `SCRIPTFUNCTIONS` wiring |
| `ZScriptC` event | `ZScriptC.h/.cpp` | Per-`ZGEOM` event hosting one or more cooperative script threads; create/run/message/fork/save/load; owns `.dll` load via `SetName` |
| `ZScriptCComponent` | `ZScriptCComponent.h/.cpp` | Global singleton installing the message-name trie (`ZScriptC_ZMessages` + `Uniques`) |
| `SF_*` helpers | `SF.h/SF.cpp` | Engine-side entries for `SCRIPTFUNCTIONS`: timeout, print, pack/unpack/input, memcpy/memset |
| Save/load passes | `ScriptSaveLoad.h/.cpp` | Pointer-graph preparation, fixup encoding/decoding for save and restore |
| Globals | `Globals.h/.cpp` | PC-address-backed engine state (heap, tables, current thread, save state) |
| Scheduler | `Glacier/EventBase/ZScheduledScript.h` (outside this dir) | 16 running queues + sleep queue; runs `ZScriptC::SchedUpdate` |

## Header reference (type contracts)

All sizes verified by `RE_VERIFY_SIZE`. Field order is frozen.

### Compiled metadata (produced by the script compiler, read by the engine)

```cpp
struct _SCRIPTCREATOR                  // 0x38
{
    const char* m_pName;               // +0x00 script name, e.g. "Hideout_Main"
    int32_t m_lScriptVariablesSize;    // +0x04 shared script-global block size
    int32_t m_lStateVariablesSize;     // +0x08 per-thread state block size
    const STATECONTROLLER* m_pStateController; // +0x0C initial state
    const _SCRIPTCREATOR* m_pParentCreator;    // +0x10 inheritance chain
    const void* m_pStatesVirtualTable;         // +0x14
    ProcessMessage_t ProcessMessage;           // +0x18 script-level message handler
    VoidFunction_t Initialize;                 // +0x1C run once at attach
    const SAVEGAMESTATICS* m_pSaveGameStatics; // +0x20
    VoidFunction_t Imports;                    // +0x24 consumes PostLoad data block
    VoidFunction_t StaticImports;              // +0x28
    VoidFunction_t UnpackResources;            // +0x2C
    VoidFunction_t UnpackStaticResources;      // +0x30
    const SCRIPTIMPORT* m_pImports;            // +0x34 SIT_END-terminated import table
};

struct _STATECONTROLLER                // one per state
{
    const FUNCTIONCONTROLLER* m_pRun;     // state body
    const FUNCTIONCONTROLLER* m_pEnter;   // one-shot enter handler
    const FUNCTIONCONTROLLER* m_pDestroy;
    void (*ProcessMessage)();             // state-level message handler
    const void* m_pFunctionsVirtualTable;
    const uint16_t m_lLevel;
    const uint16_t m_lScriptLevel;
    const _STATECONTROLLER* m_pParent;
    const char* m_pName;
    uint16_t* m_lStringOffsets;           // save/load fixup table
};

struct _FUNCTIONCONTROLLER             // 0x10, one per callable
{
    EntryPoint_t m_pEntryPoint;           // float fn(ScriptState*)  — see float protocol
    uint16_t m_lInputSize;
    uint16_t m_lDataSize;                 // local frame size (>= sizeof(LocalVarEntry))
    const char* m_pName;
    uint16_t* m_lStringOffsets;           // save/load fixup table
};
```

`m_lStringOffsets` is a zero-type-terminated list of `(type, startOffset,
endOffset)` triples; the types are `SaveRefType` values telling the save system
which dword fields in a data block are pointer-like.

`_SCRIPTIMPORTS` (`0x2`, bitfield `m_SIT : 3` + `m_lAmount : 13`) and
`_SCRIPTIMPORTTYPE` (`SIT_BYTE/SHORT/LONG/FLOAT/STRING/REF/END`) describe the
PostLoad data block consumed by `Imports`.

### Runtime thread image

```cpp
struct ScriptStateInfo                 // 0x8 (root/current pair)
{
    ScriptState* m_pRootScriptState;
    ScriptState* m_pCurrentScriptState;
};

struct _ScriptState                    // 0x40
{
    void* m_pScriptVariables;            // shared script-global block
    const SCRIPTCREATOR* m_pCreator;
    ZREF m_rThis;                        // owning ZGEOM ref
    LocalVarEntry* m_pVariables;         // current call frame
    void* m_pStateVariables;
    const STATECONTROLLER* m_pStateController;
    const STATECONTROLLER* m_pPreviousStateController;
    const STATECONTROLLER* m_pNextStateController;
    ZMSGID m_msgWaitForEvent;
    uint16_t m_Flags;                    // ZSC_FLAG_* / ZSF_* bits
    AsyncCall_Struct* m_pAsyncCall;
    AsyncCall_Struct* m_pAsyncCallLast;
    const SCRIPTCREATOR* m_pMessageHandler;
    ScriptState* m_pAlienCall;
    const void* m_pFunctionsVirtualTable;
    void* m_pThreadInfo;                 // owning ZScheduledScript
    MessageCue* m_pMessageCue;
};

struct _LocalVarEntry                  // 0x14 call-frame header
{
    const FUNCTIONCONTROLLER* m_pFunctionController; // null = free frame
    _LocalVarEntry* m_pNextVariables;                // callee direction
    _LocalVarEntry* m_pPrevVariables;                // caller direction
    uint16_t m_lFunctionIndex;
    uint16_t m_lExitFunctionIndex;                   // 0x8000 = in message
    uint16_t m_lNextVariablesSize;
    uint16_t m_lAlignment;
};
// allocation size = max(FUNCTIONCONTROLLER::m_lDataSize, sizeof(LocalVarEntry))

struct _AsyncCall_Struct               // 0xC
{
    _AsyncCall_Struct* pNext;
    float m_fStoredNextRun;
    _LocalVarEntry* m_pLVE;
};

struct _MessageCue                     // 0xC + inline payload
{
    _MessageCue* m_pLast;
    _MessageCue* m_pNext;
    ZMSGID msg;
    // payload = this + 1 (GetData())
};

struct _SwitchStateStruct              // 0x24 (state-switch frame in compiled code)
{
    LocalVarEntry m_LVE;
    const STATECONTROLLER** m_pEnters;
    uint16_t m_lNumEnters;
    uint16_t m_lNumExits;
    const STATECONTROLLER* stateController;
    const STATECONTROLLER* pSS_pOldStateController;
};

union _SpecialScriptReturnType
{
    const STATECONTROLLER* m_pForkStateController;
    ScriptState* m_pForkReturnScriptState;
    const char* m_pScriptName;
    uint32_t m_lPriority;
};
```

### Thread flags (`ScriptFlags.h`, 16-bit `m_Flags`)

| Flag | Value | Meaning |
| --- | --- | --- |
| `ZSC_ASYNC_RESUME_CLEAR_MASK` | 0x0003 | mask cleared on async resume |
| `ZSC_FLAG_ASYNC_ACTIVE` | 0x0002 | executing an async-call frame |
| `ZSC_LOCAL_ASYNC_BLOCK_MASK` | 0x0005 | local async block mask |
| `ZSC_FLAG_ASYNC_WAITING` | 0x0004 | async call waiting to resume |
| `ZSC_ALIEN_ASYNC_BLOCK_MASK` | 0x0007 | alien async block mask |
| `ZSC_FLAG_HANDLING_MESSAGE` | 0x0008 | reentrant `Command()` guard |
| `ZSC_FLAG_ALIEN_CALL_ACTIVE` | 0x0010 | thread chain includes an alien call |
| `ZSC_FLAG_SKIP_MESSAGE_QUEUE` | 0x0020 | `Command()` queues instead of dispatching |
| `ZSC_FLAG_CLEAR_AFTER_ENTRY` | 0x0040 | cleared after every entry-point call |
| `ZSF_TERMINATE` | 0x0080 | `TerminateThread()` request |
| `ZSC_FLAG_SUPPRESS_MESSAGE_COMMAND` | 0x0100 | drop incoming commands |
| `ZSC_CONTINUE_AFTER_SLEEP_MASK` | 0x0600 | keep running after `m_tNextRun` update |
| `ZSC_FLAG_ALIEN_ASYNC_UNLINK` | 0x0800 | alien async unlink |
| `ZSC_MESSAGE_QUEUE_COUNT_MASK` | 0xF000 | queued message count (max 15) |

## DLL boundary tables

### `SCRIPTFUNCTIONS` (0x6C — engine → script)

Field order matches PC `InitializeScriptFunctions` (0x0043BE50):

```
0x00 SetForkStateController(const _STATECONTROLLER*)
0x04 GetForkThread() -> const _STATECONTROLLER*
0x08 Sleep(float)
0x0C StopThread(_ScriptState*)
0x10 TerminateThread(_ScriptState*)
0x14 ResumeThread(_ScriptState*)
0x18 CheckTimeout() -> bool
0x1C SendCommand(ZREF rSender, ZMSGID, void* pData, ZREF rTarget)
0x20 SendScriptCommand(ZREF rGeomTarget, ZMSGID, void* pData, int unused)
0x24 DebugPrint(const char* fmt, ...)
0x28 Pack(void*, uint32)
0x2C Unpack(void*, uint32)
0x30 Input(void*, uint32)
0x34 GetZDefine(const char* name, void* pData, uint32 size)
0x38 Alloc(uint32, const char* file, uint32 line)
0x3C AllocNM(...)
0x40 Free(void*)
0x44 FreeNM(void*)
0x48 RunNoBreak(_ScriptState*)
0x4C FindScriptStateByRef(ZREF, const char*) -> ZREF
0x50 GetAlienVirtualTableEntry(ZREF, int32 entryNr) -> const _FUNCTIONCONTROLLER*
0x54 GetAlienScriptState(ZREF) -> _ScriptState*
0x58 GetRootScriptStateRef() -> ZREF
0x5C Memcpy(void* dst, void* src, uint32)
0x60 Memset(void* dst, uint8, uint32)
0x64 GetPriority(_ScriptState*) -> int32
0x68 SetPriority(_ScriptState*, int32)
```

Engine binds every slot with static `ScriptEngine::*` methods or `SF_*` free
functions. `Free`/`FreeNM` intentionally keep PS2-style signatures; PC debug
call-site variants are noted as comments in the header.

`INTERNALSCRIPTFUNCTIONS` (4 bytes): one `RunningThread` slot. Attach clears
the DLL's slot and points the engine's `ISF.RunningThread` **at the DLL's
struct itself** so `SetRunningThread` writes into the DLL.

### `ZScriptImportTable` ordinals (engine-side names)

| Ordinal | Engine enum | DLL export name | Content |
| --- | --- | --- | --- |
| 1 | `Z_SF` | `SF` | `SCRIPTFUNCTIONS*` filled by engine at attach |
| 2 | `Z_ISF` | `ISF` | `INTERNALSCRIPTFUNCTIONS*` running-thread slot |
| 3 | `Z_Scripts` | `Scripts` | null-terminated `SCRIPTCREATOR**` list; `[0]` = count, creators from `[1]` |
| 4 | `Z_ScriptImports` | `ScriptImports` | `void*[0x2CC]` import block overwritten with `ScriptInterfaces` |

Verified against **PC_Hideout** (2026-09-17): exports are exactly
`SF @0x10045A80 (1)`, `ISF @0x10045AEC (2)`, `Scripts @0x10045A10 (3)`,
`ScriptImports @0x10045B00 (4)`, plus `DllEntryPoint`. `Scripts[0] = 0x3B4`
(948 creators); the `.rdata` block `off_10044738` holds
`{SF, ISF, Scripts, ScriptImports}` in ordinal order. Export names appear
without the `Z_` prefix in the binary — the prefix only exists in engine
enum/source.

### ScriptInterfaces global (engine side)

`Globals.h/.cpp`: `ScriptInterfaces` is `void**` at PC `0x007F2D20`, spanning
`[0x007F2D20, 0x007F3850)` = `0x2CC` pointers, ending exactly at `Uniques`
(`0x007F3850`). It is filled at game startup by the `ScriptInterface` module's
static initializer (see SCRIPT_INTERFACE.md) and copied verbatim into each
attached DLL's `ScriptImports` block.

## Attachment sequence (`ScriptEngine::AttachSceneScripts`)

Path: scene COM value `ScriptCModule` → `scriptcs/_gamerelease/<module>.dll`
(or `OverrideScriptPath`), via `g_pSysFile->ConvertFilename`, `LoadLibraryA`.
Reference-counted (`lScriptLoadedCount`); failure is cached in
`g_bScriptLoadResult`. On the first successful attach:

1. `znew_placement<ZMallocSimple>(&g_ScriptAllocator)` +
   `AddBlock(g_ScriptMemory, 0x80000)` — 512 KB script heap.
2. `InitializeScriptFunctions((SCRIPTFUNCTIONS*)GetProcAddress(hMod, Z_SF))` —
   binds all 27 slots above.
3. Cross-link running thread: `pISF->RunningThread = nullptr;
   ISF.RunningThread = (_ScriptState*)pISF;`
4. `ScriptsPtr = (SCRIPTCREATOR**)GetProcAddress(hMod, Z_Scripts)`.
5. Copy `ScriptInterfaces[i]` → `pScriptImports[i]` for `i in [0, 0x2CC)`.
6. Run `Initialize()` (`+0x1C`) on every creator starting at `ScriptsPtr[1]`.

`DetachSceneScripts()` decrements the count and `FreeLibrary` at zero.
`ZScriptC::~ZScriptC` pairs them.

## Thread hosting & float protocol

`ZScriptC` (`0x38`, `CBaseEvent<ZGEOM>`) members: union
`{ const _SCRIPTCREATOR* m_pScriptCreator; ScriptStateInfo*
m_pInitialScriptStateInfo }` at `+0x30`, `void* m_pStoredDataBlock` at `+0x34`.
Properties: `Name` ("ScriptC", get/set builds the DLL path and attaches).
Virtual surface: `~ZScriptC`, `PostSave`, `PostLoad`, `GetProperties`, `Init2`,
`PostInit`, `PreSaveGame`, `FrameUpdate`, `Command`, `SchedUpdate`.

Method surface (used by scripts indirectly, by engine code directly):
`CreateScript(creator)`, static `FindScript(name)`, `ForkState(stateController)`,
`GetRootScriptState`, `IsValidThread`, `TerminateScript`, `FreeThread`,
`GetNrThreads`, `GetSchedEvent`, `GetNextRun`, `GetPriority`,
`LoadSaveGame(stream)`, static `NukeAndRestart`.

Each frame `ZScheduledUpdate` dispatches to `ZScriptC::SchedUpdate`, which sets
`ISF.RunningThread` and `s_CurrentRootScriptCRef`, drains the message cue,
resolves async/alien calls, then calls the current frame's entry point:

```cpp
float result = pFunctionController->m_pEntryPoint(pState);
```

The float return is the **yield protocol** between compiled script code and the
scheduler — every ScriptCS function ultimately returns through it:

| Value | Meaning |
| --- | --- |
| `-7` | Yield: rerun next scheduler pass (`m_tNextRun = 0`) |
| `-6` | Set priority from `m_SpecialScriptReturnType.m_lPriority` (clamped ≤15) |
| `-5` | Continue without sleeping |
| `-4` | Fork: `ForkState(m_SpecialScriptReturnType.m_pForkStateController)` |
| `-3` | Continue after timeout check (used by `ForkStateFree`) |
| `-2` | Terminate thread (`TerminateScript()`) |
| `>= 0` | Sleep: `m_tNextRun.secs = result * kTicksPerSecond` |

Forking: `ForkState` duplicates the runtime image (fresh `ScriptState` sharing
`m_pScriptVariables`, new state block, new `LocalVarEntry` for the target
`m_pRun`, optional enter chain + `ForkStateFree` cleanup frame returning `-3`).
`ForkStateFree_FUNCTIONCONTROLLER` is a named `.data` singleton
(`g_pForkStateFree` → it); save encoding maps it to the special `-1`
(0x7FFFFFFF) script-code offset.

Per-pass timeout: 5000 timestamp ticks inside one `SchedUpdate` pass
(`SF_CheckTimeout()`: `TimeStampCounter − g_lCurrentScriptMaxTime >= 5001`)
forces a yield unless `ZSC_CONTINUE_AFTER_SLEEP_MASK` is set.

Thread control (engine side, via `m_pThreadInfo`): `StopThread` = `Sleep(-1)`,
`ResumeThread` = `Sleep(0)`, `TerminateThread` sets `ZSF_TERMINATE`,
`SetPriority` clamps to ≤15.

## Message system

- Names compile into a trie: `ZScriptC_ZMessage { const char* m_pKeys;
  ZScriptC_Indexes* m_pIndexes }`, `ZScriptC_Indexes { m_lNextIndex,
  m_lUniqueText, m_lZMsgID }`. Static tables: `ZScriptC_ZMessages[643]`
  (PC `0x007596A0`), `Uniques[806]` (PC `0x007F3850`), installed once by
  `ZScriptCComponent` ctor (`InstallScriptMessages`) / dtor
  (`DestroyScriptMessages`).
- `GetRegisterZMessageID(name)` walks the trie (returns 0 when absent);
  `GetZMessageName(id)` rebuilds the name into a 256-byte scratch buffer.
- Script-visible ids start at `0x800`; payload sizes come from the PC dump
  `ZMSG_Sizes[786]` in `ZScriptC.cpp`. Known named constants live in
  `ScriptMessages.h` (e.g. `MSG_LevelControl_HitmanDied = 0x081D`,
  `MSG_FightController_* = 0x094F..0x0951`).
- `ZScriptC::Command(msg, data)` runs for every thread of the event: with
  `ZSC_FLAG_SKIP_MESSAGE_QUEUE` the message is appended to the thread's
  `MessageCue` (inline payload after the 0xC header, duplicate-free, max 15);
  otherwise dispatched immediately: current state's `ProcessMessage`, then the
  creator chain. Nested command dispatch uses `g_pMessageAllocator`, a nested
  4 KB `ZOffsetAlloc` (see memory below), ref-counted in `ZScriptC.cpp`.
- Outbound: `ScriptEngine::SendCommand` routes through
  `ZGEOM::SendCommand`; `SendScriptCommand(geotarget, msg, data, senderref)`
  walks the target geom's event list and `Command()`s every `ScriptC` event;
  `ScriptSendCommand(pGeom, msg, pData)` is the pointer-based helper with the
  same semantics (handles both direct-id and list-entity buffer forms).

## Memory model

- Fixed 512 KB pool `g_ScriptMemory` (`char[0x80000]`, PC
  `[0x008289C8, 0x008A89C8)`, static_assert-enforced) under
  `g_ScriptAllocator` (`ZMallocSimple`). All thread images come from here.
- `ScriptEngine::Alloc` zeroes blocks; with `g_pMessageAllocator` active it
  serves from the nested 4 KB pool with a **4-byte size header** (payload at
  header+4; `Free` detects pool membership by offset in `(0, 0x1000)`).
- `AllocNM`/`FreeNM` bypass the message pool. `AllocSize` is used by save/load
  to know block extents. `GetFreeScriptMemory` reports heap headroom.
- Never allocate script runtime data from the host heap.

## Save / Load (pointer-graph serialization)

Script save converts the entire thread image because it is pointer-based.

Prepare (`PreSaveGame` → `PrepareSave`, `ScriptSaveLoad.cpp`): lazily creates
`g_pSaveTable` (`SaveTableVector` of `SaveRefEntry`, index 0 = null sentinel)
and `g_pSavedPointersMap` (`pointer → index`), then walks each thread:
`ScriptState` → alien chain → `LocalVarEntry` chain → async calls → message
cue → state/script variable blocks. `PrepareData` interprets
`m_lStringOffsets` ranges (`SRT_VARIABLES` strings, `SRT_SCRIPTSTATE` thread
refs — invalid threads nulled, `SRT_EVENTREF` base+offset pairs,
`SRT_ASYNCCALL_STRUCT` enter arrays at offset +20).

Fixup (`FixupSaveTable(bEncode)` + `FixupData`): script-code pointers
(`FUNCTIONCONTROLLER`, `STATECONTROLLER`, vtables, creators) encode as
DLL-relative offsets with the high bit via `GetOffsetInScriptCode`
(ForkStateFree → `0xFFFFFFFF`, decoded via `0x7FFFFFFF`); heap pointers become
save-table indices. `ZScriptC::PostSave` streams the descriptor array, raw
payloads, `SAVEGAMESTATICS` blobs (`SGST_RAW/STRING/THREAD/STATEREF/EVENTREF`),
then per-thread `(root index, current index, priority, sleep time)`;
`FixupSaveTable(true)` restores live pointers afterwards.

Load (`ZScriptC::LoadSaveGame`): entries allocated from the script heap,
payloads read, `FixupLoadedData`/`ResolveScriptCodeRef` resolve indices through
`s_pLoadEntries` and script-code offsets through `GetAddressInScriptCode`;
threads rebuilt via scheduler forks, priority and remaining sleep restored.

`SaveRefType` tags: `SRT_NULL/ VARIABLES/ SCRIPTSTATE/ SCRIPTVARIABLES/
STATEVARIABLES/ DYNSTRING/ ASYNCCALL_STRUCT/ ENTERS/ MESSAGECUE/ EVENTREF`.

## Globals inventory (PC addresses, from Globals.cpp — do not fabricate others)

| Global | PC address | Notes |
| --- | --- | --- |
| `g_lCurrentScriptMaxTime` | 0x008289B0 | timeout budget for current pass |
| `g_pZScriptCDataBlock` | 0x009A2670 | PostLoad stream cursor for `Input` |
| `g_pZSC` | 0x009A2664 | current `ZScheduledScript` |
| `ISF` | 0x008289BC | engine-side running-thread link struct |
| `g_SF` | 0x008289C0 | filled `SCRIPTFUNCTIONS*` (points into DLL) |
| `lScriptLoadedCount` | 0x008A89D8 | DLL refcount |
| `ScriptsPtr` | 0x008A89D4 | DLL creator list |
| `g_pScripts` | 0x008289B8 | HMODULE of script DLL |
| `ZScriptC_ZMessages` | 0x007596A0 | message trie [643] |
| `g_pZScriptC_Messages` | 0x008A89CC | installed trie head |
| `Uniques` | 0x007F3850 | unique-suffix table [806] |
| `ScriptInterfaces` | 0x007F2D20 | `void*[0x2CC]` copied to DLL on attach |
| `g_pZScriptC_Uniques` | 0x008A89C8 | installed uniques |
| `g_bScriptLoadResult` | 0x008ACA2C | cached attach result |
| `g_ScriptAllocator` | 0x008A89E0 | `ZMallocSimple` |
| `g_ScriptMemory` | 0x008289C8 | 512 KB pool head (spans to 0x8A89C8) |
| `s_CurrentRootScriptCRef` | 0x009A2668 | current root ScriptC ref |
| `g_pMessageAllocator` | 0x008A89D0 | nested 4 KB message pool |
| `g_pCurrentSS` | 0x009A266C | current `ScriptState` |
| `s_pLoadEntries` etc. | 0x009A26xx | load/save state (`s_lObjectSaveCount`, `s_bSaving`, `s_bLoading`, `s_pCurrentSaveGameObject`, `g_pSavedPointersMap`, `g_pSaveTable`, `s_pStringMap`) |
| `g_pForkStateFree` | 0x007FD7FC | → `ForkStateFree_FUNCTIONCONTROLLER` |

## Script-facing vocabulary (`Common.h`)

`eDirection`, `eAttackStyle`, `eAudioEvent(22)`, `eAudioType(70)`,
`eEventCameraPosition`, `eEventCameraType`, `eGQCTaskType`, `eMeta(20)`,
`eScriptPathType`, `eWeaponType`, `eMoveSet(16)`, `v3 = ZVector3`,
`sCover { position, facing, movedirection, distance, indx }`,
`sGQCTask { type, rtarget }`, `ZSC_EVENT { ZMSGID zmsg; void* pData }`,
`anim = uint16_t`, `STATEREF = const void**`.
`Levels/Base.h` holds level-specific enums (`e03PhotoTouristType`, `eBarkType`,
`eDialogControl`, `eDifficulty`); remaining exports tracked by the
`scriptc.cpp` search-TODO there. Script-visible enum constants (the
`ZDEFINE_*` data) are consumed through `SCRIPTFUNCTIONS::GetZDefine` — on PC it
zero-fills (the DLL carries compiled-in values); no exe-side define registry
exists to mirror.

## Dependency edges

- Depends on: `EventBase` (`ZScheduledScript/Update/Event`,
  coroutine `m_pStoredStack`), `Geom/ZGEOM` + event lists, `ZEventBuffer`,
  `ZSTL` (`ZMallocSimple`, `ZOffsetAlloc`, `TIMETYPE`, STLport maps for save),
  `Filesystem/ZSysFile`, `System/ZSysInterface`, `RTP` properties,
  `Serializer`, `ZMessageResolver`.
- Dependents: `ZScriptCComponent` (singleton install), BloodMoney game code
  calling `ScriptSendCommand`/engine APIs, and the `ScriptInterface` static
  library publishing `ScriptInterfaces` (see next doc).
