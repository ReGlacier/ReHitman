#include <AllLevels/AllLevels.h>

/*
 * SCRIPTCREATOR metadata for the shared "Alllevels_*" scripts, reconstructed
 * from the PC_Hideout reference binary (Hideout.dll).
 *
 * Only the name / variable-size / initial-state / parent fields are populated
 * here (the "SCRIPTCREATOR only" scope). Every pointer that still resolves
 * into unreversed script code — the initial state controller, the states
 * vtable, ProcessMessage, Initialize, save-game statics, Imports, Unpack*,
 * and the import table — is written as TODO_PTR (== NULL). `grep -r TODO_PTR`
 * lists exactly what remains to be reversed.
 */

/* Alllevels_Baseboid — fully reversed (PC 0x1003E5C0). Script code lives in
 * source/Alllevels_Baseboid.c; Imports/StaticImports/Unpack* are the shared
 * empty stub (PC nullsub_1), and Baseboid is the root script: no parent
 * creator, no states virtual table, no import table. */
const SCRIPTCREATOR Alllevels_Baseboid =
{
    "Alllevels_Baseboid",                  /* m_pName (PC 0x1003E630) */
    0x0C,                      /* m_lScriptVariablesSize */
    0x00,                      /* m_lStateVariablesSize */
    &Alllevels_Baseboid_ROOTSTATE,          /* m_pStateController (PC 0x1003E590) */
    NULL,                      /* m_pParentCreator (none) */
    NULL,                      /* m_pStatesVirtualTable (none) */
    (ProcessMessage_t)Alllevels_Baseboid_PROCESSMESSAGE, /* PC 0x100029D1 */
    (VoidFunction_t)Alllevels_Baseboid_INITIALIZE,       /* STATICINITIALIZERS 0x1000270D */
    &Alllevels_Baseboid_SaveGameStatics[0],              /* PC 0x1003E5B0 */
    AS_EmptyVoid,              /* Imports (shared empty stub) */
    AS_EmptyVoid,              /* StaticImports */
    AS_EmptyVoid,              /* UnpackResources */
    AS_EmptyVoid,              /* UnpackStaticResources */
    NULL                       /* m_pImports */
};

/* Alllevels_Bird */
const SCRIPTCREATOR Alllevels_Bird =
{
    "Alllevels_Bird",          /* m_pName */
    0x78,                      /* m_lScriptVariablesSize */
    0x1C,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Baseboid,       /* m_pParentCreator */
    TODO_PTR,                  /* m_pStatesVirtualTable */
    TODO_PTR,                  /* ProcessMessage */
    TODO_PTR,                  /* Initialize */
    TODO_PTR,                  /* m_pSaveGameStatics */
    TODO_PTR,                  /* Imports */
    TODO_PTR,                  /* StaticImports */
    TODO_PTR,                  /* UnpackResources */
    TODO_PTR,                  /* UnpackStaticResources */
    TODO_PTR                   /* m_pImports */
};

/* Alllevels_Rat */
const SCRIPTCREATOR Alllevels_Rat =
{
    "Alllevels_Rat",           /* m_pName */
    0x190,                     /* m_lScriptVariablesSize */
    0x24,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Basefunc,       /* m_pParentCreator */
    TODO_PTR,                  /* m_pStatesVirtualTable */
    TODO_PTR,                  /* ProcessMessage */
    TODO_PTR,                  /* Initialize */
    TODO_PTR,                  /* m_pSaveGameStatics */
    TODO_PTR,                  /* Imports */
    TODO_PTR,                  /* StaticImports */
    TODO_PTR,                  /* UnpackResources */
    TODO_PTR,                  /* UnpackStaticResources */
    TODO_PTR                   /* m_pImports */
};

