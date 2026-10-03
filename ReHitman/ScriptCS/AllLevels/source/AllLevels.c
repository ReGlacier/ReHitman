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

/* Alllevels_Baseboid */
const SCRIPTCREATOR Alllevels_Baseboid =
{
    "Alllevels_Baseboid",      /* m_pName */
    0x0C,                      /* m_lScriptVariablesSize */
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

/* Alllevels_Levelcontrol */
const SCRIPTCREATOR Alllevels_Levelcontrol =
{
    "Alllevels_Levelcontrol",  /* m_pName */
    0x04,                      /* m_lScriptVariablesSize */
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

/* Alllevels_Perceptionconverter */
const SCRIPTCREATOR Alllevels_Perceptionconverter =
{
    "Alllevels_Perceptionconverter", /* m_pName */
    0xEC,                      /* m_lScriptVariablesSize */
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

/* Alllevels_Basefunc */
const SCRIPTCREATOR Alllevels_Basefunc =
{
    "Alllevels_Basefunc",      /* m_pName */
    0x17C,                     /* m_lScriptVariablesSize */
    0x24,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Perceptionconverter, /* m_pParentCreator */
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
