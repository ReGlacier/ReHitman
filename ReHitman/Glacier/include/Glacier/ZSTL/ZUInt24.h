#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZUInt24
    {
        // methods
        ZUInt24(uint32_t iValue)
        {
            Set(iValue);
        }

        void Set(uint32_t iValue)
        {
            m_Data[0] = static_cast<uint8_t>(iValue & 0xFF);
            m_Data[1] = static_cast<uint8_t>((iValue >> 8) & 0xFF);
            m_Data[2] = static_cast<uint8_t>((iValue >> 16) & 0xFF);
        }

        operator uint32_t() const
        {
            return (static_cast<uint32_t>(m_Data[2]) << 16) | (static_cast<uint32_t>(m_Data[1]) << 8) | static_cast<uint32_t>(m_Data[0]);
        }

        // members
        uint8_t m_Data[3];
    };
    RE_VERIFY_SIZE(ZUInt24, 0x3);
}