/* Alllevels_Levelcontrol — root level-control script, reversed 2026-10-05
 * (PC 0x100433F8). Script code in source/Alllevels_Levelcontrol.c; state/FC
 * records in source/AllLevels_LevelcontrolParentData.c. Root creator: no
 * parent, no states virtual table, no import table; Initialize/Imports/
 * StaticImports/Unpack* are the shared empty stub (PC nullsub_1), and the
 * savegame-statics pointer aliases the root state's string-offset blob
 * (PC 0x100433F0, a bare SGST_END terminator). */
const SCRIPTCREATOR Alllevels_Levelcontrol =
{
    "Alllevels_Levelcontrol",  /* m_pName (PC 0x10043494) */
    0x04,                      /* m_lScriptVariablesSize */
    0x00,                      /* m_lStateVariablesSize  */
    &Alllevels_Levelcontrol_ROOTSTATE, /* PC 0x100433D0 */
    NULL,                      /* m_pParentCreator (none) */
    NULL,                      /* m_pStatesVirtualTable (none in PC) */
    (ProcessMessage_t)Alllevels_Levelcontrol_PROCESSMESSAGE, /* PC 0x10037646 */
    AS_EmptyVoid,              /* Initialize (shared empty stub) */
    (const SAVEGAMESTATICS*)&Alllevels_Levelcontrol_OffsetStrings[0], /* PC 0x100433F0 */
    AS_EmptyVoid,              /* Imports */
    AS_EmptyVoid,              /* StaticImports */
    AS_EmptyVoid,              /* UnpackResources */
    AS_EmptyVoid,              /* UnpackStaticResources */
    NULL                       /* m_pImports */
};

/* Alllevels_Perceptionconverter */
const SCRIPTCREATOR Alllevels_Perceptionconverter =
{
    "Alllevels_Perceptionconverter", /* m_pName */
    0xEC,                      /* m_lScriptVariablesSize */
    0x00,                      /* m_lStateVariablesSize */
    &Alllevels_Perceptionconverter_ROOTSTATE,
    NULL,                      /* m_pParentCreator */
    NULL,                      /* m_pStatesVirtualTable */
    (ProcessMessage_t)Alllevels_Perceptionconverter_PROCESSMESSAGE,
    (VoidFunction_t)Alllevels_Perceptionconverter_INITIALIZE,
    &Alllevels_Perceptionconverter_SaveGameStatics[0],
    (VoidFunction_t)Alllevels_Perceptionconverter_IMPORTS,
    AS_EmptyVoid,
    AS_EmptyVoid,
    AS_EmptyVoid,
    Alllevels_Perceptionconverter_ImportDescriptors
};

/* Alllevels_Basefunc */
const SCRIPTCREATOR Alllevels_Basefunc =
{
    "Alllevels_Basefunc",      /* m_pName */
    0x17C,                     /* m_lScriptVariablesSize */
    0x24,                      /* m_lStateVariablesSize */
    &Alllevels_Basefunc_ROOTSTATE,
    &Alllevels_Perceptionconverter, /* m_pParentCreator */
    NULL, /* states VT pending exact child-state records */
    (ProcessMessage_t)Alllevels_Basefunc_PROCESSMESSAGE,
    (VoidFunction_t)Alllevels_Basefunc_INITIALIZE,
    &Alllevels_Basefunc_SaveGameStatics[0],
    (VoidFunction_t)Alllevels_Basefunc_IMPORTS,
    AS_EmptyVoid,
    AS_EmptyVoid,
    AS_EmptyVoid,
    NULL /* import descriptor blob pending */
};

/* Alllevels_Vehicles_Car */
const SCRIPTCREATOR Alllevels_Vehicles_Car =
{
    "Alllevels_Vehicles_Car",  /* m_pName */
    0x54,                      /* m_lScriptVariablesSize */
    0x00,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    TODO_PTR,                  /* m_pParentCreator */
    TODO_PTR,                  /* m_pStatesVirtualTable */
    TODO_PTR,                  /* ProcessMessage */
    TODO_PTR,                  /* Initialize */
    TODO_PTR,                  /* m_pSaveGameStatics */
    TODO_PTR,                  /* Imports */
    TODO_PTR,                  /* StaticImports */
    TODO_PTR,                  /* UnpackResources */
    TODO_PTR,                  /* UnpackStaticResources */
    TODO_PTR                   /* m_pImports */
};

