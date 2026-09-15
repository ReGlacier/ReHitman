# Glacier Script Engine

This directory contains Glacier's script engine (the `ScriptC` event family). The
engine runs per-scene script code that ships in a separate script DLL: each
`ZGEOM` in the scene can host a `ZScriptC` event, and that event owns one or more
cooperatively scheduled script threads driven by the event scheduler in
`source/EventBase`.

The system has six main parts:

1. `ScriptEngine` is the static host API: DLL attach/detach, script heap,
   thread control, message name resolution, and script-code address mapping.
2. `ZScriptC` is the per-geom event: it creates, runs, messages, forks,
   saves, and destroys script threads.
3. `SCRIPTFUNCTIONS` / `INTERNALSCRIPTFUNCTIONS` are the call tables exchanged
   between the engine and the script DLL at attach time.
4. `SCRIPTCREATOR`, `STATECONTROLLER`, and `FUNCTIONCONTROLLER` are the static
   script metadata produced by the script compiler and loaded from the DLL.
5. `ScriptState` and `LocalVarEntry` are the runtime thread image: call frames,
   state variables, async calls, and the message cue.
6. `ZScheduledScript` / `ZScheduledUpdate` / `ZScheduledEvent` (EventBase) form
   the cooperative thread scheduler that actually runs `SchedUpdate()`.

## Big Picture

```text
Scene stream                        Script DLL (LoadLibrary)
    |                                   |
    v                                   v
ZScriptC::SetName()  ----AttachSceneScripts()---->  Z_SF / Z_ISF / Z_Scripts
    |                                                     |
    v                                                     v
FindScript(name) -> SCRIPTCREATOR ----Initialize()-->  engine fills
    |                                                  ScriptInterfaces
    v
ZScriptC::PostLoad() -> CreateScript() -> ScriptState + ScriptStateInfo
    |
    v
ZScheduledUpdate::AddEvent() -> ZScheduledScript (thread)
    |
    v
ZScheduledUpdate::ScheduleEvents() -> ZScheduledScript::Run()
    -> ZEventBase::Call(0x1000) -> ZScriptC::SchedUpdate()
    -> FUNCTIONCONTROLLER::m_pEntryPoint(ScriptState*)  (compiled script code)
```

## Attaching The Script DLL

Scripts for a scene live in one DLL resolved from the scene COM value
`ScriptCModule`. `ZScriptC::SetName()` builds the path
(`scriptcs/_gamerelease/<module>.dll`, or `OverrideScriptPath` when set) and
calls `ScriptEngine::AttachSceneScripts()` once per module. Attaching is
reference-counted through `lScriptLoadedCount`; `DetachSceneScripts()` frees
the library when the count reaches zero.

Attach performs four bindings:

```cpp
// 1. Script heap: 512 KB fixed pool from .data.
g_ScriptAllocator.AddBlock(g_ScriptMemory, 0x80000u);

// 2. Fill the DLL's SCRIPTFUNCTIONS import table (ordinal Z_SF).
InitializeScriptFunctions((SCRIPTFUNCTIONS*)GetProcAddress(hMod, MAKEINTRESOURCEA(Z_SF)));

// 3. Cross-link the running-thread slot (ordinal Z_ISF).
auto* pISF = (INTERNALSCRIPTFUNCTIONS*)GetProcAddress(hMod, MAKEINTRESOURCEA(Z_ISF));
pISF->RunningThread = nullptr;
ISF.RunningThread = reinterpret_cast<_ScriptState*>(pISF);

// 4. Grab the creator list (Z_Scripts) and copy the 0x2CC-entry
//    ScriptInterfaces table into the DLL's import block (Z_ScriptImports),
//    then run every creator's Initialize().
for (int i = 1; ScriptsPtr[i]; ++i)
    ScriptsPtr[i]->Initialize();
```

The four DLL ordinals are defined by `ZScriptImportTable`:

