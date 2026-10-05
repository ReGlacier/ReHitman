#ifndef __HBM_ALL_LEVELS_ALLLEVELS_RAT_H__
#define __HBM_ALL_LEVELS_ALLLEVELS_RAT_H__

/*
 * Alllevels_Rat — shared rat boid script (parent: Alllevels_Basefunc).
 * Reconstructed from PC_Hideout.
 */

#include <AllLevels/ScriptCreator.h>
#include <ScriptRuntime/ScriptSupport.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const SCRIPTCREATOR Alllevels_Rat;

/* Parent-side records required by child scripts (Hideout_Happyrat). The
 * Alllevels_Rat script code itself is not ported yet — those entries stay
 * TODO_PTR; see AllLevels/source/AllLevelsRatParentData.c for the verified
 * PC_Hideout addresses. */
extern const STATECONTROLLER       Alllevels_Rat_ROOTSTATE;
extern const FUNCTIONCONTROLLER    Alllevels_Rat_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER    Alllevels_Rat_EnterFUNCTIONCONTROLLER;

/* Rat-class state overridden by Hideout_Hideout_Happyrat (PC 0x10043C14). */
extern const STATECONTROLLER       Alllevels_Rat_Happyfunness_STATECONTROLLER;
extern const FUNCTIONCONTROLLER    Alllevels_Rat_Happyfunness_RunFUNCTIONCONTROLLER;

/* Rat-class state entered by Hideout_Hideout_Happyrat_Dodeadstuff
 * (PC 0x10043B2C). */
extern const STATECONTROLLER       Alllevels_Rat_HappyStateCONTROLLER_10043B2C;

/* Rat method FCs entered by the happy-rat flow (PC 0x100439AC / 0x10043BFC). */
extern const FUNCTIONCONTROLLER    Alllevels_Rat_UnreversedFUNCTIONCONTROLLER_100439AC;
extern const FUNCTIONCONTROLLER    Alllevels_Rat_UnreversedFUNCTIONCONTROLLER_10043BFC;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HBM_ALL_LEVELS_ALLLEVELS_RAT_H__ */
