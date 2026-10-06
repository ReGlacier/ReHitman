/*
 * Alllevels_Levelcontrol.c — C port of the shared level-control script
 * "Alllevels_Levelcontrol" (PC_Hideout / Hideout.dll).
 *
 *   RUN               0x100372C2  (_Alllevels_Levelcontrol_RUN)
 *   ENTER             0x100373BE  (_Alllevels_Levelcontrol_ENTER)
 *   Idle RUN          0x100373F4  (_Alllevels_Levelcontrol_Idle_RUN)
 *   Missioncompleted  0x10037452  (_Alllevels_Levelcontrol_Missioncompleted)
 *   Missionfailed     0x10037483  (_Alllevels_Levelcontrol_Missionfailed)
 *   Characterkilled   0x100374B8  (_Alllevels_Levelcontrol_Characterkilled)
 *   PROCESSMESSAGE    0x10037646  (_Alllevels_Levelcontrol_PROCESSMESSAGE)
 *   msg 0x0857 body   0x100374ED  (_Alllevels_Levelcontrol_Message_0857)
 *   Imports / StaticImports / Unpack* = shared empty stub (nullsub_1)
 *
 * Behaviour: the root RUN installs ENTER (which initializes the single
 * permanent-script-line handle at script-var +0x00 to -1 = "none") and then
 * switches the thread into the one "Idle" state, whose RUN sleeps forever
 * (yield -1.0 = 1 second, re-armed on every frame timeout). The three state
 * functions broadcast mission/kill notifications through the level-control
 * imports (import table slots 287/288/289 — Silevelcontrol__Missioncompleted/
 * Missionfailed/Characterkilled; see Docs/PC_Hideout_CONTEXT.md §10 for the
 * slot math). PROCESSMESSAGE only handles message 0x0857: for up to 8
 * (first, second) geometry pairs of the message payload it looks up the
 * "Alllevels_Human" script thread of the first geometry and, unless that
 * human already has enough lines, creates/updates the one permanent red
 * script line, then forwards 0x0859 (or 0x085B when mode == 2) commands to
 * the geometry. Levelcontrol is a root creator (no parent), so messages are
 * never forwarded up a creator chain.
 *
 * Creator/FC/STATECONTROLLER records live in AllLevels.c and the Levelcontrol
 * parent-data file (source/AllLevels_LevelcontrolParentData.c).
 */

#include <AllLevels/Alllevels_Levelcontrol.h>
#include <ScriptRuntime/ScriptSupport.h>

/* ================================================================== */
/* Script variables (m_lScriptVariablesSize == 0x04)                  */
/* ================================================================== */

typedef struct LevelcontrolScriptVars
{
    int32_t m_lScriptlineHandle; /* +0x00 — handle of the one permanent red
                                  * script line drawn by the 0x0857 handler;
                                  * ENTER sets -1 = "no line" (PC 0x100373E8) */
} LevelcontrolScriptVars; /* 0x04 == creator m_lScriptVariablesSize (PC 0x100433FC) */

/* Message 0x0857 payload (pMsgArg). Eight (first, second) geometry pairs
 * terminated by a null first entry, plus the mode dword at +0x40. */
typedef struct LevelcontrolMsg0857
{
    ZREF    m_rFirst[8];    /* +0x00 */
    ZREF    m_rSecond[8];   /* +0x20 */
    int32_t m_lMode;        /* +0x40 — 0: stop after the first accepted pair,
                             *          2: send 0x085B and stop,
                             *          otherwise: walk all pairs (PC loops i < 8) */
} LevelcontrolMsg0857;

/* ================================================================== */
/* Script functions                                                   */
/* ================================================================== */

/* root state / function-controller records — see parent-data file */

/* root RUN — _Alllevels_Levelcontrol_RUN (0x100372C2).
 * Standard root driver: install ENTER, wait, then prepare a switch-state
 * block pointing at this script's "Idle" state and switch into it. */
