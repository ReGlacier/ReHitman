#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct _LocalVarEntry
    {
        const FUNCTIONCONTROLLER* m_pFunctionController;
        struct _LocalVarEntry* m_pNextVariables;
        struct _LocalVarEntry* m_pPrevVariables;
        uint16_t m_lFunctionIndex;
        uint16_t m_lExitFunctionIndex;
        uint16_t m_lNextVariablesSize;
        uint16_t m_lAlignment;
    };
    RE_VERIFY_SIZE(_LocalVarEntry, 0x14);

    using LocalVarEntry = _LocalVarEntry;
}