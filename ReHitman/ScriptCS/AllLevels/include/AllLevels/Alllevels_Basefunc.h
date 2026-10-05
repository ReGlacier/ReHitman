#ifndef __HBM_ALL_LEVELS_ALLLEVELS_BASEFUNC_H__
#define __HBM_ALL_LEVELS_ALLLEVELS_BASEFUNC_H__

/*
 * Alllevels_Basefunc — shared base-functionality script
 * (parent: Alllevels_Perceptionconverter).
 * Reconstructed from PC_Hideout.
 */

#include <AllLevels/ScriptCreator.h>
#include <ScriptRuntime/ScriptSupport.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const SCRIPTCREATOR Alllevels_Basefunc;

float Alllevels_Basefunc_RUN(ScriptState* pState);
float Alllevels_Basefunc_ENTER(ScriptState* pState);
float Alllevels_Basefunc_Start_RUN(ScriptState* pState);
void  Alllevels_Basefunc_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg);
void  Alllevels_Basefunc_IMPORTS(ScriptState* pState);
void  Alllevels_Basefunc_INITIALIZE(void);

extern const STATECONTROLLER Alllevels_Basefunc_ROOTSTATE;
extern const STATECONTROLLER Alllevels_Basefunc_State_Start;
extern const void* const Alllevels_Basefunc_StateVT[];
extern const FUNCTIONCONTROLLER Alllevels_Basefunc_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Basefunc_EnterFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Basefunc_Start_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Basefunc_Start_EnterFUNCTIONCONTROLLER;
extern const void* const Alllevels_Basefunc_FunctionsVT[];
extern const SAVEGAMESTATICS Alllevels_Basefunc_SaveGameStatics[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HBM_ALL_LEVELS_ALLLEVELS_BASEFUNC_H__ */
