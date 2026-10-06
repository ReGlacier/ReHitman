/*
 * Hideout_Canary.c — C port of the script "Hideout_Hideout_Canary" from
 * PC_Hideout (Hideout.dll). All records mirror the binary exactly.
 *
 *   creator      0x100440D4   root SC   0x10044090
 *   RUN 0x1003C4AE  ENTER 0x1003C752  Idle/Ambient RUN 0x1003C5C8
 *   PROCESSMESSAGE 0x1003C716  IMPORTS 0x1003C5AA
 *
 * Canary idle ambient behaviour: ENTER resolves the four movement animations
 * on the bird link; the root state switches into "Ambient", which copies the
 * feed-point position onto this geometry and plays one of the animations with
 * Alllevels_Baseboid::Playanimwait (random pick, 4 outcomes).
 */

#include <Hideout/Hideout_Canary.h>
#include <AllLevels/Alllevels_Bird.h>
#include <AllLevels/Alllevels_Baseboid.h>
#include <ScriptRuntime/ScriptSupport.h>
#include <stddef.h>

/* ================================================================== */
/* Script / state variable layout                                     */
/* ================================================================== */

typedef struct CanaryScriptVars
{
    uint8_t  m_ParentReserved[0x78];  /* +0x00 parent-chain fields (Bird/Baseboid, unreversed) */
    ZREF     m_rFeedPoint;            /* +0x78 imported feed-point reference */
    uint16_t m_hAnimEat;              /* +0x7C "/Movement/Eating"            */
    uint16_t m_hAnimStand;            /* +0x7E "/Movement/Stand_Relax"       */
    uint16_t m_hAnimWhistleLong;      /* +0x80 "/Movement/Whistle_Long"      */
    uint16_t m_hAnimWhistleShort;     /* +0x82 "/Movement/Whistle_Short"     */
} CanaryScriptVars; /* 0x84 == creator m_lScriptVariablesSize (PC 0x100440D8+4) */

/* Ambient-state frame locals (frame vars begin at lve+0x14 = +20):
 *   +20 fRandom, +24 feed ref, +28 pos (PC writes at lve+20/24/28..36). */
typedef struct CanaryAmbientStateVars
{
    float m_fRandom;
    ZREF  m_rFeedPoint;
    v3    m_Pos;
} CanaryAmbientStateVars;

/* ================================================================== */
/* Script-functions                                                   */
/* ================================================================== */

/* state / function-controller records defined below (see field maps) */
extern const FUNCTIONCONTROLLER Hideout_Hideout_Canary_RUN_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Canary_ENTER_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Canary_Ambient_RUN_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Canary_Ambient_ENTER_FUNCTIONCONTROLLER;
extern const STATECONTROLLER Hideout_Hideout_Canary_State_Ambient;
extern const STATECONTROLLER Hideout_Hideout_Canary_ROOTSTATE;
extern const void* const Hideout_Hideout_Canary_FunctionsVT[];
extern const void* const Hideout_Hideout_Canary_StatesVT[];

