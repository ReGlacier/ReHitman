/*
 * Hideout_Levelcontrol.c — C port of the script "Hideout_Hideout_Levelcontrol"
 * from PC_Hideout (Hideout.dll). All records mirror the binary exactly.
 *
 *   creator      0x10044488   root SC   0x10044458   Idle SC  0x100444E0
 *   strings/imports blob 0x10044478   savegame statics 0x10044480
 *   RUN 0x1003CD72  IMPORTS 0x1003CE6E  Checkitems 0x1003CE98
 *   ENTER 0x1003CF28  Idle RUN 0x1003CFCF  PROCESSMESSAGE 0x1003D02D
 *
 * Behaviour: ENTER chains the parent Alllevels_Levelcontrol ENTER, then runs
 * "Checkitems" twice — once with the list at script-vars +0x08, once with the
 * list at +0x04 (Zlist__Getcount/Zlist__Getref/ZScene-level
 * Holevelcontrol__Hideoutcheckitem + Checkcustomweaponstate on every hit) —
 * then the root state switches to "Idle", which sleeps in a 1000 s loop.
 * PROCESSMESSAGE swallows 0x0B3D (Silevelcontrol__Missioncompleted).
 */

#include <Hideout/Hideout_Levelcontrol.h>
#include <AllLevels/Alllevels_Levelcontrol.h>
#include <ScriptRuntime/ScriptSupport.h>
#include <stddef.h>

/* ================================================================== */
/* Script / state variable layout                                     */
/* ================================================================== */

typedef struct LevelcontrolScriptVars
{
    uint8_t m_ParentReserved[0x04];  /* +0x00 Alllevels_Levelcontrol permanent
                                      * script-line handle (parent ENTER writes
                                      * -1 = no line; see Message_0857 logic) */
    ZREF    m_rCheckListB;           /* +0x04 — checked by ENTER resume case 3    */
    ZREF    m_rList;                 /* +0x08 — checked first by ENTER case 2     */
} LevelcontrolScriptVars; /* 0x0C == creator m_lScriptVariablesSize (PC 0x1004448C) */

/* Checkitems frame locals (frame vars begin at lve+0x14; the calling ENTER
 * allocates the 0x20-byte frame — PC writes at lve+0x14/0x18/0x1C):
 *   +0x14 list ref, +0x18 loop index, +0x1C current item. */
typedef struct LevelcontrolCheckitemsVars
{
    ZREF    m_rList;
    int32_t m_lIndex;
    ZREF    m_rItem;
} LevelcontrolCheckitemsVars;

/* ================================================================== */
/* Script-functions                                                   */
/* ================================================================== */

/* state / function-controller records defined below (see field maps) */
extern const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_RUN_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_ENTER_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_Checkitems_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_Idle_RUN_FUNCTIONCONTROLLER;
extern const STATECONTROLLER Hideout_Hideout_Levelcontrol_State_Idle;
extern const STATECONTROLLER Hideout_Hideout_Levelcontrol_ROOTSTATE;

/* Loop-yield helper emitted by the script compiler for while-loops
 * (PC sub_1003D14B; not exported by ScriptSupport.h). Faithful mirror. */
static int LC_WhileYield(ScriptState* pState, uint16_t resumeCase)
{
    LocalVarEntry* target;
    uint16_t flags;

    if (!SF.CheckTimeout() && !(pState->m_Flags & ZSC_FLAG_CLEAR_AFTER_ENTRY))
        return 0;

    flags = pState->m_Flags;
    if ((flags & 0x000Cu) != 0 || (flags & ZSC_FLAG_ASYNC_ACTIVE) == 0)
        target = pState->m_pVariables;
    else
        target = pState->m_pAsyncCall->m_pLVE;

    target->m_lFunctionIndex = resumeCase;
    return 1;
}

