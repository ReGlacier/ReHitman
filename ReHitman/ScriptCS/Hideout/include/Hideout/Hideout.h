#ifndef __HBM_HIDEOUT_H__
#define __HBM_HIDEOUT_H__

/*
 * Hideout — level-specific script-code layer for the Hideout DLL.
 *
 * Declares the SCRIPTCREATOR metadata for the level-specific "Hideout_*"
 * scripts reconstructed from the PC_Hideout reference binary (Hideout.dll).
 *
 * Script creator list (PC_Hideout Scripts table, Hideout subset):
 *   Hideout_Levelcontrol, Hideout_Canary, Hideout_Happyrat
 *
 * These are the level-specific scripts; the shared "Alllevels_*" scripts
 * live in the AllLevels layer (see AllLevels/AllLevels.h).
 */

#include <AllLevels/ScriptCreator.h>
#include <Hideout/Hideout_Levelcontrol.h>
#include <Hideout/Hideout_Canary.h>
#include <Hideout/Hideout_Happyrat.h>

#endif /* __HBM_HIDEOUT_H__ */
