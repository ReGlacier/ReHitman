#include <Hideout/Hideout.h>
#include <AllLevels/AllLevels.h>

/*
 * SCRIPTCREATOR metadata for the level-specific "Hideout_*" scripts.
 *
 * Every "Hideout_*" script now lives in its own translation unit:
 *   source/Hideout_Levelcontrol.c, source/Hideout_Canary.c,
 *   source/Hideout_Happyrat.c
 * Each file carries its creator record, state/function controllers and shared
 * tables ported from PC_Hideout. Script bodies still being ported keep their
 * unresolved pointers as TODO_PTR; `grep -r TODO_PTR` lists what remains.
 */
/* Hideout_Levelcontrol is ported in source/Hideout_Levelcontrol.c;
 * Hideout_Happyrat is ported in source/Hideout_Happyrat.c. */