/* root RUN — _Hideout_Hideout_Canary_RUN (0x1003C4AE). */
float Hideout_Hideout_Canary_RUN(ScriptState* pState)
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
        AS_InstallCall(pState, frame, 3, &Hideout_Hideout_Canary_ENTER_FUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
    case 2:
        AS_InstallCall(pState, frame, 3, &Hideout_Hideout_Canary_ENTER_FUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    case 3:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
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
            &Hideout_Hideout_Canary_State_Ambient;
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

/* ENTER — _Hideout_Hideout_Canary_ENTER (0x1003C752). Chains the parent
 * (Alllevels_Bird) ENTER, then resolves the four movement animations. */
float Hideout_Hideout_Canary_ENTER(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    CanaryScriptVars* sv = (CanaryScriptVars*)pState->m_pScriptVariables;
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex == 0)
    {
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0x14, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 0;
            return 0.0f;
        }
        frame->m_lNextVariablesSize = 0x14;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }
    if (resumeIndex <= 1)
    {
        AS_InstallCall(pState, frame, 2, &Alllevels_Bird_EnterFUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
    if (resumeIndex == 2)
    {
        sv->m_hAnimEat          = (uint16_t)ScriptImports.Zlink__Getanim(pState->m_rThis, "/Movement/Eating");
        sv->m_hAnimStand        = (uint16_t)ScriptImports.Zlink__Getanim(pState->m_rThis, "/Movement/Stand_Relax");
        sv->m_hAnimWhistleLong  = (uint16_t)ScriptImports.Zlink__Getanim(pState->m_rThis, "/Movement/Whistle_Long");
        sv->m_hAnimWhistleShort = (uint16_t)ScriptImports.Zlink__Getanim(pState->m_rThis, "/Movement/Whistle_Short");
    }
    SF.Free(frame->m_pNextVariables);
    frame->m_pNextVariables = NULL;
    return AS_ExitCall(pState);
}

/* Ambient RUN — _Hideout_Hideout_Canary_Ambient_RUN (0x1003C5C8). Copies
 * the feed-point position onto this geometry and plays one of the animations
 * via Alllevels_Baseboid::Playanimwait. */
float Hideout_Hideout_Canary_Ambient_RUN(ScriptState* pState)
{
    CanaryScriptVars* sv = (CanaryScriptVars*)pState->m_pScriptVariables;
    LocalVarEntry* lve = AS_GetFrame(pState);
    CanaryAmbientStateVars* v;

    for (;;)
    {
        uint16_t resume = lve->m_lFunctionIndex & 0x7FFFu;
        if (resume == 0)
            break;
        if (resume == 1)
            goto body;
        if (resume == AS_EXIT_INDEX_NONE)
        {
            SF.Free(lve->m_pNextVariables);
            lve->m_pNextVariables = NULL;
            lve->m_pFunctionController = NULL;
            return 0.0f;
        }
        lve->m_lFunctionIndex = 1;
        if (SF.CheckTimeout())
            return 0.0f;
    }

    lve->m_pNextVariables = (LocalVarEntry*)SF.Alloc(24, __FILE__, __LINE__);
    if (!lve->m_pNextVariables)
        return 0.0f;
    lve->m_lNextVariablesSize = 24;
    lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;

body:
    v = (CanaryAmbientStateVars*)AS_FRAME_VARS(lve);
    v->m_rFeedPoint = sv->m_rFeedPoint;

    v->m_Pos = ScriptImports.Zgeom__Getposition(v->m_rFeedPoint);
    ScriptImports.Zgeom__Setposition(pState->m_rThis, v->m_Pos.x, v->m_Pos.y, v->m_Pos.z);

    v->m_fRandom = ScriptImports.Engine__Random() * 4.0f;
    {
        LocalVarEntry* cont = lve->m_pNextVariables;
        uint16_t anim = 0;
        uint16_t resumeCase;

        /* Faithful transcription of the PC branch tree (a NaN path picks the
         * eating animation): */
        if (!(v->m_fRandom >= 1.0f))
        {
            if (v->m_fRandom >= 1.0f) /* unreachable — kept as in the binary */
            {
                anim = sv->m_hAnimStand;          /* PC word[63] */
                resumeCase = 3;
            }
            else
            {
                anim = sv->m_hAnimEat;            /* PC word[62] */
                resumeCase = 2;
            }
        }
        else if (v->m_fRandom >= 3.0f)
        {
            anim = sv->m_hAnimWhistleShort;       /* PC word[65] */
            resumeCase = 5;
        }
        else
        {
            anim = sv->m_hAnimWhistleLong;        /* PC word[64] */
            resumeCase = 4;
        }

        *(uint16_t*)AS_FRAME_VARS(cont) = anim;
        AS_InstallCall(pState, lve, resumeCase,
                       &Alllevels_Baseboid_PlayanimwaitFUNCTIONCONTROLLER);
    }
    return SC_RET_CONTINUE_AFTER_TIMEOUT;
}

/* PROCESSMESSAGE — _Hideout_Hideout_Canary_PROCESSMESSAGE (0x1003C716):
 * swallow 0x81B, everything else walks up the creator chain. */
void Hideout_Hideout_Canary_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    if (msgId != 0x81B)
        AS_CallParentProcessMessage(pState, msgId, pMsgArg);
}

/* IMPORTS — _Hideout_Hideout_Canary_IMPORTS (0x1003C5AA): the compiled form
 * calls the parent creator's Imports first, then SF.Input on this script's
 * variables (+120 = the feed point). PC passes the state in a1. */
void Hideout_Hideout_Canary_IMPORTS(ScriptState* pState)
{
    /* Parent chain first — Alllevels_Bird::IMPORTS (PC chain @0x1000A603)
     * is not ported yet; expected once Alllevels Bird code lands: */
    /* Alllevels_Bird_IMPORTS(pState); */

    SF.Input((char*)pState->m_pScriptVariables +
             offsetof(CanaryScriptVars, m_rFeedPoint), 4);
}

/* ================================================================== */
/* Function controllers (input sizes / data exactly as in PC)         */
/* ================================================================== */

const FUNCTIONCONTROLLER Hideout_Hideout_Canary_RUN_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Canary_RUN,
    0x14, 0x00, NULL, NULL
};

