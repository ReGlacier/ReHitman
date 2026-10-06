#pragma once

#include <cstdint>


namespace Glacier
{
    class ZComponentDefaultAllocator
    {
    public:
        // methods
        static void* Alloc(size_t size);
        static void Free(void*);
    };
}