/* Alllevels_Human */
const SCRIPTCREATOR Alllevels_Human =
{
    "Alllevels_Human",         /* m_pName */
    0x270,                     /* m_lScriptVariablesSize */
    0x3C,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Basefunc,       /* m_pParentCreator */
    TODO_PTR,                  /* m_pStatesVirtualTable */
    TODO_PTR,                  /* ProcessMessage */
    TODO_PTR,                  /* Initialize */
    TODO_PTR,                  /* m_pSaveGameStatics */
    TODO_PTR,                  /* Imports */
    TODO_PTR,                  /* StaticImports */
    TODO_PTR,                  /* UnpackResources */
    TODO_PTR,                  /* UnpackStaticResources */
    TODO_PTR                   /* m_pImports */
};

/* Alllevels_Civilian */
const SCRIPTCREATOR Alllevels_Civilian =
{
    "Alllevels_Civilian",      /* m_pName */
    0x290,                     /* m_lScriptVariablesSize */
    0x3C,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Human,          /* m_pParentCreator */
    TODO_PTR,                  /* m_pStatesVirtualTable */
    TODO_PTR,                  /* ProcessMessage */
    TODO_PTR,                  /* Initialize */
    TODO_PTR,                  /* m_pSaveGameStatics */
    TODO_PTR,                  /* Imports */
    TODO_PTR,                  /* StaticImports */
    TODO_PTR,                  /* UnpackResources */
    TODO_PTR,                  /* UnpackStaticResources */
    TODO_PTR                   /* m_pImports */
};

/* Alllevels_Armed */
const SCRIPTCREATOR Alllevels_Armed =
{
    "Alllevels_Armed",         /* m_pName */
    0x2A0,                     /* m_lScriptVariablesSize */
    0x3C,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Civilian,       /* m_pParentCreator */
    TODO_PTR,                  /* m_pStatesVirtualTable */
    TODO_PTR,                  /* ProcessMessage */
    TODO_PTR,                  /* Initialize */
    TODO_PTR,                  /* m_pSaveGameStatics */
    TODO_PTR,                  /* Imports */
    TODO_PTR,                  /* StaticImports */
    TODO_PTR,                  /* UnpackResources */
    TODO_PTR,                  /* UnpackStaticResources */
    TODO_PTR                   /* m_pImports */
};

/* Alllevels_Guard */
const SCRIPTCREATOR Alllevels_Guard =
{
    "Alllevels_Guard",         /* m_pName */
    0x2DC,                     /* m_lScriptVariablesSize */
    0x3C,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Civilian,       /* m_pParentCreator */
    TODO_PTR,                  /* m_pStatesVirtualTable */
    TODO_PTR,                  /* ProcessMessage */
    TODO_PTR,                  /* Initialize */
    TODO_PTR,                  /* m_pSaveGameStatics */
    TODO_PTR,                  /* Imports */
    TODO_PTR,                  /* StaticImports */
    TODO_PTR,                  /* UnpackResources */
    TODO_PTR,                  /* UnpackStaticResources */
    TODO_PTR                   /* m_pImports */
};

/* ==================================================================
 * Parent-side state records (verified PC_Hideout data 2026-10-05)
 *
 * These anchors exist so child scripts (Hideout_*) can embed the verbatim
 * parent chains. Function bodies are ported per script in
 * source/Alllevels_Baseboid.c and source/Alllevels_Bird.c; anything still
 * unported keeps a TODO_PTR entry pointer with the IDA name noted so the
 * chains stay linkable.
 * ================================================================== */

