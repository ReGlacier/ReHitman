#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct _SCRIPTCREATOR
    {
        const char* m_pName;
        int32_t m_lScriptVariablesSize;
        int32_t m_lStateVariablesSize;
        const void* m_pStateController;
        const _SCRIPTCREATOR* m_pParentCreator;
        void* m_pStatesVirtualTable;
        void* ProcessMessage;
        int32_t Initialize;
        const SAVEGAMESTATICS* m_pSaveGameStatics;
        void (*Imports)();
        int32_t StaticImports;
        void (*UnpackResources)();
        int32_t UnpackStaticResources;
        const SCRIPTIMPORT* m_pImports;
    };
    RE_VERIFY_SIZE(_SCRIPTCREATOR, 0x38);

    using SCRIPTCREATOR = _SCRIPTCREATOR;
}