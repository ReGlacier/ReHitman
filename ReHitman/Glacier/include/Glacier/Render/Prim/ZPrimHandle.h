#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Globals.h>
#include <cstdint>


namespace Glacier
{
    struct ZPrimHandleBase
    {
        uint32_t m_lHandleValue { 0u };
    };

    struct ZPrimHandle : public ZPrimHandleBase
    {
        template <typename T>
        T const* Get() const
        {
            if (m_lHandleValue & 0x80000000u)
            {
                // It's raw pointer
                return reinterpret_cast<T const*>(static_cast<uintptr_t>(m_lHandleValue & 0x7FFFFFFF));
            }
            else
            {
                // It's index
                return static_cast<T const*>(g_apPrimHandleToPointerTable[m_lHandleValue]);
            }
        }

        template <typename T>
        operator T const*() const
        {
            return Get<T>();
        }

        // Not BM/Glacier related stuff, just for runtime assertions
        operator bool() const
        {
            return m_lHandleValue != 0u;
        }

        // Not BM/Glacier related stuff, just for easier code
        operator uint32_t() const
        {
            return m_lHandleValue;
        }
    };
}