/* Alllevels_Baseboid — FC records (PC 0x1003E578 / 0x1003E584) */
const FUNCTIONCONTROLLER Alllevels_Baseboid_RunFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Baseboid_RUN,      /* _Alllevels_Baseboid_RUN */
    0x14, 0x00, NULL, NULL    /* PC +0x04: initial frame size */
};

const FUNCTIONCONTROLLER Alllevels_Baseboid_EnterFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Baseboid_ENTER,    /* PC _Alllevels_Baseboid_ENTER */
    0x14, 0x00, NULL, NULL
};

/* Baseboid PlayAnimWait FC (PC 0x1003E600): input 0x18, called via
 * AS_InstallCall from other scripts' state functions. */
const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayanimwaitFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Baseboid_PlayAnimWait, /* _Alllevels_Baseboid_PlayAnimWait */
    0x18, 0x00, NULL, NULL
};

/* Baseboid animation-segment FCs (PC 0x1003E60C / 0x1003E618 / 0x1003E624):
 * installed by child scripts through AS_InstallCall. */
const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayanimsegmentFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Baseboid_PlayAnimSegment,
    0x28, 0x00, NULL, NULL
};

const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayanimsegmentwaitFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Baseboid_PlayAnimSegmentWait,
    0x28, 0x00, NULL, NULL
};

const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayaniminterpolatedwaitFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Baseboid_PlayAnimInterpolatedWait,
    0x44, 0x00, NULL, NULL
};

/* Alllevels_Baseboid root state (PC 0x1003E590) */
const STATECONTROLLER Alllevels_Baseboid_ROOTSTATE =
{
    &Alllevels_Baseboid_RunFUNCTIONCONTROLLER,
    &Alllevels_Baseboid_EnterFUNCTIONCONTROLLER,
    NULL,                     /* m_pDestroy       */
    NULL,                     /* ProcessMessage   */
    NULL,                     /* m_pFunctionsVirtualTable (none in PC) */
    1,                        /* m_lLevel         */
    1,                        /* m_lScriptLevel   */
    NULL,                     /* no parent        */
    NULL,
    NULL
};

/* Alllevels_Bird — FC records (PC 0x1003F1A8 / 0x1003F1B4) */
const FUNCTIONCONTROLLER Alllevels_Bird_RunFUNCTIONCONTROLLER =
{
    /* TODO: Finish me after Alllevels_Bird::_RUN reversed
     * (PC entry _Alllevels_Bird_RUN @0x1000A507) */
    TODO_PTR,
    0x14, 0x00, NULL, NULL
};

const FUNCTIONCONTROLLER Alllevels_Bird_EnterFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Bird_ENTER,        /* PC _Alllevels_Bird_ENTER;
                                                * installed via AS_InstallCall
                                                * by child ENTERs */
    0x14, 0x00, NULL, NULL
};

/* Alllevels_Bird functions virtual table (PC 0x1003F184). TODO: Finish me
 * after Alllevels_Bird reversed — remaining vfunc entries unknown; the two
 * state FCs above are chained for now. */
const void* const Alllevels_Bird_FunctionsVT[] =
{
    &Alllevels_Bird_RunFUNCTIONCONTROLLER,
    &Alllevels_Bird_EnterFUNCTIONCONTROLLER,
    NULL
};

/* Alllevels_Bird root state (PC 0x1003F1C0) */
const STATECONTROLLER Alllevels_Bird_ROOTSTATE =
{
    &Alllevels_Bird_RunFUNCTIONCONTROLLER,
    &Alllevels_Bird_EnterFUNCTIONCONTROLLER,
    NULL,                     /* m_pDestroy       */
    NULL,                     /* ProcessMessage   */
    (const void*)&Alllevels_Bird_FunctionsVT[0],
    1,                        /* m_lLevel         */
    2,                        /* m_lScriptLevel   */
    &Alllevels_Baseboid_ROOTSTATE,
    NULL,
    NULL                      /* string offsets @0x1003F1E0: TODO */
};
