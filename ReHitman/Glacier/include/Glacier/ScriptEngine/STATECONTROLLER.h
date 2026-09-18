#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/FUNCTIONCONTROLLER.h>
#include <cstdint>


namespace Glacier
{
    struct _STATECONTROLLER;

    struct _STATECONTROLLER
    {
        const FUNCTIONCONTROLLER* m_pRun;
        const FUNCTIONCONTROLLER* m_pEnter;
        const FUNCTIONCONTROLLER* m_pDestroy;
        void (*ProcessMessage)();
        const void* m_pFunctionsVirtualTable;
        const uint16_t m_lLevel;
        const uint16_t m_lScriptLevel;
        const _STATECONTROLLER* m_pParent;
        const char* m_pName;
        uint16_t* m_lStringOffsets;
    };

    using STATECONTROLLER = _STATECONTROLLER;
}