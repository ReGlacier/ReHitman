#ifndef __HBM_ALL_LEVELS_H__
#define __HBM_ALL_LEVELS_H__

/*
 * AllLevels — shared-script-code layer, linked into every scene DLL.
 *
 * Declares the SCRIPTCREATOR metadata for the shared "Alllevels_*" scripts
 * reconstructed from the PC_Hideout reference binary (Hideout.dll).
 *
 * Script creator list (PC_Hideout Scripts table, AllLevels subset):
 *   Alllevels_Baseboid, Alllevels_Bird, Alllevels_Rat,
 *   Alllevels_Levelcontrol, Alllevels_Perceptionconverter, Alllevels_Basefunc,
 *   Alllevels_Vehicles_Car, Alllevels_Human, Alllevels_Civilian,
 *   Alllevels_Armed, Alllevels_Guard
 */

#include <AllLevels/ScriptCreator.h>
#include <AllLevels/Alllevels_Baseboid.h>
#include <AllLevels/Alllevels_Bird.h>
#include <AllLevels/Alllevels_Rat.h>
#include <AllLevels/Alllevels_Levelcontrol.h>
#include <AllLevels/Alllevels_Perceptionconverter.h>
#include <AllLevels/Alllevels_Basefunc.h>
#include <AllLevels/Alllevels_Vehicles_Car.h>
#include <AllLevels/Alllevels_Human.h>
#include <AllLevels/Alllevels_Civilian.h>
#include <AllLevels/Alllevels_Armed.h>
#include <AllLevels/Alllevels_Guard.h>

#endif /* __HBM_ALL_LEVELS_H__ */
