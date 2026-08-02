#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    enum SaveRefType
    {
        SRT_NULL = 0,
        SRT_VARIABLES = 1,
        SRT_SCRIPTSTATE = 2,
        SRT_SCRIPTVARIABLES = 3,
        SRT_STATEVARIABLES = 4,
        SRT_DYNSTRING = 5,
        SRT_ASYNCCALL_STRUCT = 6,
        SRT_ENTERS = 7,
        SRT_MESSAGECUE = 8,
        SRT_EVENTREF = 9,
    };

    struct SaveRefEntry
    {
        void* m_pAddr { nullptr };
        uint32_t m_lSize { 0u };
        SaveRefType m_srt;
        const void* m_pExtra { nullptr };
    };
}