/*
 * Hideout_Happyrat.c — C port of the script "Hideout_Hideout_Happyrat" from
 * PC_Hideout (Hideout.dll). All records mirror the binary exactly.
 *
 *   creator      0x10044380   root SC   0x1004432C
 *   RUN 0x1003C829  ENTER 0x1003CCEB  PROCESSMESSAGE 0x1003CCA9
 *   IMPORTS 0x1003C93D  Initialize (statics) 0x1003C925
 *   Dodeadstuff 0x1003CADC (state function, entered through messages 0x921/0x92E)
 *   unnamed level-2 state 0x100443CC, run FC 0x10043C0 -> sub_1003C95E (NOT ported)
 *
 *   FC  RUN 0x10044314   ENTER 0x10044320   Dodeadstuff 0x100443EC
 *   FC  unnamed-state run 0x100443C0   states VT 0x100441C8   functions VT 0x100441E8
 *   string offsets / imports table 0x1004434C   savegame statics 0x1004435C
 *
 * Happy-rat behaviour: ENTER chains the parent Alllevels_Rat ENTER and clears
 * the local "sent" flags. Message 0xB3E marks this rat as "sent to the happy
 * place" and switches into the (Rat-declared) Happyfunness state; messages
 * 0x921 / 0x92E run Dodeadstuff synchronously, which switches into the Rat's
 * happy state and reports (once per level) that all four rats were sent.
 *
 * The two states installed by this script ("Happyfunness" 0x10043C14 and
 * 0x10043B2C) are declared by the parent Alllevels_Rat class (scriptlevel 3,
 * state parent = Alllevels_Rat root state) and therefore live in
 * AllLevelsRatParentData.c; the FC whose entry point is this script's
 * Happyfunness_RUN is also defined there so the record keeps its exact shape.
 */

#include <Hideout/Hideout_Happyrat.h>
#include <AllLevels/Alllevels_Rat.h>
#include <ScriptRuntime/ScriptSupport.h>
#include <stddef.h>

/* ================================================================== */
/* Script / state variable layout                                     */
/* ================================================================== */

typedef struct HappyratScriptVars
{
    uint8_t  m_ParentReserved[0x190];  /* +0x00 parent-chain fields (Alllevels_Rat) */
    uint8_t  m_fSentFlags;             /* +0x190 bit0 = "happy state entered",
                                        *         bit1 = "already counted"          */
    uint8_t  m_Padding[3];             /* +0x191                                          */
    void*    m_pvSwitchState;          /* +0x194 imported state-controller parameter
                                        *         (used by the unported state 0x100443CC)  */
} HappyratScriptVars; /* 0x198 == creator m_lScriptVariablesSize (PC 0x10044380+4) */

/* The only frame variables used by the ported functions are the shared
 * SwitchStateStruct allocated by the root RUN (36 bytes) and by Dodeadstuff
 * (36 bytes). Happyfunness_RUN only allocates the bare frame header (32 bytes)
 * that the inherited FC at PC 0x10043BFC consumes as its input frame. */

/* ================================================================== */
/* Save-game statics (PC 0x10046630 / 0x10046634 / 0x10046638)        */
/* ================================================================== */

static uint8_t  s_bHaveTimerBase;   /* byte_10046630  "timer base initialised" */
static float    s_fTimerBase;       /* flt_10046634   Engine__Gettime anchor    */
static int32_t  s_lSentRatCount;    /* dword_10046638 rats sent counter        */

/* ================================================================== */
/* Script-functions                                                   */
/* ================================================================== */

/* state / function-controller records defined below */
extern const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_RUN_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_ENTER_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_Dodeadstuff_FUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_UnnamedState_100443CC_RunFUNCTIONCONTROLLER;
extern const STATECONTROLLER Hideout_Hideout_Happyrat_ROOTSTATE;
extern const STATECONTROLLER Hideout_Hideout_Happyrat_UnnamedState_100443CC;
extern const void* const Hideout_Hideout_Happyrat_FunctionsVT[];
extern const void* const Hideout_Hideout_Happyrat_StatesVT[];