| Ordinal | Export | Meaning |
| --- | --- | --- |
| `Z_SF` (1) | `SCRIPTFUNCTIONS*` | Engine-to-script call table, filled by `InitializeScriptFunctions()` |
| `Z_ISF` (2) | `INTERNALSCRIPTFUNCTIONS*` | Script-to-engine running-thread slot |
| `Z_Scripts` (3) | `SCRIPTCREATOR**` | Null-terminated creator list (index 0 is the list head) |
| `Z_ScriptImports` (4) | `void**` | DLL import table overwritten with `ScriptInterfaces` |

`SCRIPTFUNCTIONS` is the complete runtime API the compiled script uses:
`Sleep`, `StopThread`, `TerminateThread`, `ResumeThread`, `SendCommand`,
`SendScriptCommand`, `Alloc`/`Free` (+ NM variants), `SetForkStateController`,
`GetForkThread`, `GetAlienScriptState`, `GetAlienVirtualTableEntry`,
`FindScriptStateByRef`, `GetRootScriptStateRef`, `RunNoBreak`, `DebugPrint`,
`CheckTimeout`, `Pack`/`Unpack`/`Input`, `GetZDefine`, `Memcpy`, `Memset`,
`GetPriority`, and `SetPriority`. The engine-side implementations are the
static `ScriptEngine` methods plus the `SF_*` helpers in `SF.cpp`.

## Script Metadata

Compiled scripts are described by three read-only structures:

```cpp
struct _SCRIPTCREATOR            // one per script, in the DLL
{
    const char* m_pName;
    int32_t m_lScriptVariablesSize;      // script-global data block size
    int32_t m_lStateVariablesSize;       // per-thread state data block size
    const STATECONTROLLER* m_pStateController;
    const SCRIPTCREATOR* m_pParentCreator;   // inheritance chain
    const void* m_pStatesVirtualTable;
    ProcessMessage_t ProcessMessage;     // script-level message handler
    VoidFunction_t Initialize;           // run once at attach
    const SAVEGAMESTATICS* m_pSaveGameStatics;
    VoidFunction_t Imports;              // consumes the PostLoad data block
    VoidFunction_t StaticImports;
    VoidFunction_t UnpackResources;
    VoidFunction_t UnpackStaticResources;
    const SCRIPTIMPORT* m_pImports;      // SIT_END-terminated import table
};

struct _STATECONTROLLER          // one per state
{
    const FUNCTIONCONTROLLER* m_pRun;    // state body
    const FUNCTIONCONTROLLER* m_pEnter;  // one-shot enter handler
    const FUNCTIONCONTROLLER* m_pDestroy;
    void (*ProcessMessage)();            // state-level message handler
    const void* m_pFunctionsVirtualTable;
    const uint16_t m_lLevel;
    const uint16_t m_lScriptLevel;
    const STATECONTROLLER* m_pParent;
    const char* m_pName;
    uint16_t* m_lStringOffsets;          // save/load fixup table
};

struct _FUNCTIONCONTROLLER       // one per callable
{
    EntryPoint_t m_pEntryPoint;          // void/float fn(ScriptState*)
    uint16_t m_lInputSize;
    uint16_t m_lDataSize;                // local frame size (>= LocalVarEntry)
    const char* m_pName;
    uint16_t* m_lStringOffsets;          // save/load fixup table
};
```

`m_lStringOffsets` is a list of `(type, startOffset, endOffset)` triples
terminated by a zero type. The types are `SaveRefType` values and tell the
save system which dword fields inside a data block are pointers that need
fixup.

## Runtime Thread Image

Each running script is a `ScriptStateInfo` pair (root + current state)
stored as the `ZScheduledScript` user data:

