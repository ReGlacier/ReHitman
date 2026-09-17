#include <Hideout/Hideout.h>
#include <AllLevels/AllLevels.h>
#include <ScriptRuntime/ScriptRuntime.h>

/*
 * ScriptDLL contract tables (ordinals 1-4), reconstructed from the
 * PC_Hideout reference binary (Hideout.dll).
 *
 *   SF            @1 DATA  SCRIPTFUNCTIONS (0x6C), zero-filled in the image;
 *                          the engine writes all 27 slots at AttachSceneScripts.
 *   ISF           @2 DATA  INTERNALSCRIPTFUNCTIONS — RunningThread; the engine
 *                          clears it then points its own ISF at this field.
 *   Scripts       @3 DATA  SCRIPTCREATOR** — [0] = count, creators at [1..],
 *                          null-terminated.
 *   ScriptImports @4 DATA  void*[0x2CC] import block; zero-filled in the image;
 *                          the engine overwrites it with ScriptInterfaces.
 *
 * The creator list below preserves the PC_Hideout Scripts table order
 * (AllLevels shared scripts interleaved with the level-specific Hideout ones).
 */

SCRIPTFUNCTIONS SF;              /* @1 DATA */
INTERNALSCRIPTFUNCTIONS ISF;    /* @2 DATA */

/* Ordinal 3: Scripts — SCRIPTCREATOR** list, PC_Hideout order */
const SCRIPTCREATOR* const Scripts[] =
{
    (const SCRIPTCREATOR*)14,   /* [0] count */
    &Alllevels_Baseboid,        /* [1] */
    &Alllevels_Bird,            /* [2] */
    &Alllevels_Rat,             /* [3] */
    &Hideout_Levelcontrol,      /* [4] */
    &Hideout_Canary,            /* [5] */
    &Hideout_Happyrat,          /* [6] */
    &Alllevels_Levelcontrol,    /* [7] */
    &Alllevels_Perceptionconverter, /* [8] */
    &Alllevels_Basefunc,        /* [9] */
    &Alllevels_Vehicles_Car,    /* [10] */
    &Alllevels_Human,           /* [11] */
    &Alllevels_Civilian,        /* [12] */
    &Alllevels_Armed,           /* [13] */
    &Alllevels_Guard,           /* [14] */
    NULL                        /* terminator */
};

/* Ordinal 4: ScriptImports — void*[0x2CC] import block */
void* ScriptImports[0x2CC];     /* @4 DATA */
