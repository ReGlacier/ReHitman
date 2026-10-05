#ifndef __HBM_ALL_LEVELS_ALLLEVELS_BASEBOID_H__
#define __HBM_ALL_LEVELS_ALLLEVELS_BASEBOID_H__

/*
 * Alllevels_Baseboid — shared base boid script (root of the boid chain).
 * Fully reversed from PC_Hideout: creator fields, state/FC records and all
 * eight script functions are ported (see source/Alllevels_Baseboid.c and
 * Docs/PC_Hideout_CONTEXT.md §7).
 */

#include <AllLevels/ScriptCreator.h>
#include <ScriptRuntime/ScriptSupport.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const SCRIPTCREATOR Alllevels_Baseboid;

/* Ported script code (source/Alllevels_Baseboid.c). Children install the
 * animation primitives through the FC records below (AS_InstallCall); the
 * var/segment protocol of the two animation channels is documented there. */
float Alllevels_Baseboid_RUN(ScriptState* pState);
float Alllevels_Baseboid_ENTER(ScriptState* pState);
float Alllevels_Baseboid_PlayAnimWait(ScriptState* pState);
float Alllevels_Baseboid_PlayAnimSegment(ScriptState* pState);
float Alllevels_Baseboid_PlayAnimSegmentWait(ScriptState* pState);
float Alllevels_Baseboid_PlayAnimInterpolatedWait(ScriptState* pState);
void  Alllevels_Baseboid_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg);
void  Alllevels_Baseboid_INITIALIZE(void);   /* _ALLLEVELS_BASEBOID_STATICINITIALIZERS */

extern const SAVEGAMESTATICS    Alllevels_Baseboid_SaveGameStatics[]; /* PC 0x1003E5B0 */

extern const STATECONTROLLER    Alllevels_Baseboid_ROOTSTATE;
extern const FUNCTIONCONTROLLER Alllevels_Baseboid_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Baseboid_EnterFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayanimwaitFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayanimsegmentFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayanimsegmentwaitFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Baseboid_PlayaniminterpolatedwaitFUNCTIONCONTROLLER;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HBM_ALL_LEVELS_ALLLEVELS_BASEBOID_H__ */