/* PC sub_1003D14B — the compiled "the wait was aborted" epilogue: when the
 * thread timed out or ZSC_FLAG_CLEAR_AFTER_ENTRY is set, rewinds the active
 * frame to the given resume index. Non-zero = caller must yield (return 0). */
static int AS_ResumeOnAbort(ScriptState* pState, uint16_t resumeIndex)
{
    uint16_t flags = pState->m_Flags;
    LocalVarEntry* target;

    if (!SF.CheckTimeout() && (flags & ZSC_FLAG_CLEAR_AFTER_ENTRY) == 0)
        return 0;

    if ((flags & (ZSC_FLAG_ASYNC_WAITING | ZSC_FLAG_HANDLING_MESSAGE)) != 0 ||
        (flags & ZSC_FLAG_ASYNC_ACTIVE) == 0)
    {
        target = pState->m_pVariables;
    }
    else
    {
        target = pState->m_pAsyncCall->m_pLVE;
    }
    target->m_lFunctionIndex = resumeIndex;
    return 1;
}

/* root RUN — _Hideout_Hideout_Happyrat_RUN (0x1003C829). */
float Hideout_Hideout_Happyrat_RUN(ScriptState* pState)
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
        AS_InstallCall(pState, frame, 3, &Hideout_Hideout_Happyrat_ENTER_FUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
    case 2:
        AS_InstallCall(pState, frame, 3, &Hideout_Hideout_Happyrat_ENTER_FUNCTIONCONTROLLER);
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
            &Alllevels_Rat_Happyfunness_STATECONTROLLER;
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

/* ENTER — _Hideout_Hideout_Happyrat_ENTER (0x1003CCEB). Chains the parent
 * (Alllevels_Rat) ENTER, then clears the local "sent" flags. */
float Hideout_Hideout_Happyrat_ENTER(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    HappyratScriptVars* sv = (HappyratScriptVars*)pState->m_pScriptVariables;
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
        AS_InstallCall(pState, frame, 2, &Alllevels_Rat_EnterFUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
    if (resumeIndex == 2)
    {
        sv->m_fSentFlags &= (uint8_t)~0x3u;
    }
    SF.Free(frame->m_pNextVariables);
    frame->m_pNextVariables = NULL;
    return AS_ExitCall(pState);
}

/* Happyfunness state RUN — _Hideout_Hideout_Happyrat_Happyfunness_RUN
 * (0x1003B008). Overrides the parent-class state; picks one of two inherited
 * Rat methods (70 % / 30 %). */
float Hideout_Hideout_Happyrat_Happyfunness_RUN(ScriptState* pState)
{
    LocalVarEntry* lve = AS_GetFrame(pState);

    for (;;)
    {
        uint16_t resume = lve->m_lFunctionIndex & 0x7FFFu;

        if (resume > 4)
        {
            if (resume == 5 || resume == 6)
                goto body;
            if (resume == AS_EXIT_INDEX_NONE)
            {
                SF.Free(lve->m_pNextVariables);
                lve->m_pNextVariables = NULL;
                pState->m_pVariables->m_pFunctionController = NULL;
                return 0.0f;
            }
            lve->m_lFunctionIndex = 1;
            if (SF.CheckTimeout())
                return 0.0f;
            continue;
        }

        if (resume == 4)
            goto install;

        if (resume != 0)
        {
            if (resume <= 3)
                goto body;
            lve->m_lFunctionIndex = 1;
            if (SF.CheckTimeout())
                return 0.0f;
            continue;
        }
        else
        {
            lve->m_pNextVariables = (LocalVarEntry*)SF.Alloc(32, __FILE__, __LINE__);
            if (!lve->m_pNextVariables)
            {
                lve->m_lFunctionIndex = 0;
                return 0.0f;
            }
            lve->m_lNextVariablesSize = 32;
            lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        }

body:
        if (AS_ResumeOnAbort(pState, 4))
            return 0.0f;

install:
        if (ScriptImports.Engine__Random() >= 0.3f)
            AS_InstallCall(pState, lve, 6, &Alllevels_Rat_UnreversedFUNCTIONCONTROLLER_100439AC);
        else
            AS_InstallCall(pState, lve, 5, &Alllevels_Rat_UnreversedFUNCTIONCONTROLLER_10043BFC);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
}

/* Dodeadstuff — _Hideout_Hideout_Happyrat_Dodeadstuff (0x1003CADC). Counts the
 * rats that were sent away (the level shows the "happy place" subtitle once
 * four of them are gone) and switches into the Rat happy state. */
float Hideout_Hideout_Happyrat_Dodeadstuff(ScriptState* pState)
{
    LocalVarEntry* lve = AS_GetFrame(pState);
    HappyratScriptVars* sv = (HappyratScriptVars*)pState->m_pScriptVariables;
    uint16_t resumeIndex = lve->m_lFunctionIndex & 0x7FFFu;
    uint16_t resumeCase;
    uint8_t flags;

    if (resumeIndex != 0)
    {
        if (resumeIndex != 1)
        {
            SF.Free(lve->m_pNextVariables);
            lve->m_pNextVariables = NULL;
            return AS_ExitCall(pState);
        }
    }
    else
    {
        lve->m_pNextVariables = (LocalVarEntry*)SF.Alloc(36, __FILE__, __LINE__);
        if (!lve->m_pNextVariables)
        {
            lve->m_lFunctionIndex = 0;
            return 0.0f;
        }
        lve->m_lNextVariablesSize = 36;
        lve->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    flags = sv->m_fSentFlags;
    if ((flags & 1u) != 0)
    {
        if ((flags & 2u) != 0)
        {
            SF.Free(lve->m_pNextVariables);
            lve->m_pNextVariables = NULL;
            return AS_ExitCall(pState);
        }
        sv->m_fSentFlags = (uint8_t)(flags | 2u);
        s_lSentRatCount = s_lSentRatCount + 1;
        if (s_lSentRatCount >= 4)
        {
            float fElapsed = ScriptImports.Engine__Gettime() - s_fTimerBase;
            ScriptImports.Holevelcontrol__Showhappysubtitle(
                "You sent all the\nrats to a happy\nplace in", (int)fElapsed);
        }
        if (!s_bHaveTimerBase)
        {
            s_bHaveTimerBase = 1;
            s_fTimerBase = ScriptImports.Engine__Gettime();
        }
        resumeCase = 3;
    }
    else
    {
        resumeCase = 2;
    }

    ((SwitchStateStruct*)lve->m_pNextVariables)->stateController =
        &Alllevels_Rat_HappyStateCONTROLLER_10043B2C;
    AS_InstallSwitchState(pState, lve, resumeCase, &AS_SwitchState_FUNCTIONCONTROLLER);
    return SC_RET_CONTINUE_AFTER_TIMEOUT;
}

/* PROCESSMESSAGE — _Hideout_Hideout_Happyrat_PROCESSMESSAGE (0x1003CCA9).
 * 0x921 / 0x92E run Dodeadstuff inline (fresh frame + SF.RunNoBreak inside
 * AS_InstallCall), 0xB3E marks the rat and switches into the Rat Happyfunness
 * state. Anything else is swallowed once this rat was "sent"; otherwise it
 * walks up the creator chain (PC sub_1003CC6F == AS_CallParentProcessMessage).
 * PC breaks on allocation failure (__debugbreak()); the flow continues. */
void Hideout_Hideout_Happyrat_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    HappyratScriptVars* sv = (HappyratScriptVars*)pState->m_pScriptVariables;

    switch (msgId)
    {
    case 0x921:
    {
        LocalVarEntry* frame = (LocalVarEntry*)SF.Alloc(0x14, __FILE__, __LINE__);
        if (!frame) {} /* PC: __debugbreak(); execution continues */
        AS_InstallCall(pState, frame, 6, &Hideout_Hideout_Happyrat_Dodeadstuff_FUNCTIONCONTROLLER);
        SF.Free(frame);
        return;
    }
    case 0x92E:
    {
        LocalVarEntry* frame = (LocalVarEntry*)SF.Alloc(0x14, __FILE__, __LINE__);
        if (!frame) {} /* PC: __debugbreak(); execution continues */
        AS_InstallCall(pState, frame, 5, &Hideout_Hideout_Happyrat_Dodeadstuff_FUNCTIONCONTROLLER);
        SF.Free(frame);
        return;
    }
    case 0xB3E:
    {
        SwitchStateStruct* sw = (SwitchStateStruct*)SF.Alloc(36, __FILE__, __LINE__);
        if (!sw) {} /* PC: __debugbreak(); execution continues */
        sv->m_fSentFlags |= 1u;
        sw->stateController = &Alllevels_Rat_Happyfunness_STATECONTROLLER;
        AS_InstallSwitchState(pState, (LocalVarEntry*)sw, 4, &AS_SwitchState_FUNCTIONCONTROLLER);
        SF.Free(sw);
        return;
    }
    default:
        if ((sv->m_fSentFlags & 1u) == 0)
            AS_CallParentProcessMessage(pState, msgId, pMsgArg);
        return;
    }
}

/* IMPORTS — _Hideout_Hideout_Happyrat_IMPORTS (0x1003C93D): calls the parent
 * creator's Imports first, then SF.Input on this script's switch-state
 * parameter at +0x194 (4 bytes). */
void Hideout_Hideout_Happyrat_IMPORTS(ScriptState* pState)
{
    /* Parent chain first — Alllevels_Rat_IMPORTS (PC _Alllevels_Rat_IMPORTS
     * @0x1003A915) is not ported yet; expected once Alllevels_Rat code lands: */
    /* Alllevels_Rat_IMPORTS(pState); */

    SF.Input((char*)pState->m_pScriptVariables +
             offsetof(HappyratScriptVars, m_pvSwitchState), 4);
}

/* Initialize — _Hideout_Hideout_Happyrat_STATICINITIALIZERS (0x1003C925). */
void Hideout_Hideout_Happyrat_INITIALIZE(void)
{
    s_lSentRatCount = 0;
    s_fTimerBase = 0.0f;
    s_bHaveTimerBase = 0;
}

/* ================================================================== */
/* Function controllers (input sizes / data exactly as in PC)         */
/* ================================================================== */

/* PC 0x10044314 */
const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_RUN_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Happyrat_RUN,
    0x14, 0x00, NULL, NULL
};

/* PC 0x10044320 */
const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_ENTER_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Happyrat_ENTER,
    0x14, 0x00, NULL, NULL
};

/* PC 0x100443EC — installed by the 0x921 / 0x92E message handlers. */
const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_Dodeadstuff_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Happyrat_Dodeadstuff,
    0x14, 0x00, NULL, NULL
};

