/*
 * AllLevelsRatParentData.c — parent-side (Alllevels_Rat) records required by
 * the child script "Hideout_Hideout_Happyrat" (see Hideout_Happyrat.c).
 *
 * Every value below is transcribed verbatim from PC_Hideout (Hideout.dll):
 *
 *   Alllevels_Rat creator          0x10043BB0 (kept in AllLevels.c)
 *   Alllevels_Rat root SC          0x10043B0C
 *     FC RUN  0x10043AF4 -> sub_1003A819 (_Alllevels_Rat_RUN)      NOT ported
 *     FC ENTER 0x10043B00 -> sub_1003A936 (_Alllevels_Rat_ENTER)   NOT ported
 *   Rat state "Happyfunness"       0x10043C14 (scriptlevel 3, parent = root)
 *     FC 0x10043C08 -> _Hideout_Hideout_Happyrat_Happyfunness_RUN (PORTED in
 *                      Hideout_Happyrat.c — the child overrides this state)
 *   Rat state 0x10043B2C           (scriptlevel 3, parent = root)
 *     FC run   0x10043940 -> sub_1003B5E6  NOT ported
 *     FC enter 0x1004394C -> sub_1000C9AE  NOT ported
 *     ProcessMessage -> sub_1003B6F6       NOT ported
 *   Rat method FCs used by Happyfunness_RUN:
 *     0x100439AC -> sub_1003AF52 (input 0x14)               NOT ported
 *     0x10043BFC -> sub_1003AE72 (input 0x20, data 0x0C)   NOT ported
 *
 * The functions virtual table used by these states (PC 0x100439C8) is the
 * complete inherited method table of the Rat class (~70 records into the
 * Basefunc / Levelcontrol blobs); it is not reversed, so the slots are NULL
 * with the PC address noted. Unported function entries stay TODO_PTR per the
 * AllLevels.c precedent.
 */

#include <AllLevels/Alllevels_Rat.h>
#include <ScriptRuntime/ScriptSupport.h>

/* Ported in Hideout_Happyrat.c — referenced here by the "Happyfunness" FC
 * record whose entry point it overrides. */
float Hideout_Hideout_Happyrat_Happyfunness_RUN(ScriptState* pState);

/* ================================================================== */
/* Alllevels_Rat root state records (PC 0x10043AF4 / 0x10043B00 / 0x10043B0C) */
/* ================================================================== */

const FUNCTIONCONTROLLER Alllevels_Rat_RunFUNCTIONCONTROLLER =
{
    /* TODO: Finish me after Alllevels_Rat::_RUN reversed
     * (PC entry sub_1003A819 @0x1003A819) */
    TODO_PTR,
    0x14, 0x00, NULL, NULL
};

const FUNCTIONCONTROLLER Alllevels_Rat_EnterFUNCTIONCONTROLLER =
{
    /* TODO: Finish me after Alllevels_Rat::_ENTER reversed
     * (PC entry sub_1003A936 @0x1003A936); installed through AS_InstallCall
     * by the child ENTER (Hideout_Hideout_Happyrat_ENTER). */
    TODO_PTR,
    0x14, 0x00, NULL, NULL
};

const STATECONTROLLER Alllevels_Rat_ROOTSTATE =
{
    &Alllevels_Rat_RunFUNCTIONCONTROLLER,
    &Alllevels_Rat_EnterFUNCTIONCONTROLLER,
    NULL,                         /* m_pDestroy                          */
    NULL,                         /* ProcessMessage                      */
    NULL,                         /* FunctionsVT: PC 0x100439C8, unreversed */
    1,                            /* m_lLevel                            */
    3,                            /* m_lScriptLevel                      */
    TODO_PTR,                     /* parent: PC Alllevels_Basefunc root
                                   * state controller @0x1003EA1C        */
    NULL,
    NULL
};

/* ================================================================== */
/* Rat-class "Happyfunness" state (PC 0x10043C14) — the state the child
 * script's ROOT RUN and its 0xB3E message handler switch into. Only the
 * RUN entry is ported (in Hideout_Happyrat.c).                       */