```cpp
struct ScriptStateInfo
{
    ScriptState* m_pRootScriptState;     // scheduler-visible thread root
    ScriptState* m_pCurrentScriptState;  // innermost active state
};

struct _ScriptState
{
    void* m_pScriptVariables;            // shared script-global block
    const SCRIPTCREATOR* m_pCreator;
    ZREF m_rThis;                        // owning ZGEOM ref
    LocalVarEntry* m_pVariables;         // current call frame
    void* m_pStateVariables;             // per-thread state block
    const STATECONTROLLER* m_pStateController;
    const STATECONTROLLER* m_pPreviousStateController;
    const STATECONTROLLER* m_pNextStateController;
    ZMSGID m_msgWaitForEvent;
    uint16_t m_Flags;                    // ZSC_FLAG_* / ZSF_* bits
    AsyncCall_Struct* m_pAsyncCall;      // pending async-call queue
    AsyncCall_Struct* m_pAsyncCallLast;
    const SCRIPTCREATOR* m_pMessageHandler;
    ScriptState* m_pAlienCall;           // cross-thread call chain
    const void* m_pFunctionsVirtualTable;
    void* m_pThreadInfo;                 // owning ZScheduledScript
    MessageCue* m_pMessageCue;           // queued messages
};
```

Call frames are `LocalVarEntry` nodes in a doubly linked list. The 0x14-byte
header is followed by the function's local data, so the allocation size is
`max(m_pRun->m_lDataSize, sizeof(LocalVarEntry))`:

```cpp
struct _LocalVarEntry
{
    const FUNCTIONCONTROLLER* m_pFunctionController; // null = free frame
    LocalVarEntry* m_pNextVariables;                 // callee direction
    LocalVarEntry* m_pPrevVariables;                 // caller direction
    uint16_t m_lFunctionIndex;
    uint16_t m_lExitFunctionIndex;                   // 0x8000 = in message
    uint16_t m_lNextVariablesSize;
    uint16_t m_lAlignment;
};
```

Thread flags (`ScriptFlags.h`) control scheduling:

| Flag | Meaning |
| --- | --- |
| `ZSC_FLAG_ASYNC_ACTIVE` (0x0002) | Executing an async-call frame |
| `ZSC_FLAG_ASYNC_WAITING` (0x0004) | Async call is waiting to resume |
| `ZSC_FLAG_HANDLING_MESSAGE` (0x0008) | Reentrant `Command()` guard |
| `ZSC_FLAG_ALIEN_CALL_ACTIVE` (0x0010) | Thread chain includes an alien call |
| `ZSC_FLAG_SKIP_MESSAGE_QUEUE` (0x0020) | `Command()` queues instead of dispatching |
| `ZSC_FLAG_CLEAR_AFTER_ENTRY` (0x0040) | Cleared after every entry-point call |
| `ZSF_TERMINATE` (0x0080) | `TerminateThread()` request |
| `ZSC_FLAG_SUPPRESS_MESSAGE_COMMAND` (0x0100) | Drop incoming commands |
| `ZSC_CONTINUE_AFTER_SLEEP_MASK` (0x0600) | Keep running after `m_tNextRun` update |
| `ZSC_MESSAGE_QUEUE_COUNT_MASK` (0xF000) | Queued message count (max 15) |

## Scheduling

`ZScheduledUpdate` owns 16 running queues plus one time-sorted sleeping queue
(`ZScheduledEventList`, an intrusive circular list). Each frame
`ScheduleEvents()` wakes due sleepers, then walks priorities with a leaky
bucket (`DoesPriorityRun()`, 0x100-tick threshold, halved per level) until the
cycle budget `m_lMaxCyclesToRun` is spent.

A script thread runs like this:

1. `ZScheduledScript::Run()` attaches itself to its `ZScriptC` event and calls
   `Call(0x1000, ...)`, which dispatches to `ZScriptC::SchedUpdate()`.
2. `SchedUpdate()` sets `ISF.RunningThread` and `s_CurrentRootScriptCRef`,
   drains the message cue, resolves pending async calls (including alien
   calls), then invokes the current frame's entry point:
   `float result = pFunctionController->m_pEntryPoint(pState)`.
3. The float result is a protocol between compiled script code and the
   scheduler:

| Result | Meaning |
| --- | --- |
| `-7` | Yield: rerun next scheduler pass (`m_tNextRun = 0`) |
| `-6` | Set priority from `m_SpecialScriptReturnType.m_lPriority` (clamped to 15) |
| `-5` | Continue without sleeping |
| `-4` | Fork: `ForkState(m_SpecialScriptReturnType.m_pForkStateController)` |
| `-3` | Continue after the timeout check (used by `ForkStateFree`) |
| `-2` | Terminate the thread (`TerminateScript()`) |
| `>= 0` | Sleep: `m_tNextRun.secs = result * TIMETYPE::kTicksPerSecond` |

