#ifndef __HBM_ALL_LEVELS_ALLLEVELS_BIRD_H__
#define __HBM_ALL_LEVELS_ALLLEVELS_BIRD_H__

/*
 * Alllevels_Bird — shared bird boid script (parent: Alllevels_Baseboid).
 * Reconstructed from PC_Hideout.
 */

#include <AllLevels/ScriptCreator.h>
#include <ScriptRuntime/ScriptSupport.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const SCRIPTCREATOR Alllevels_Bird;

/* Ported script code (source/Alllevels_Bird.c). The parent-side records
 * referenced by child scripts (e.g. Hideout_Canary chains this ENTER) are
 * declared here; entries for code that is still unported stay TODO_PTR in
 * AllLevels.c. */
float Alllevels_Bird_ENTER(ScriptState* pState);

extern const STATECONTROLLER       Alllevels_Bird_ROOTSTATE;
extern const FUNCTIONCONTROLLER    Alllevels_Bird_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER    Alllevels_Bird_EnterFUNCTIONCONTROLLER;
extern const void* const           Alllevels_Bird_FunctionsVT[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HBM_ALL_LEVELS_ALLLEVELS_BIRD_H__ */
