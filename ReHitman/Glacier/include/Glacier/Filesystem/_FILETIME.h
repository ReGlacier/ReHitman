#pragma once

#include <cstdint>


namespace Glacier
{
    struct _FILETIME
    {
        // methods
        _FILETIME() = default;
        
        explicit _FILETIME(uint64_t value) 
            : dwLowDateTime(static_cast<uint32_t>(value & 0xFFFFFFFF))
            , dwHighDateTime(static_cast<uint32_t>(value >> 32)) 
        {}

        _FILETIME(uint32_t low, uint32_t high)
            : dwHighDateTime(high)
            , dwLowDateTime(low)
        {}

        operator uint64_t() const 
        {
            return (static_cast<uint64_t>(dwHighDateTime) << 32) | static_cast<uint64_t>(dwLowDateTime);
        }

        // members
        uint32_t dwHighDateTime{0u};
        uint32_t dwLowDateTime{0u};
    };
}