/* PC 0x100443C0 — run FC of the unnamed class state (0x100443CC). */
const FUNCTIONCONTROLLER Hideout_Hideout_Happyrat_UnnamedState_100443CC_RunFUNCTIONCONTROLLER =
{
    /* TODO: Finish me after the generated switch-to-imported-state driver at
     * PC sub_1003C95E @0x1003C95E is reversed. */
    TODO_PTR,
    0x1C, 0x08, NULL, NULL    /* PC input 28, data 8 */
};

/* ================================================================== */
/* State controllers (PC 0x1004432C / 0x100443CC)                     */
/* ================================================================== */

/* verbatim string-offsets / imports table (PC 0x1004434C, 16 bytes) */
static uint16_t s_Happyrat_OffsetStrings[] =
{
    0x0008, 0x0008, 0x000A, 0x000D, 0x000D, 0x000D, 0x0006, 0x0000
};

const STATECONTROLLER Hideout_Hideout_Happyrat_ROOTSTATE =
{
    &Hideout_Hideout_Happyrat_RUN_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Happyrat_ENTER_FUNCTIONCONTROLLER,
    NULL,                                     /* m_pDestroy       */
    NULL,                                     /* ProcessMessage   */
    (const void*)&Hideout_Hideout_Happyrat_FunctionsVT[0],
    1,                                        /* m_lLevel         */
    4,                                        /* m_lScriptLevel   */
    &Alllevels_Rat_ROOTSTATE,                 /* parent = Alllevels_Rat root */
    NULL,
    s_Happyrat_OffsetStrings
};