/* root RUN — _Hideout_Hideout_Levelcontrol_RUN (0x1003CD72). */
float Hideout_Hideout_Levelcontrol_RUN(ScriptState* pState)
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
        AS_InstallCall(pState, frame, 3,
                       &Hideout_Hideout_Levelcontrol_ENTER_FUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
    case 2:
        AS_InstallCall(pState, frame, 3,
                       &Hideout_Hideout_Levelcontrol_ENTER_FUNCTIONCONTROLLER);
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
            &Hideout_Hideout_Levelcontrol_State_Idle;
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

/* ENTER — _Hideout_Hideout_Levelcontrol_ENTER (0x1003CF28). Chains the parent
 * (Alllevels_Levelcontrol) ENTER, then calls Checkitems for each imported list
 * (list at script-vars +0x08 first, then +0x04). */
float Hideout_Hideout_Levelcontrol_ENTER(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    LevelcontrolScriptVars* sv = (LevelcontrolScriptVars*)pState->m_pScriptVariables;
    LevelcontrolCheckitemsVars* cv;
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex != 0)
    {
        if (resumeIndex != 1)
        {
            if (resumeIndex == 2)
            {
                cv = (LevelcontrolCheckitemsVars*)AS_FRAME_VARS(frame->m_pNextVariables);
                cv->m_rList = sv->m_rList;      /* PC: [next+20] = scriptvar +8 */
                AS_InstallCall(pState, frame, 3,
                               &Hideout_Hideout_Levelcontrol_Checkitems_FUNCTIONCONTROLLER);
                return SC_RET_CONTINUE_AFTER_TIMEOUT;
            }
            if (resumeIndex != 3)
            {
                SF.Free(frame->m_pNextVariables);
                frame->m_pNextVariables = NULL;
                return AS_ExitCall(pState);
            }
            cv = (LevelcontrolCheckitemsVars*)AS_FRAME_VARS(frame->m_pNextVariables);
            cv->m_rList = sv->m_rCheckListB;    /* PC: [next+20] = scriptvar +4 */
            AS_InstallCall(pState, frame, 4,
                           &Hideout_Hideout_Levelcontrol_Checkitems_FUNCTIONCONTROLLER);
            return SC_RET_CONTINUE_AFTER_TIMEOUT;
        }
    }
    else
    {
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(32, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 0;
            return 0.0f;
        }
        frame->m_lNextVariablesSize = 32;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }
    AS_InstallCall(pState, frame, 2, &Alllevels_Levelcontrol_EnterFUNCTIONCONTROLLER);
    return SC_RET_CONTINUE_AFTER_TIMEOUT;
}

/* Checkitems — _Hideout_Hideout_Levelcontrol_Checkitems (0x1003CE98).
 * Walks the list installed by ENTER; for every item that
 * Holevelcontrol__Hideoutcheckitem accepts, runs
 * Holevelcontrol__Checkcustomweaponstate on it. */
float Hideout_Hideout_Levelcontrol_Checkitems(ScriptState* pState)
{
    LocalVarEntry* lve = AS_GetFrame(pState);
    LevelcontrolCheckitemsVars* v = (LevelcontrolCheckitemsVars*)AS_FRAME_VARS(lve);
    int startAtIncrement = 0;

    switch (lve->m_lFunctionIndex & 0x7FFFu)
    {
    case 0:
        lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        /* fall through */
    case 1:
        v->m_lIndex = 0;
        break;
    case 2:
    case 4:
        break;                    /* resume straight into the loop test */
    case 3:
        startAtIncrement = 1;     /* PC "goto LABEL_12" — straight to ++index */
        break;
    default:
        return AS_ExitCall(pState);
    }

    for (;;)
    {
        if (!startAtIncrement)
        {
            int32_t i = v->m_lIndex;
            if (i >= (int32_t)ScriptImports.Zlist__Getcount(v->m_rList))
                break;
            v->m_rItem = ScriptImports.Zlist__Getref(v->m_rList, i);
            if ((unsigned char)ScriptImports.Holevelcontrol__Hideoutcheckitem(v->m_rItem))
                ScriptImports.Holevelcontrol__Checkcustomweaponstate(v->m_rItem);
        }
        startAtIncrement = 0;

        ++v->m_lIndex;
        if (LC_WhileYield(pState, 4))
            return 0.0f;
    }
    return AS_ExitCall(pState);
}

/* Idle RUN — _Hideout_Hideout_Levelcontrol_Idle_RUN (0x1003CFCF). Long sleep
 * loop; on thread exit clears the frame function controller and yields 0. */
float Hideout_Hideout_Levelcontrol_Idle_RUN(ScriptState* pState)
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
    return 1000.0f;
}

/* PROCESSMESSAGE — _Hideout_Hideout_Levelcontrol_PROCESSMESSAGE (0x1003D02D):
 * 0x0B3D finishes the mission, everything else walks up the creator chain. */
void Hideout_Hideout_Levelcontrol_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    if (msgId == 0x0B3D)
    {
        ScriptImports.Silevelcontrol__Missioncompleted();
        return;
    }
    AS_CallParentProcessMessage(pState, msgId, pMsgArg);
}

/* IMPORTS — _Hideout_Hideout_Levelcontrol_IMPORTS (0x1003CE6E): the compiled
 * form invokes the parent creator's Imports (Alllevels_Levelcontrol's is the
 * shared empty stub, PC nullsub_1 @0x1003C93C), then SF.Input on the two
 * list references at script-vars +0x08 and +0x04 (in that order). */
void Hideout_Hideout_Levelcontrol_IMPORTS(ScriptState* pState)
{
    /* Alllevels_Levelcontrol::Imports — PC: shared empty stub, no-op. */

    SF.Input((char*)pState->m_pScriptVariables +
             offsetof(LevelcontrolScriptVars, m_rList), 4);
    SF.Input((char*)pState->m_pScriptVariables +
             offsetof(LevelcontrolScriptVars, m_rCheckListB), 4);
}

