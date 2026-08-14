#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Globals.h>
#include <cstdint>


namespace Glacier
{
    struct ZHandle
    {
        uint32_t m_lHandleValue { 0u };
    };

    struct ZPrimHandle : public ZHandle
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
    };
}