float Alllevels_Levelcontrol_RUN(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);

    switch (frame->m_lFunctionIndex)
    {
    case 0:
        pState->m_pFunctionsVirtualTable =
            pState->m_pCreator->m_pStateController->m_pFunctionsVirtualTable;
        /* fall through */
    case 1:
    {
        void* block = SF.Alloc(0x14, __FILE__, __LINE__);
        frame->m_pNextVariables = (LocalVarEntry*)block;
        if (!block)
        {
            frame->m_lFunctionIndex = 1;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
            return 0.0f;
        }
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        /* fall through */
    }
    case 2:
        AS_InstallCall(pState, frame, 3,
                       &Alllevels_Levelcontrol_EnterFUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    case 3:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
        /* fall through */
    case 4:
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(36, __FILE__, __LINE__);
        if (frame->m_pNextVariables)
        {
            pState->m_Flags &= (uint16_t)~(ZSC_FLAG_ASYNC_WAITING | ZSC_FLAG_SKIP_MESSAGE_QUEUE);
            frame->m_lFunctionIndex = 5;
        }
        else
        {
            frame->m_lFunctionIndex = 4;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
        }
        return 0.0f;
    case 5:
        ((SwitchStateStruct*)frame->m_pNextVariables)->stateController =
            &Alllevels_Levelcontrol_State_Idle;
        AS_InstallSwitchState(pState, frame, 6, &AS_SwitchState_FUNCTIONCONTROLLER);
        return 0.0f;
    case 6:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
        return SC_RET_TERMINATE;
    default:
        return SC_RET_TERMINATE;
    }
}

/* ENTER — _Alllevels_Levelcontrol_ENTER (0x100373BE).
 * Initializes the script-line handle to -1 ("none") and unwinds; no async
 * points. */
float Alllevels_Levelcontrol_ENTER(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    LevelcontrolScriptVars* sv = (LevelcontrolScriptVars*)pState->m_pScriptVariables;
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex != 0)
    {
        if (resumeIndex != 1)
            return AS_ExitCall(pState);
    }
    else
    {
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    sv->m_lScriptlineHandle = -1;
    return AS_ExitCall(pState);
}

/* Idle RUN — _Alllevels_Levelcontrol_Idle_RUN (0x100373F4).
 * Sleep loop: yields -1.0 (1-second sleep) and re-arms on every frame
 * timeout; on thread exit clears the frame function controller and yields 0. */
float Alllevels_Levelcontrol_Idle_RUN(ScriptState* pState)
{
    LocalVarEntry* lve = AS_GetFrame(pState);

    for (;;)
    {
        uint16_t resume = lve->m_lFunctionIndex & 0x7FFFu;
        if (resume == 0)
            break;
        if (resume == 1)
            break;
        if (resume == AS_EXIT_INDEX_NONE)
        {
            lve->m_pFunctionController = NULL;
            return 0.0f;
        }
        lve->m_lFunctionIndex = 1;
        if (SF.CheckTimeout())
            return 0.0f;
    }

    if ((lve->m_lFunctionIndex & 0x7FFFu) == 0)
        lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    lve->m_lFunctionIndex = 2;
    return -1.0f;
}

/* Missioncompleted — _Alllevels_Levelcontrol_Missioncompleted (0x10037452).
 * State function (FC input 0x14 — no parameters). */
float Alllevels_Levelcontrol_Missioncompleted(ScriptState* pState)
{
    LocalVarEntry* lve = AS_GetFrame(pState);
    uint16_t resumeIndex = lve->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex != 0)
    {
        if (resumeIndex != 1)
            return AS_ExitCall(pState);
    }
    else
    {
        lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    ScriptImports.Silevelcontrol__Missioncompleted();
    return AS_ExitCall(pState);
}

/* Missionfailed — _Alllevels_Levelcontrol_Missionfailed (0x10037483).
 * State function (FC input 0x18: the caller installs the reason string as
 * the frame's first local). */
float Alllevels_Levelcontrol_Missionfailed(ScriptState* pState)
{
    LocalVarEntry* lve = AS_GetFrame(pState);
    uint16_t resumeIndex = lve->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex != 0)
    {
        if (resumeIndex != 1)
            return AS_ExitCall(pState);
    }
    else
    {
        lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    ScriptImports.Silevelcontrol__Missionfailed(
        *(const char* const*)AS_FRAME_VARS(lve));
    return AS_ExitCall(pState);
}