/* Unnamed state (PC 0x100443CC) — state of this class (scriptlevel 4), its
 * run entry is not ported (see the FC above). */
const STATECONTROLLER Hideout_Hideout_Happyrat_UnnamedState_100443CC =
{
    &Hideout_Hideout_Happyrat_UnnamedState_100443CC_RunFUNCTIONCONTROLLER,
    NULL,                                     /* m_pEnter   (PC 0x100443D0 = 0) */
    NULL,                                     /* m_pDestroy */
    NULL,                                     /* ProcessMessage */
    (const void*)&Hideout_Hideout_Happyrat_FunctionsVT[0],
    2,                                        /* m_lLevel         */
    4,                                        /* m_lScriptLevel   */
    &Hideout_Hideout_Happyrat_ROOTSTATE,      /* parent state     */
    NULL,
    NULL
};

/* ================================================================== */
/* Save-game statics (PC array 0x1004435C, verbatim)                  */
/* ================================================================== */

static const SAVEGAMESTATICS s_Happyrat_SaveGameStatics[] =
{
    { 0, 1, &s_bHaveTimerBase },   /* PC {type 0, size 1, byte  @0x10046630} */
    { 0, 4, &s_fTimerBase },       /* PC {type 0, size 4, float @0x10046634} */
    { 0, 4, &s_lSentRatCount },    /* PC {type 0, size 4, int   @0x10046638} */
    { 5, 0, NULL }                 /* SGST_END terminator                    */
};