/* ================================================================== */
/* Function controllers (input sizes / data exactly as in PC)         */
/* ================================================================== */

const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_RUN_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Levelcontrol_RUN,
    0x14, 0x00, NULL, NULL        /* PC 0x10044440 */
};

const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_ENTER_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Levelcontrol_ENTER,
    0x14, 0x00, NULL, NULL        /* PC 0x1004444C */
};

/* Checkitems FC — PC 0x100444C8: input 0x20 (0x14 header + the three frame
 * vars), data 0x08, no name; PC m_lStringOffsets = 0x1003CFCF (debug-string
 * overlap into the Idle RUN code) — not representable, left NULL. */
const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_Checkitems_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Levelcontrol_Checkitems,
    0x20, 0x08, NULL, NULL
};

/* Idle RUN FC — PC 0x100444D4. PC m_lStringOffsets self-references the FC
 * (0x100444D4); not represented here. */
const FUNCTIONCONTROLLER Hideout_Hideout_Levelcontrol_Idle_RUN_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Levelcontrol_Idle_RUN,
    0x14, 0x00, NULL, NULL
};

/* Idle ENTER — absent in PC (state m_pEnter = NULL, no stub record). */

/* ================================================================== */
/* State controllers (PC 0x10044458 / 0x100444E0)                     */
/* ================================================================== */

/* verbatim string-offset / imports blob (PC 0x10044478, shared by the root
 * state and the creator; its tail 0x0005,0,0 doubles as the SGST terminator
 * the savegame-statics pointer refers to, exactly as in PC) */
static uint16_t s_Levelcontrol_OffsetStrings[] =
{
    0x000D, 0x000D, 0x0006, 0x0000, 0x0005, 0x0000, 0x0000, 0x0000
};

const STATECONTROLLER Hideout_Hideout_Levelcontrol_ROOTSTATE =
{
    &Hideout_Hideout_Levelcontrol_RUN_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Levelcontrol_ENTER_FUNCTIONCONTROLLER,
    NULL,                                     /* m_pDestroy       */
    NULL,                                     /* ProcessMessage   */
    NULL,                                     /* m_pFunctionsVirtualTable (none in PC) */
    1,                                        /* m_lLevel         */
    2,                                        /* m_lScriptLevel   */
    &Alllevels_Levelcontrol_ROOTSTATE,        /* parent = Alllevels_Levelcontrol root */
    NULL,
    s_Levelcontrol_OffsetStrings
};

const STATECONTROLLER Hideout_Hideout_Levelcontrol_State_Idle =
{
    &Hideout_Hideout_Levelcontrol_Idle_RUN_FUNCTIONCONTROLLER,
    NULL,                                     /* m_pEnter (none in PC)    */
    NULL,                                     /* m_pDestroy               */
    NULL,                                     /* ProcessMessage           */
    NULL,                                     /* m_pFunctionsVirtualTable */
    2,                                        /* m_lLevel         */
    2,                                        /* m_lScriptLevel   */
    &Hideout_Hideout_Levelcontrol_ROOTSTATE,  /* parent state    */
    NULL,
    /* PC: pointer to the "Hideout_Hideout_Levelcontrol" name string
     * (0x10044500) — see the creator m_pName literal. */
    s_Levelcontrol_OffsetStrings              /* shared blob stand-in */
};

/* ================================================================== */
/* Statics (PC 0x10044480)                                            */
/* ================================================================== */

static const SAVEGAMESTATICS s_Levelcontrol_SaveGameStatics[] =
{
    { 5, 0, NULL }    /* SGST_END terminator (the only entry in PC) */
};

/* ================================================================== */
/* Creator record (PC 0x10044488)                                     */
/* ================================================================== */

const SCRIPTCREATOR Hideout_Levelcontrol =
{
    "Hideout_Hideout_Levelcontrol",           /* m_pName (PC 0x10044500) */
    0x0C,                                     /* m_lScriptVariablesSize */
    0x00,                                     /* m_lStateVariablesSize  */
    &Hideout_Hideout_Levelcontrol_ROOTSTATE,  /* PC 0x10044458 */
    &Alllevels_Levelcontrol,                  /* parent creator (PC 0x100433F8) */
    NULL,                                     /* m_pStatesVirtualTable (none in PC) */
    (ProcessMessage_t)Hideout_Hideout_Levelcontrol_PROCESSMESSAGE,
    AS_EmptyVoid,                             /* Initialize (PC nullsub_1) */
    &s_Levelcontrol_SaveGameStatics[0],
    (VoidFunction_t)Hideout_Hideout_Levelcontrol_IMPORTS,
    AS_EmptyVoid,                             /* StaticImports (PC nullsub_1) */
    AS_EmptyVoid,                             /* UnpackResources (PC nullsub_1) */
    AS_EmptyVoid,                             /* UnpackStaticResources (nullsub) */
    (const SCRIPTIMPORT*)&s_Levelcontrol_OffsetStrings[0]
};