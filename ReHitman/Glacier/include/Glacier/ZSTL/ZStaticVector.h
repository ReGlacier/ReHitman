#pragma once

#include <cstdint>

namespace Glacier
{
    template <typename T, size_t N>
    struct ZStaticVector
    {
        uint32_t m_iSize;
        T m_Data[N];

        T& operator[](size_t i) { return m_Data[i]; }
        [[nodiscard]] bool IsEmpty() const { return m_iSize == 0u; }
    };
}