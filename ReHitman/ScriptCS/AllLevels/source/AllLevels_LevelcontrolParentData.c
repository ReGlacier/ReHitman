/*
 * AllLevels_LevelcontrolParentData.c — Alllevels_Levelcontrol metadata
 * (creator-adjacent records), verified against PC_Hideout 2026-10-05.
 *
 * Script code lives in source/Alllevels_Levelcontrol.c; this file holds the
 * FUNCTIONCONTROLLER / STATECONTROLLER records for the root state (also
 * embedded verbatim by child scripts such as Hideout_Levelcontrol) and the
 * "Idle" state. The Alllevels_Levelcontrol SCRIPTCREATOR itself lives in
 * AllLevels.c (shared); nothing here duplicates it.
 *
 *   creator              0x100433F8
 *   root SC              0x100433D0   strings/savegame-statics 0x100433F0
 *   RUN FC               0x100433B8   entry _Alllevels_Levelcontrol_RUN @0x100372C2
 *   ENTER FC             0x100433C4   entry _Alllevels_Levelcontrol_ENTER @0x100373BE
 *   Idle SC              0x10043444   (level 2, parent = root SC)
 *   Idle RUN FC          0x10043438   entry _Alllevels_Levelcontrol_Idle_RUN @0x100373F4
 *   Idle state FC table  0x10043464   (Missioncompleted / Missionfailed /
 *                                     Characterkilled / characterharmed-Fc)
 */

#include <AllLevels/Alllevels_Levelcontrol.h>
#include <ScriptRuntime/ScriptRuntime.h>

/* ================================================================== */
/* Shared strings blob / savegame statics (PC 0x100433F0)             */
/* ================================================================== */

/* The root state's m_lStringOffsets and the creator's m_pSaveGameStatics
 * point at the SAME bytes in PC (0x100433F0): one SGST_END terminator
 * { type 5, size 0 } doubles as the (empty) string-offset table. */
uint16_t Alllevels_Levelcontrol_OffsetStrings[] =
{
    0x0005, 0x0000, 0x0000, 0x0000
};

/* ================================================================== */
/* Root state function controllers (PC 0x100433B8 / 0x100433C4)       */
/* ================================================================== */

const FUNCTIONCONTROLLER Alllevels_Levelcontrol_RunFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Levelcontrol_RUN,  /* _Alllevels_Levelcontrol_RUN */
    0x14, 0x00, NULL, NULL    /* PC +0x04: input 0x14, no name/strings */
};

const FUNCTIONCONTROLLER Alllevels_Levelcontrol_EnterFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Levelcontrol_ENTER, /* _Alllevels_Levelcontrol_ENTER;
                                                * installed by child ENTERs via
                                                * AS_InstallCall */
    0x14, 0x00, NULL, NULL
};

/* ================================================================== */
/* Root state controller (PC 0x100433D0)                              */
/* ================================================================== */

const STATECONTROLLER Alllevels_Levelcontrol_ROOTSTATE =
{
    &Alllevels_Levelcontrol_RunFUNCTIONCONTROLLER,
    &Alllevels_Levelcontrol_EnterFUNCTIONCONTROLLER,
    NULL,                     /* m_pDestroy       */
    NULL,                     /* ProcessMessage   */
    NULL,                     /* m_pFunctionsVirtualTable (none in PC) */
    1,                        /* m_lLevel         */
    1,                        /* m_lScriptLevel   */
    NULL,                     /* no parent (root of its chain) */
    NULL,
    Alllevels_Levelcontrol_OffsetStrings
};

/* ================================================================== */
/* "Idle" state function controllers                                   */
/* ================================================================== */

/* Idle RUN FC (PC 0x10043438) */
const FUNCTIONCONTROLLER Alllevels_Levelcontrol_Idle_RunFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Levelcontrol_Idle_RUN, /* _Alllevels_Levelcontrol_Idle_RUN */
    0x14, 0x00, NULL, NULL
};

/* Missioncompleted FC (PC 0x10043464; entry in the state FC table below). */
const FUNCTIONCONTROLLER Alllevels_Levelcontrol_MissioncompletedFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Levelcontrol_Missioncompleted,
    0x14, 0x00, NULL, NULL
};

/* Missionfailed FC (PC 0x10043470). PC m_lStringOffsets = 0x100459E4
 * (string-offset table for the imported reason text) — not representable. */
const FUNCTIONCONTROLLER Alllevels_Levelcontrol_MissionfailedFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Levelcontrol_Missionfailed,
    0x18, 0x00, NULL, NULL
};

/* Characterkilled FC (PC 0x1004347C). */
const FUNCTIONCONTROLLER Alllevels_Levelcontrol_CharacterkilledFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Levelcontrol_Characterkilled,
    0x1C, 0x00, NULL, NULL     /* PC input 0x1C: ZREF frame local + padding */
};

/* The FC list PC places right after the Idle STATECONTROLLER (0x10043464),
 * unreferenced elsewhere in the binary. The 4th slot reuses the
 * Characterkilled entry — the PC compiler emitted one body for both the
 * killed/harmed notifications (see the port file). */
const FUNCTIONCONTROLLER Alllevels_Levelcontrol_IdleFUNCTIONCONTROLLERS[] =
{
    { (EntryPoint_t)Alllevels_Levelcontrol_Missioncompleted, 0x14, 0x00, NULL, NULL },
    { (EntryPoint_t)Alllevels_Levelcontrol_Missionfailed,    0x18, 0x00, NULL, NULL },
    { (EntryPoint_t)Alllevels_Levelcontrol_Characterkilled,  0x1C, 0x00, NULL, NULL },
    { (EntryPoint_t)Alllevels_Levelcontrol_Characterkilled,  0x1C, 0x00, NULL, NULL },
};

/* ================================================================== */
/* "Idle" state controller (PC 0x10043444)                             */
/* ================================================================== */

const STATECONTROLLER Alllevels_Levelcontrol_State_Idle =
{
    &Alllevels_Levelcontrol_Idle_RunFUNCTIONCONTROLLER,
    NULL,                     /* m_pEnter                          */
    NULL,                     /* m_pDestroy                        */
    NULL,                     /* ProcessMessage                    */
    NULL,                     /* m_pFunctionsVirtualTable (none in PC) */
    2,                        /* m_lLevel         */
    1,                        /* m_lScriptLevel   */
    &Alllevels_Levelcontrol_ROOTSTATE, /* parent state            */
    NULL,
    /* PC: 0x10037452 — debug-string bytes overlapping the Missioncompleted
     * code (debug-string overlap, not representable; same pattern as the
     * child Hideout_Levelcontrol records). */
    NULL
};
