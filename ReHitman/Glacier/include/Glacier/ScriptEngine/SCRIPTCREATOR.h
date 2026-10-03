#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct _SCRIPTCREATOR
    {
        const char* m_pName;                            // +0x00
        int32_t m_lScriptVariablesSize;                 // +0x04
        int32_t m_lStateVariablesSize;                  // +0x08
        const STATECONTROLLER* m_pStateController;      // +0x0C
        const _SCRIPTCREATOR* m_pParentCreator;         // +0x10
        const void* m_pStatesVirtualTable;              // +0x14
        ProcessMessage_t ProcessMessage;                // +0x18
        VoidFunction_t Initialize;                      // +0x1C
        const SAVEGAMESTATICS* m_pSaveGameStatics;      // +0x20
        VoidFunction_t Imports;                         // +0x24
        VoidFunction_t StaticImports;                   // +0x28
        VoidFunction_t UnpackResources;                 // +0x2C
        VoidFunction_t UnpackStaticResources;           // +0x30
        const SCRIPTIMPORT* m_pImports;                 // +0x34
    };
    RE_VERIFY_SIZE(_SCRIPTCREATOR, 0x38);

    using SCRIPTCREATOR = _SCRIPTCREATOR;
}