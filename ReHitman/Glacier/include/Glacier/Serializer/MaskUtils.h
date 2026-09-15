#pragma once

#include <cstdint>


namespace Glacier
{
    template <typename T>
    constexpr uint32_t GetEndiannessMask()
    {
        if constexpr (sizeof(T) == 8) return 7u;
        if constexpr (sizeof(T) == 4) return 3u;
        if constexpr (sizeof(T) == 2) return 1u;
        return 0u;
    }
}