4. `ScriptEngine::Sleep(fTime)` reaches the thread through `g_pZSC` and simply
   calls `ZScheduledScript::Sleep()`, which moves the thread between the
   running and sleeping queues.

Per-pass timeout: when the script spends 5000 timestamp ticks inside one
`SchedUpdate()` pass (`SF_CheckTimeout()` / `CheckScriptTime()`), the thread
yields with `m_tNextRun.secs = 0` unless `ZSC_CONTINUE_AFTER_SLEEP_MASK` is
set.

## Forking And Thread Control

`ZScriptC::ForkState(pController)` duplicates the runtime image: a fresh
`ScriptState` sharing `m_pScriptVariables` with the root, a new state block, a
new `LocalVarEntry` for the target `m_pRun`, and an optional one-shot enter
chain (`enter` frame + the `ForkStateFree` cleanup frame, which frees both
frames and returns `-3`). The scheduler thread is forked last via
`ZScheduledEvent::Fork()`, which links threads through `m_pThread`.

Engine-side thread control (all exported through `SCRIPTFUNCTIONS`):

```cpp
ScriptEngine::StopThread(pState);       // Sleep(-1)  - suspend
ScriptEngine::ResumeThread(pState);     // Sleep(0)   - wake into running queue
ScriptEngine::TerminateThread(pState);  // set ZSF_TERMINATE
ScriptEngine::SetPriority(pState, 6);   // clamped to 0..15
```

`ForkStateFree_FUNCTIONCONTROLLER` is a named `.data` singleton;
`g_pForkStateFree` points at it and the save system encodes it as the special
offset `-1` (see below).

## Messages

Message names are compiled into a trie: `ZScriptC_ZMessage` nodes hold a key
string and per-key `ZScriptC_Indexes` records (`m_lNextIndex` for trie
descent, `m_lUniqueText` for shared suffixes in the `Uniques` table,
`m_lZMsgID` for the terminal id). The engine installs the static tables once
via `ZScriptCComponent`:

```cpp
ZScriptCComponent::ZScriptCComponent()  { ScriptEngine::InstallScriptMessages(ZScriptC_ZMessages, Uniques); }
ZScriptCComponent::~ZScriptCComponent() { ScriptEngine::DestroyScriptMessages(); }
```

`GetRegisterZMessageID(name)` walks the trie; `GetZMessageName(id)` rebuilds
the name into a 256-byte scratch buffer.

Incoming commands arrive through `ZScriptC::Command(msg, data)` for every
thread of the event:

- If `ZSC_FLAG_SKIP_MESSAGE_QUEUE` is set, the message is appended to the
  thread's `MessageCue` (payload stored inline after the 0xC-byte header,
  sizes come from the `ZMSG_Sizes` dump, ids start at 0x800). Duplicates are
  dropped and the cue is capped at 15 entries.
- Otherwise the message is dispatched immediately: first the current state's
  `ProcessMessage`, then the creator chain's `ProcessMessage`
  (`m_pMessageHandler` walks `m_pParentCreator`).

`Command()` runs with a nested `g_pMessageAllocator` (a 0x1000-byte
`ZOffsetAlloc` used by `ScriptEngine::Alloc` for message-scoped allocations);
it is created on the first dispatch and destroyed when the last nested
command exits. `ScriptEngine::SendCommand()` / `SendScriptCommand()` are the
outbound paths and route through `ZGEOM::SendCommand()` and the geom's event
list.

## Memory Model

All script runtime memory comes from the fixed 512 KB heap
`g_ScriptMemory`, managed by `g_ScriptAllocator` (`ZMallocSimple`).
`ScriptEngine::Alloc`/`AllocNM` zero the block; `Free`/`FreeNM` return it.
During message dispatch, `Alloc` transparently uses the nested 4 KB
`g_pMessageAllocator` pool (blocks carry a 4-byte size header and are freed
by offset). `GetFreeScriptMemory()` reports the remaining heap.