const FUNCTIONCONTROLLER Hideout_Hideout_Canary_ENTER_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Canary_ENTER,
    0x14, 0x00, NULL, NULL
};

const FUNCTIONCONTROLLER Hideout_Hideout_Canary_Ambient_RUN_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Canary_Ambient_RUN,
    0x28, 0x14, NULL, NULL    /* PC 0x10044118: input 40, data 20 */
};

/* Ambient ENTER — PC entry is the shared empty stub (loc_1000721C). */
const FUNCTIONCONTROLLER Hideout_Hideout_Canary_Ambient_ENTER_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)AS_EmptyEntryPoint,
    0x14, 0x00, NULL, NULL
};

/* ================================================================== */
/* State controllers (PC 0x10044090 / 0x10044130)                     */
/* ================================================================== */

/* verbatim string-offset / imports table shared by the records (PC 0x100440B0) */
static uint16_t s_Canary_OffsetStrings[] =
{
    0x000D, 0x000D, 0x000D, 0x0008, 0x000B, 0x000B,
    0x000B, 0x0008, 0x000A, 0x000A, 0x000A, 0x000D,
    0x0006, 0x000D, 0x0005, 0x0000, 0x0000
};

const STATECONTROLLER Hideout_Hideout_Canary_State_Ambient =
{
    &Hideout_Hideout_Canary_Ambient_RUN_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Canary_Ambient_ENTER_FUNCTIONCONTROLLER,
    NULL,                                     /* m_pDestroy       */
    NULL,                                     /* ProcessMessage   */
    (const void*)&Hideout_Hideout_Canary_FunctionsVT[0],
    2,                                        /* m_lLevel         */
    3,                                        /* m_lScriptLevel   */
    &Hideout_Hideout_Canary_ROOTSTATE,        /* parent state    */
    NULL,
    s_Canary_OffsetStrings
};

const STATECONTROLLER Hideout_Hideout_Canary_ROOTSTATE =
{
    &Hideout_Hideout_Canary_RUN_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Canary_ENTER_FUNCTIONCONTROLLER,
    NULL,                                     /* m_pDestroy       */
    NULL,                                     /* ProcessMessage   */
    (const void*)&Hideout_Hideout_Canary_FunctionsVT[0],
    1,                                        /* m_lLevel         */
    3,                                        /* m_lScriptLevel   */
    &Alllevels_Bird_ROOTSTATE,                /* parent = Alllevels_Bird root */
    NULL,
    s_Canary_OffsetStrings
};

/* ================================================================== */
/* Statics / imports (PC 0x100440CC / 0x100440B0)                     */
/* ================================================================== */

static const SAVEGAMESTATICS s_Canary_SaveGameStatics[] =
{
    { 5, 0, NULL }    /* SGST_END terminator (the only entry in PC) */
};

/* ================================================================== */
/* Virtual tables (PC 0x10044054 / 0x10044118)                        */
/* ================================================================== */

const void* const Hideout_Hideout_Canary_FunctionsVT[] =
{
    &Hideout_Hideout_Canary_RUN_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Canary_ENTER_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Canary_Ambient_RUN_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Canary_Ambient_ENTER_FUNCTIONCONTROLLER,
    NULL
};

const void* const Hideout_Hideout_Canary_StatesVT[] =
{
    &Hideout_Hideout_Canary_ROOTSTATE,
    NULL
};

/* ================================================================== */
/* Creator record (PC 0x100440D4)                                     */
/* ================================================================== */

const SCRIPTCREATOR Hideout_Canary =
{
    "Hideout_Hideout_Canary",                 /* PC 0x100440D4 */
    0x84,                                     /* m_lScriptVariablesSize */
    0x1C,                                     /* m_lStateVariablesSize  */
    &Hideout_Hideout_Canary_ROOTSTATE,
    &Alllevels_Bird,                          /* parent creator */
    (const void*)&Hideout_Hideout_Canary_StatesVT[0],
    (ProcessMessage_t)Hideout_Hideout_Canary_PROCESSMESSAGE,
    AS_EmptyVoid,                            /* Initialize (PC: nullsub stub)  */
    &s_Canary_SaveGameStatics[0],
    (VoidFunction_t)Hideout_Hideout_Canary_IMPORTS,
    AS_EmptyVoid,                            /* StaticImports (PC: nullsub)      */
    AS_EmptyVoid,                            /* UnpackResources (PC: nullsub)    */
    AS_EmptyVoid,                            /* UnpackStaticResources (nullsub) */
    (const SCRIPTIMPORT*)&s_Canary_OffsetStrings[0]
};
