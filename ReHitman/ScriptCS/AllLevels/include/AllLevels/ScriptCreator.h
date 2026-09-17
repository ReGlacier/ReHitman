#ifndef __HBM_ALL_LEVELS_SCRIPTCREATOR_H__
#define __HBM_ALL_LEVELS_SCRIPTCREATOR_H__

/*
 * Compatibility shim: the common script-DLL runtime type descriptions
 * (SCRIPTCREATOR, STATECONTROLLER, FUNCTIONCONTROLLER, SAVEGAMESTATICS,
 * SCRIPTIMPORT, SCRIPTFUNCTIONS, INTERNALSCRIPTFUNCTIONS, TODO_PTR) now live
 * in the ScriptRuntime module. This header re-exports them so existing
 * `#include <AllLevels/ScriptCreator.h>` sites keep working.
 */

#include <ScriptRuntime/ScriptRuntime.h>

#endif /* __HBM_ALL_LEVELS_SCRIPTCREATOR_H__ */