```cpp
auto* pState = (ScriptState*)ScriptEngine::Alloc(sizeof(ScriptState), __FILE__, __LINE__);
ScriptEngine::Free(pState);
```

## Save / Load

Script save is pointer-based, so the engine converts the whole thread image
into a portable form. `ScriptSaveLoad.cpp` implements the two passes:

- **Prepare** (`PreSaveGame` -> `PrepareSave`): walks each thread's
  `ScriptState`, `LocalVarEntry` chain, async calls, message cue, and variable
  blocks, appending `SaveRefEntry` records to `g_pSaveTable` and memorizing
  `pointer -> index` in `g_pSavedPointersMap`.
- **Fixup** (`FixupSaveTable(false)`): rewrites pointers in place.
  Script-code pointers (`FUNCTIONCONTROLLER`, `STATECONTROLLER`, vtables)
  become DLL-relative offsets with the high bit set via
  `ScriptEngine::GetOffsetInScriptCode()`; heap pointers become save-table
  indices. `m_lStringOffsets` tables describe which fields inside each data
  block need conversion (`FixupData`).

`ZScriptC::PostSave()` then streams the descriptor array, the raw payloads,
and the per-script `SAVEGAMESTATICS` blobs, followed by each thread's
`(root index, current index, priority, sleep time)`. After streaming,
`FixupSaveTable(true)` restores live pointers.

Loading (`LoadSaveGame()`) is the mirror: entries are allocated from the
script heap, payloads are read, then `FixupLoadedEntry` resolves indices back
through `s_pLoadEntries` and script-code offsets through
`GetAddressInScriptCode()` (offset `0x7FFFFFFF` maps back to the
`ForkStateFree` singleton). Threads are rebuilt by forking
`ZScheduledScript`s and restoring priority and remaining sleep time.

`SaveRefType` tags each entry: `SRT_VARIABLES` (LocalVarEntry),
`SRT_SCRIPTSTATE`, `SRT_SCRIPTVARIABLES`, `SRT_STATEVARIABLES`,
`SRT_DYNSTRING` (interned strings), `SRT_ASYNCCALL_STRUCT`, `SRT_ENTERS`,
`SRT_MESSAGECUE`, and `SRT_EVENTREF` (offset+base encoded event pointers).
`SAVEGAMESTATICS` describe raw (`SGST_RAW`), string, thread, state-ref, and
event-ref statics stored per script in the DLL.

## Coroutine Stack Switching (x86)

Generic `ZScheduledEvent::Run()` preserves the callee's native stack so a
thread can yield in the middle of C++ code. `EnterSchedulerMode()`
(`ScriptCoroutineScheduler.asm`) saves all registers, copies a previously
stored stack image back onto the current stack, frees the stored image, and
resumes; when there is nothing to restore it jumps to `g_pFunction`
(`ZScheduledEvent::EnterFunction` -> `SchedUpdate()`). The stored image lives
in `m_pStoredStack` / `m_lStoredStackLength` on the event. `ZScheduledScript`
does not use this path for its own dispatch - scripts yield through the float
return protocol instead - but the machinery is shared by all scheduled
events.

## Basic Usage

```cpp
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/ZScriptC.h>

// Attach the scene script module (ref-counted).
if (Glacier::ScriptEngine::AttachSceneScripts("scriptcs/_gamerelease/M01.dll"))
{
    // Resolve a compiled script by name ( '/' and '\\' fold to '_').
    const Glacier::SCRIPTCREATOR* pCreator = Glacier::ZScriptC::FindScript("Guard_Main");
}

// Message name <-> id.
uint16_t msgId = Glacier::ScriptEngine::GetRegisterZMessageID("Alarm");
const char* name = Glacier::ScriptEngine::GetZMessageName(msgId);

// Script heap.
void* pBlock = Glacier::ScriptEngine::Alloc(128, __FILE__, __LINE__);
Glacier::ScriptEngine::Free(pBlock);

// Thread control from engine code.
Glacier::ScriptEngine::SetPriority(pState, 6);
Glacier::ScriptEngine::StopThread(pState);
Glacier::ScriptEngine::ResumeThread(pState);
Glacier::ScriptEngine::TerminateThread(pState);

// Outbound command to another geom's ScriptC events.
Glacier::ScriptEngine::SendScriptCommand(rTargetGeom, msgId, pData, rSenderGeom);
```