/* ================================================================== */
/* Virtual tables                                                     */
/* ================================================================== */

/* PC 0x100441E8: the compiled table lists the complete inherited method set
 * (Alllevels_Rat / Basefunc entries at 0x100434B0.. etc. are not reversed);
 * only the records owned by this script are modelled, as in the Canary
 * template. */
const void* const Hideout_Hideout_Happyrat_FunctionsVT[] =
{
    &Hideout_Hideout_Happyrat_RUN_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Happyrat_ENTER_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Happyrat_Dodeadstuff_FUNCTIONCONTROLLER,
    &Hideout_Hideout_Happyrat_UnnamedState_100443CC_RunFUNCTIONCONTROLLER,
    NULL
};

/* PC 0x100441C8 likewise mixes in the parent classes' states. */
const void* const Hideout_Hideout_Happyrat_StatesVT[] =
{
    &Hideout_Hideout_Happyrat_ROOTSTATE,
    &Hideout_Hideout_Happyrat_UnnamedState_100443CC,
    NULL
};

/* ================================================================== */
/* Creator record (PC 0x10044380)                                     */
/* ================================================================== */

const SCRIPTCREATOR Hideout_Happyrat =
{
    "Hideout_Hideout_Happyrat",                 /* PC 0x100443F8 */
    0x198,                                      /* m_lScriptVariablesSize */
    0x24,                                       /* m_lStateVariablesSize  */
    &Hideout_Hideout_Happyrat_ROOTSTATE,        /* PC 0x1004432C */
    &Alllevels_Rat,                             /* parent creator (PC 0x10043BB0) */
    (const void*)&Hideout_Hideout_Happyrat_StatesVT[0],
    (ProcessMessage_t)Hideout_Hideout_Happyrat_PROCESSMESSAGE,
    (VoidFunction_t)Hideout_Hideout_Happyrat_INITIALIZE, /* STATICINITIALIZERS */
    &s_Happyrat_SaveGameStatics[0],
    (VoidFunction_t)Hideout_Hideout_Happyrat_IMPORTS,
    AS_EmptyVoid,                               /* StaticImports (PC: nullsub) */
    AS_EmptyVoid,                               /* UnpackResources (PC: nullsub) */
    AS_EmptyVoid,                               /* UnpackStaticResources (nullsub) */
    (const SCRIPTIMPORT*)&s_Happyrat_OffsetStrings[0]
};
