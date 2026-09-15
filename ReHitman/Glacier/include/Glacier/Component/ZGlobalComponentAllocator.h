#pragma once

#include <cstdint>


namespace Glacier
{
    class ZGlobalComponentAllocator
    {
    public:
        // methods
        static void* Alloc(size_t lSize);
        static void Free(void* ptr);
    };
}