## Header Reference

| Header | Responsibility |
| --- | --- |
| `ScriptEngine.h` | Static host API: DLL attach/detach, heap, threads, messages, address mapping |
| `ZScriptC.h` | Per-geom script event: create/run/message/fork/save threads |
| `ZScriptCComponent.h` | Global component installing the message name tables |
| `SCRIPTFUNCTIONS.h` | Engine-to-script call table layout (0x6C bytes) |
| `INTERNALSCRIPTFUNCTIONS.h` | Script-to-engine running-thread slot |
| `SCRIPTCREATOR.h` | Compiled script descriptor loaded from the DLL |
| `STATECONTROLLER.h` | Compiled state descriptor (run/enter/destroy/messages) |
| `FUNCTIONCONTROLLER.h` | Compiled function descriptor (entry point + frame size) |
| `ScriptState.h` / `ScriptStateInfo.h` | Runtime thread image and root/current pair |
| `LocalVarEntry.h` / `MsgLocalVarEntry.h` | Call frame header and message frame |
| `AsyncCall_Struct.h` | Pending async-call queue node |
| `MessageCue.h` | Queued message node with inline payload |
| `ScriptFlags.h` | `m_Flags` bit definitions |
| `SCRIPTIMPORTS.h` / `SCRIPTIMPORTTYPE.h` | PostLoad import descriptors and packed types |
| `SAVEGAMESTATICS.h` / `SAVEGAMESTATICSTYPE.h` | Per-script saved statics descriptors |
| `SaveRefEntry.h` | Save-table entry and `SaveRefType` tags |
| `ScriptSaveLoad.h` | Prepare/fixup passes for save and load |
| `SF.h` | Engine-side `SCRIPTFUNCTIONS` helpers (timeout, print, pack, memcpy) |
| `SpecialScriptReturnType.h` | Side channel for fork/priority returns |
| `SwitchStateStruct.h` | State-switch frame used by compiled state code |
| `ZScriptC_ZMessage.h` | Message name trie nodes |
| `ZScriptImportTable.h` | DLL export ordinals |
| `Globals.h` | Global engine state (heap, tables, current thread, save state) |
| `Common.h` | Shared script-facing enums and small structs |

## Important Rules

- Do not change field order or virtual method order. `ScriptState`,
  `LocalVarEntry`, `SCRIPTCREATOR`, `STATECONTROLLER`, `FUNCTIONCONTROLLER`,
  `SCRIPTFUNCTIONS`, `ZScriptC`, and the EventBase classes all have verified
  binary-compatible sizes (`RE_VERIFY_SIZE`).
- Never allocate script runtime data from the host heap. Use
  `ScriptEngine::Alloc`/`Free` (or the NM variants) so save/load fixup and
  `AllocSize()` keep working.
- `AttachSceneScripts` is ref-counted; always pair it with
  `DetachSceneScripts` (`ZScriptC::~ZScriptC` does this).
- Keep the `SCRIPTFUNCTIONS` layout in sync with the DLL side; the table is
  copied blindly into the script module at attach time.
- Script-code pointers inside saved data must round-trip through
  `GetOffsetInScriptCode`/`GetAddressInScriptCode`; the `ForkStateFree`
  singleton is the special `-1` offset and must never be relocated.
- Message cue payloads are sized by the `ZMSG_Sizes` dump and inline in the
  allocation; do not reallocate them separately.
- The scheduler is cooperative: long-running script code must yield via the
  float return protocol, and any priority above 15 is clamped.
- `ZScriptC::Command` is re-entrant through nested message handling; use the
  existing flags (`ZSC_FLAG_HANDLING_MESSAGE`, `lMessageAllocatorCount`)
  instead of adding new guards.
