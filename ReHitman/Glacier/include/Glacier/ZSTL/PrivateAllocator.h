#pragma once

#include <Glacier/ZSTL/ISaveMemoryManager.h>
#include <Glacier/ZSTL/STLport.h>

#include <cstddef>


namespace Glacier
{
    template <typename T, typename MemoryManager>
    class PrivateAllocator : public stlp::allocator<T>
    {
    public:
        using Base = stlp::allocator<T>;
        using value_type = typename Base::value_type;
        using pointer = typename Base::pointer;
        using const_pointer = typename Base::const_pointer;
        using reference = typename Base::reference;
        using const_reference = typename Base::const_reference;
        using size_type = typename Base::size_type;
        using difference_type = typename Base::difference_type;

        template <typename U>
        struct rebind
        {
            using other = PrivateAllocator<U, MemoryManager>;
        };

        PrivateAllocator() = default;
        PrivateAllocator(const PrivateAllocator&) = default;

        template <typename U>
        PrivateAllocator(const PrivateAllocator<U, MemoryManager>&)
        {
        }

        ~PrivateAllocator() = default;

        PrivateAllocator& operator=(const PrivateAllocator&) = default;

        pointer allocate(size_type count, const void* = nullptr)
        {
            return count ? static_cast<pointer>(MemoryManager::Alloc(static_cast<int>(count * sizeof(value_type)))) : nullptr;
        }

        void deallocate(pointer ptr, size_type)
        {
            if (ptr)
                MemoryManager::Free(ptr);
        }
    };
}
