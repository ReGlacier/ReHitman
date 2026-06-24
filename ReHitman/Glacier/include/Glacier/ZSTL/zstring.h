#pragma once

#include <cstdint>

namespace Glacier
{
    class zstring
    {
    public:
        char* m_pData;
        uint32_t m_iLength;
        uint32_t m_iCapacity;
    };
}