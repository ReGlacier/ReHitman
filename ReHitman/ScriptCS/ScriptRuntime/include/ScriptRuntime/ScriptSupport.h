#ifndef __HBM_SCRIPT_RUNTIME_SCRIPT_SUPPORT_H__
#define __HBM_SCRIPT_RUNTIME_SCRIPT_SUPPORT_H__

/*
 * ScriptSupport — C mirror of the per-DLL "support section" that the original
 * script compiler emits into every ScriptCS module (PC_Hideout: 0x1003D000..
 * 0x1003D9xx, plus its shared FC records). Ported 1:1 from the PC_Hideout
 * decompiles; each function below names its now-named PC_Hideout twin:
 *
 *   AS_GetFrame                <- AS_GetActiveFrame (sub_1003D27C)
 *   AS_InstallCall             <- AS_InstallCall (sub_1003D299)
 *   AS_ExitCall                <- AS_ExitCall (sub_1003D216)
 *   AS_InstallSwitchState      <- InstallSwitchState (0x1003D8C5)
 *   AS_InstallForcedAsyncCall  <- InstallForcedAsyncCall (0x1003D460)
 *   AS_UnlinkAsyncCall         <- AS_UnlinkAsyncCall (sub_1003D188)
 *   AS_CountStateTransitions   <- AS_CountStateTransitions (sub_1003D06F)
 *   AS_ResizeFrameVars         <- AS_ResizeFrameVars (sub_1003D0E2)
 *   AS_SwitchState             <- AS_SwitchState (sub_1003D581)
 *   AS_SwitchStateDriver       <- AS_SwitchStateDriver (sub_1003D64C)
 *   AS_CallParentProcessMessage<- the creator-parent walk every
 *                                _PROCESSMESSAGE ends with (e.g. 0x1003C716)
 *   AS_EmptyVoid / AS_EmptyEntryPoint <- shared stubs (nullsub_1 @0x1003C93C,
 *                                        loc_1000721C)
 *
 * The compiled-script ABI details (float return protocol, frame layout,
 * switch state records) are documented in Docs/PC_Hideout_CONTEXT.md section
 * "Compiled script runtime (support section)".
 */

#include <ScriptRuntime/ScriptRuntime.h>

/* Shared runtime blocks defined by each script module's source/Scripts.c
 * (DLL ordinals 1 and 4); the engine fills them at AttachSceneScripts. */
extern SCRIPTFUNCTIONS SF;
extern SCRIPTIMPORTSTABLE ScriptImports;

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================ */
/* Runtime thread-image mirrors (frozen engine structures, C)   */
/* ============================================================ */

/* _ScriptState — Glacier/ScriptEngine/ScriptState.h, 0x40 on x86. */
#ifndef SCRIPT_STATE_DEFINED
#define SCRIPT_STATE_DEFINED
typedef struct ScriptState
{
    void*                      m_pScriptVariables;        /* +0x00 */
    const SCRIPTCREATOR*       m_pCreator;                /* +0x04 */
    ZREF                       m_rThis;                   /* +0x08 */
    struct LocalVarEntry*      m_pVariables;              /* +0x0C */
    void*                      m_pStateVariables;         /* +0x10 */
    const STATECONTROLLER*     m_pStateController;        /* +0x14 */
    const STATECONTROLLER*     m_pPreviousStateController;/* +0x18 */
    const STATECONTROLLER*     m_pNextStateController;    /* +0x1C */
    uint16_t                   m_msgWaitForEvent;         /* +0x20 */
    uint16_t                   m_Flags;                   /* +0x22 ZSC_FLAG_* */
    struct AsyncCall_Struct*   m_pAsyncCall;              /* +0x24 */
    struct AsyncCall_Struct*   m_pAsyncCallLast;          /* +0x28 */
    const SCRIPTCREATOR*       m_pMessageHandler;         /* +0x2C */
    struct ScriptState*        m_pAlienCall;              /* +0x30 */
    const void*                m_pFunctionsVirtualTable;  /* +0x34 */
    void*                      m_pThreadInfo;             /* +0x38 */
    void*                      m_pMessageCue;             /* +0x3C */
} ScriptState;
#endif

/* _LocalVarEntry — Glacier/ScriptEngine/LocalVarEntry.h, 0x14. Frame-local
 * variables live immediately after the header ((char*)lve + 0x14). */
typedef struct LocalVarEntry
{
    const FUNCTIONCONTROLLER*  m_pFunctionController;     /* +0x00 */
    struct LocalVarEntry*      m_pNextVariables;          /* +0x04 deeper frame */
    struct LocalVarEntry*      m_pPrevVariables;          /* +0x08 calling frame */
    uint16_t                   m_lFunctionIndex;          /* +0x0C resume case (0x8000 = exiting) */
    uint16_t                   m_lExitFunctionIndex;      /* +0x0E 0x7FFF=none, 0xFFFF=switch driver */
    uint16_t                   m_lNextVariablesSize;      /* +0x10 */
    uint16_t                   m_lAlignment;              /* +0x12 */
} LocalVarEntry;