/* Characterkilled — _Alllevels_Levelcontrol_Characterkilled (0x100374B8).
 * State function (FC input 0x1C: ZREF at the frame's first local).
 * NOTE: in PC the "Characterharmed" FC record (4th entry of the state's FC
 * table, 0x10043488) references this same entry — the compiled output of the
 * PC build genuinely shares one body for both mission notifications. */
float Alllevels_Levelcontrol_Characterkilled(ScriptState* pState)
{
    LocalVarEntry* lve = AS_GetFrame(pState);
    uint16_t resumeIndex = lve->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex != 0)
    {
        if (resumeIndex != 1)
            return AS_ExitCall(pState);
    }
    else
    {
        lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    ScriptImports.Silevelcontrol__Characterkilled(
        *(const ZREF*)AS_FRAME_VARS(lve));
    return AS_ExitCall(pState);
}

/* 0x0857 message body — PC _Alllevels_Levelcontrol_Message_0857 (0x100374ED).
 * Walks the payload's geometry pairs; for each pair whose first geometry has
 * an "alllevels.human" script thread that does not already carry enough
 * lines, (re)points the single permanent red script line at that pair and
 * forwards a 0x0859 command (0x085B in mode 2) to the geometry. If the
 * thread lookup succeeds but yields no state, the PC code puts the thread to
 * sleep through SF.Sleep(-2.0) and aborts — mirrored as-is. */
static void Alllevels_Levelcontrol_HandleMessage0857(ScriptState* pState,
                                                    const LevelcontrolMsg0857* msg)
{
    LevelcontrolScriptVars* sv = (LevelcontrolScriptVars*)pState->m_pScriptVariables;
    int i;

    for (i = 0; i < 8; ++i)
    {
        ZREF rFirst = msg->m_rFirst[i];
        ZREF rSecond = msg->m_rSecond[i];
        ScriptState* pAlien;

        if (!rFirst)
            break;

        pAlien = (ScriptState*)SF.GetAlienScriptState(
            SF.FindScriptStateByRef(rFirst, "alllevels.human"));
        if (!pAlien)
        {
            SF.Sleep(-2.0f);
            return;
        }

        /* TODO: Finish me after Alllevels_Human script variables reversed —
         * PC checks human script-vars byte +0x17D bit 0 (line already shown?)
         * and human script-vars int +0x68 (line count, must stay < 3). */
        {
            const uint8_t* pHumanVars = (const uint8_t*)pAlien->m_pScriptVariables;

            if ((pHumanVars[0x17D] & 0x01u) == 0 &&
                *(const int32_t*)(pHumanVars + 0x68) < 3)
            {
                if (sv->m_lScriptlineHandle == -1)
                    sv->m_lScriptlineHandle =
                        ScriptImports.Debugfunctions__Displaypermanentscriptline(
                            rFirst, rSecond, 0x00FF0000);   /* red, RGB */
                else
                    ScriptImports.Debugfunctions__Modifypermanentscriptline(
                        sv->m_lScriptlineHandle, rFirst, rSecond, 0x00FF0000);

                if (msg->m_lMode == 2)
                {
                    SF.SendCommand(rFirst, 0x085B, &rSecond, pState->m_rThis);
                    return;
                }

                SF.SendCommand(rFirst, 0x0859, &rSecond, pState->m_rThis);
                if (msg->m_lMode == 0)
                    return;
            }
        }
        /* PC also reloads the low byte of m_lMode (v13) here — dead store. */
    }
}

/* PROCESSMESSAGE — _Alllevels_Levelcontrol_PROCESSMESSAGE (0x10037646).
 * Only 0x0857 is handled; every other message id is swallowed — correct for
 * a root creator (PC m_pParentCreator = NULL). */
void Alllevels_Levelcontrol_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    if (msgId == 0x0857)
        Alllevels_Levelcontrol_HandleMessage0857(
            pState, (const LevelcontrolMsg0857*)pMsgArg);
}
