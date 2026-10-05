#ifndef __HBM_ALL_LEVELS_ALLLEVELS_PERCEPTIONCONVERTER_H__
#define __HBM_ALL_LEVELS_ALLLEVELS_PERCEPTIONCONVERTER_H__

/*
 * Alllevels_Perceptionconverter — shared perception-converter base script
 * (root of the NPC chain: Basefunc/Rat/Human derive from it).
 * Reconstructed from PC_Hideout (creator 0x10043658, root SC 0x100435C8 with
 * a 16-entry script-function virtual table at 0x10043570, one "Idle"-style
 * state at 0x10043878, 12 savegame statics, import descriptors 0x100435E8).
 *
 * Port in progress (Docs/PC_Hideout_CONTEXT.md §11): RUN / IMPORTS /
 * STATICINITIALIZERS done; ENTER (0x10037921), PROCESSMESSAGE (0x1003A742)
 * and the state functions keep TODO_PTR entries in the FC/SC records below.
 */

#include <AllLevels/ScriptCreator.h>
#include <ScriptRuntime/ScriptSupport.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const SCRIPTCREATOR Alllevels_Perceptionconverter;

/* Ported script code (source/Alllevels_Perceptionconverter.c). */
float Alllevels_Perceptionconverter_RUN(ScriptState* pState);
float Alllevels_Perceptionconverter_ENTER(ScriptState* pState);
void  Alllevels_Perceptionconverter_IMPORTS(ScriptState* pState);
void  Alllevels_Perceptionconverter_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg);
void  Alllevels_Perceptionconverter_INITIALIZE(void); /* _STATICINITIALIZERS 0x10037751 */

/* Anchors (source/Alllevels_Perceptionconverter.c). Ported children embed
 * this chain verbatim. */
extern const STATECONTROLLER    Alllevels_Perceptionconverter_ROOTSTATE;
extern const STATECONTROLLER    Alllevels_Perceptionconverter_State_Idle;
extern const FUNCTIONCONTROLLER Alllevels_Perceptionconverter_RunFUNCTIONCONTROLLER;
extern const FUNCTIONCONTROLLER Alllevels_Perceptionconverter_EnterFUNCTIONCONTROLLER;
extern const void* const        Alllevels_Perceptionconverter_FunctionsVT[];
extern const FUNCTIONCONTROLLER Alllevels_Perceptionconverter_FunctionCONTROLLERS[];

/* SGST list (PC 0x100435F0) — 13 entries incl. the terminator. */
extern const SAVEGAMESTATICS Alllevels_Perceptionconverter_SaveGameStatics[];
extern const SCRIPTIMPORT Alllevels_Perceptionconverter_ImportDescriptors[];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HBM_ALL_LEVELS_ALLLEVELS_PERCEPTIONCONVERTER_H__ */
