#ifndef __HBM_ALL_LEVELS_ALLLEVELS_LEVELCONTROL_H__
#define __HBM_ALL_LEVELS_ALLLEVELS_LEVELCONTROL_H__

/*
 * Alllevels_Levelcontrol — shared level-control script (root of the
 * level-control chain; Hideout_Levelcontrol derives from it).
 * Reconstructed from PC_Hideout.
 */

#include <AllLevels/ScriptCreator.h>
#include <ScriptRuntime/ScriptSupport.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const SCRIPTCREATOR Alllevels_Levelcontrol;

/* Ported script code (source/Alllevels_Levelcontrol.c), fully reversed from
 * PC including the PROCESSMESSAGE 0x0857 handler. */
float Alllevels_Levelcontrol_RUN(ScriptState* pState);
float Alllevels_Levelcontrol_ENTER(ScriptState* pState);
float Alllevels_Levelcontrol_Idle_RUN(ScriptState* pState);
float Alllevels_Levelcontrol_Missioncompleted(ScriptState* pState);
float Alllevels_Levelcontrol_Missionfailed(ScriptState* pState);
float Alllevels_Levelcontrol_Characterkilled(ScriptState* pState);
void  Alllevels_Levelcontrol_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg);

/* Root-state records referenced by child scripts (e.g. Hideout_Levelcontrol),
 * and the "Idle" state records. All verified against the PC data
 * (source/AllLevels_LevelcontrolParentData.c). */
extern const STATECONTROLLER    Alllevels_Levelcontrol_ROOTSTATE;
extern const STATECONTROLLER    Alllevels_Levelcontrol_State_Idle;
extern const FUNCTIONCONTROLLER Alllevels_Levelcontrol_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Levelcontrol_EnterFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Levelcontrol_Idle_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Levelcontrol_MissioncompletedFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Levelcontrol_MissionfailedFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Levelcontrol_CharacterkilledFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Levelcontrol_IdleFUNCTIONCONTROLLERS[];

/* PC 0x100433F0 — the root state's string-offset blob doubles as the
 * creator's savegame statics (a bare SGST_END terminator). */
extern uint16_t Alllevels_Levelcontrol_OffsetStrings[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HBM_ALL_LEVELS_ALLLEVELS_LEVELCONTROL_H__ */
