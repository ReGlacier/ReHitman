#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    union _SpecialScriptReturnType
    {
        const STATECONTROLLER* m_pForkStateController;
        ScriptState* m_pForkReturnScriptState;
        const char* m_pScriptName;
        uint32_t m_lPriority;
    };
}