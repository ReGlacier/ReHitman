#pragma once

#include <cstdint>

namespace Glacier
{
    template <typename T>
    struct ZSimpleArray
    {
        char* m_Array;
        uint32_t m_Size;
    };
}