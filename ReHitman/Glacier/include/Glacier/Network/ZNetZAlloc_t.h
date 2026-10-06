#pragma once

#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    class ZNetZAlloc_t
    {
    public:
        void NetZFree(void* ptr)
        {
            ZUniMemory::Free(ptr);
        }

        void* NetZFree(int size)
        {
            return ZUniMemory::Allocate(size);
        }
    };
}