/* _AsyncCall_Struct — Glacier/ScriptEngine/AsyncCall_Struct.h, 0xC. */
typedef struct AsyncCall_Struct
{
    struct AsyncCall_Struct*   pNext;                     /* +0x00 */
    float                      m_fStoredNextRun;          /* +0x04 */
    LocalVarEntry*             m_pLVE;                    /* +0x08 */
} AsyncCall_Struct;

/* _SwitchStateStruct — Glacier/ScriptEngine/SwitchStateStruct.h, 0x24. */
typedef struct SwitchStateStruct
{
    LocalVarEntry              m_LVE;                     /* +0x00 */
    const STATECONTROLLER**    m_pEnters;                 /* +0x14 */
    uint16_t                   m_lNumEnters;              /* +0x18 */
    uint16_t                   m_lNumExits;               /* +0x1A */
    const STATECONTROLLER*     stateController;           /* +0x1C */
    const STATECONTROLLER*     pSS_pOldStateController;   /* +0x20 */
} SwitchStateStruct;

/* Frame locals / script-variable access. */
#define AS_FRAME_VARS(lve)        ((void*)((char*)(lve) + sizeof(LocalVarEntry)))
#define AS_SCRIPT_VAR(type, p, o) (*(type*)((char*)(p) + (o)))

/* State/creator-level ProcessMessage ABI: (state, msgId, arg) — the frozen
 * ProcessMessage_t field type keeps its 2-arg form; compiled scripts always
 * emit 3 args, so the support + script layer uses this local type. */
typedef void (*StateProcessMessage_t)(ScriptState* pState, uint16_t msgId, void* pMsgArg);

/* Shared empty entry/stub bodies (PC: nullsub_1 @0x1003C93C,
 * and the frame-ignoring stub at loc_1000721C). */
void  AS_EmptyVoid(void);
float AS_EmptyEntryPoint(ScriptState* pState);

/* ============================================================ */
/* Compiled-script ABI constants                                */
/* ============================================================ */

/* m_Flags bits — mirror Glacier/ScriptEngine/ScriptFlags.h names. */
#define ZSC_FLAG_ASYNC_ACTIVE             0x0002u
#define ZSC_FLAG_ASYNC_WAITING            0x0004u
#define ZSC_FLAG_HANDLING_MESSAGE         0x0008u
#define ZSC_FLAG_ALIEN_CALL_ACTIVE        0x0010u
#define ZSC_FLAG_SKIP_MESSAGE_QUEUE       0x0020u
#define ZSC_FLAG_CLEAR_AFTER_ENTRY        0x0040u
#define ZSC_CONTINUE_AFTER_SLEEP_MASK     0x0600u

/* Float return protocol for every compiled entry point (see the ScriptEngine
 * README "Scheduling" table):
 *   >= 0        sleep seconds (0 = retry / rerun next pass)
 *   -7          rerun with fNextRun = 0
 *   -3          continue after the installed/created frame (async chain)
 *   -2          terminate (root RUN done / fork cleanup) */
#define SC_RET_RETRY_NEXT_PASS            (-7.0f)
#define SC_RET_CONTINUE_AFTER_TIMEOUT     (-3.0f)
#define SC_RET_TERMINATE                  (-2.0f)

/* Resume-index sentinel pair used throughout the compiled output. */
#define AS_EXIT_INDEX_NONE                0x7FFFu

/* ============================================================ */
/* Support-section runtime (ported 1:1 from PC_Hideout)         */
/* ============================================================ */

LocalVarEntry* AS_GetFrame(ScriptState* pState);
void  AS_InstallCall(ScriptState* pState, LocalVarEntry* pFrame,
                     uint16_t nextCase, const FUNCTIONCONTROLLER* pCalleeFC);
float AS_ExitCall(ScriptState* pState);
uint16_t AS_InstallSwitchState(ScriptState* pState, LocalVarEntry* pFrame,
                               uint16_t nextCase, const FUNCTIONCONTROLLER* pSwitchFC);
void  AS_InstallForcedAsyncCall(ScriptState* pState, LocalVarEntry* pFrame,
                                uint16_t nextCase, const FUNCTIONCONTROLLER* pFC);
void  AS_UnlinkAsyncCall(ScriptState* pState, AsyncCall_Struct* pNode);
void  AS_CountStateTransitions(const STATECONTROLLER* pFrom, const STATECONTROLLER* pTo,
                               uint16_t* pNumEnters, uint16_t* pNumExits);
int   AS_ResizeFrameVars(ScriptState* pState, uint16_t* pFrameVarsSize, uint16_t newSize);
float AS_SwitchState(ScriptState* pState);
float AS_SwitchStateDriver(ScriptState* pState);
void  AS_CallParentProcessMessage(ScriptState* pState, uint16_t msgId, void* pMsgArg);

/* Shared DoSwitchState FC records (PC: 0x10044520 / 0x1004452C). */
extern const FUNCTIONCONTROLLER AS_SwitchState_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER AS_SwitchStateDriver_FUNCTIONCONTROLLER;

#ifdef __cplusplus
}
#endif

#endif /* __HBM_SCRIPT_RUNTIME_SCRIPT_SUPPORT_H__ */
