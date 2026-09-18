#pragma once

#include <cstdint>


namespace Glacier
{
    /** Low-level byte source used by buffered serializer input streams. */
    struct IInputStream
    {
        virtual ~IInputStream() = default;

        /** Reads up to size bytes into the destination buffer. */
        virtual uint32_t Read(void* address, const uint32_t size) = 0;
    };
}
