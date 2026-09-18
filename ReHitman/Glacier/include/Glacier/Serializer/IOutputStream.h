#pragma once

#include <cstdint>


namespace Glacier
{
    struct IOutputStream
    {
        virtual ~IOutputStream() = default;
        virtual uint32_t Write(const void* pAddr, const uint32_t lSize) = 0;
    };
}
