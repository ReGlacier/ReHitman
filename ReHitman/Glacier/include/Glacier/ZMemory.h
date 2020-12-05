#pragma once

namespace Glacier
{
    /**
     * @struct UseManagedAllocator
     * @brief Wrapper under Glacier's ZMalloc, ZFree functions
     */
    struct UseManagedAllocator
    {
        void* operator new(size_t sz);
        void operator delete(void* ptr) noexcept;
    };
}