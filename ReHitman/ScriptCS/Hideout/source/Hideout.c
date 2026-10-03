#include <Hideout/Hideout.h>
#include <AllLevels/AllLevels.h>

/*
 * SCRIPTCREATOR metadata for the level-specific "Hideout_*" scripts,
 * reconstructed from the PC_Hideout reference binary (Hideout.dll).
 *
 * Only the name / variable-size / initial-state / parent fields are populated
 * here (the "SCRIPTCREATOR only" scope). Every pointer that still resolves
 * into unreversed script code — the initial state controller, the states
 * vtable, ProcessMessage, Initialize, save-game statics, Imports, Unpack*,
 * and the import table — is written as TODO_PTR (== NULL). `grep -r TODO_PTR`
 * lists exactly what remains to be reversed.
 */

/* Hideout_Levelcontrol */
const SCRIPTCREATOR Hideout_Levelcontrol =
{
    "Hideout_Hideout_Levelcontrol", /* m_pName */
    0x0C,                      /* m_lScriptVariablesSize */
    0x00,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Levelcontrol,   /* m_pParentCreator */
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

/* Hideout_Canary */
const SCRIPTCREATOR Hideout_Canary =
{
    "Hideout_Hideout_Canary",  /* m_pName */
    0x84,                      /* m_lScriptVariablesSize */
    0x1C,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Bird,           /* m_pParentCreator */
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

/* Hideout_Happyrat */
const SCRIPTCREATOR Hideout_Happyrat =
{
    "Hideout_Hideout_Happyrat", /* m_pName */
    0x198,                     /* m_lScriptVariablesSize */
    0x24,                      /* m_lStateVariablesSize */
    TODO_PTR,                  /* m_pStateController */
    &Alllevels_Rat,            /* m_pParentCreator */
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