/* ================================================================== */

const FUNCTIONCONTROLLER Alllevels_Rat_Happyfunness_RunFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Hideout_Hideout_Happyrat_Happyfunness_RUN,
    0x14, 0x00, NULL, NULL    /* PC 0x10043C08: input 20, data 0 */
};

const STATECONTROLLER Alllevels_Rat_Happyfunness_STATECONTROLLER =
{
    &Alllevels_Rat_Happyfunness_RunFUNCTIONCONTROLLER,
    NULL,                         /* m_pEnter       (PC 0x10043C18 = 0) */
    NULL,                         /* m_pDestroy                          */
    NULL,                         /* ProcessMessage                      */
    NULL,                         /* FunctionsVT: PC 0x100439C8, unreversed */
    2,                            /* m_lLevel                            */
    3,                            /* m_lScriptLevel                      */
    &Alllevels_Rat_ROOTSTATE,     /* parent state (PC 0x10043B0C)       */
    NULL,
    NULL
};

/* ================================================================== */
/* Rat-class state at PC 0x10043B2C — the switch target of
 * Hideout_Hideout_Happyrat_Dodeadstuff. All of its functions are the
 * parent's and stay TODO_PTR.                                        */
/* ================================================================== */

const FUNCTIONCONTROLLER Alllevels_Rat_UnnamedState_10043B2C_RunFUNCTIONCONTROLLER =
{
    /* TODO: Finish me after the Rat state run at PC sub_1003B5E6 @0x1003B5E6
     * is reversed. */
    TODO_PTR,
    0x14, 0x00, NULL, NULL    /* PC 0x10043940 */
};

const FUNCTIONCONTROLLER Alllevels_Rat_UnnamedState_10043B2C_EnterFUNCTIONCONTROLLER =
{
    /* TODO: Finish me after the Rat state enter at PC sub_1000C9AE
     * @0x1000C9AE is reversed. */
    TODO_PTR,
    0x14, 0x00, NULL, NULL    /* PC 0x1004394C */
};

const STATECONTROLLER Alllevels_Rat_HappyStateCONTROLLER_10043B2C =
{
    &Alllevels_Rat_UnnamedState_10043B2C_RunFUNCTIONCONTROLLER,
    &Alllevels_Rat_UnnamedState_10043B2C_EnterFUNCTIONCONTROLLER,
    NULL,                         /* m_pDestroy                          */
    /* TODO: Finish me after the state-side ProcessMessage at PC
     * sub_1003B6F6 @0x1003B6F6 is reversed. */
    TODO_PTR,                     /* ProcessMessage                      */
    NULL,                         /* FunctionsVT: PC 0x100439C8, unreversed */
    2,                            /* m_lLevel                            */
    3,                            /* m_lScriptLevel                      */
    &Alllevels_Rat_ROOTSTATE,     /* parent state (PC 0x10043B0C)       */
    NULL,
    NULL
};

/* ================================================================== */
/* Rat method FCs called by the ported Happyfunness_RUN (random 70/30
 * pick; entries are parent-class functions, not ported).            */
/* ================================================================== */

const FUNCTIONCONTROLLER Alllevels_Rat_UnreversedFUNCTIONCONTROLLER_100439AC =
{
    /* TODO: Finish me after the Rat method at PC sub_1003AF52 @0x1003AF52
     * is reversed. */
    TODO_PTR,
    0x14, 0x00, NULL, NULL    /* PC input 20, data 0 */
};

const FUNCTIONCONTROLLER Alllevels_Rat_UnreversedFUNCTIONCONTROLLER_10043BFC =
{
    /* TODO: Finish me after the Rat method at PC sub_1003AE72 @0x1003AE72
     * is reversed (it receives 12 bytes of frame parameters). */
    TODO_PTR,
    0x20, 0x0C, NULL, NULL    /* PC 0x10043BFC: input 32, data 12 */
};
