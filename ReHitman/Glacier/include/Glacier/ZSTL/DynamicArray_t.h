#pragma once

#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <cstdint>


namespace Glacier
{
    template <typename T>
    struct DynamicArray_t
    {
        // methods
        DynamicArray_t() = default;
        
        ~DynamicArray_t()
        {
            cleanup();
        }

        void cleanup()
        {
            if (array)
            {
                ZUniMemory::Free(array);
                array = nullptr;
                max = cur = 0u;
            }
        }

        void init(uint32_t capacity)
        {
            cleanup();

            if (capacity > 0)
            {
                array = static_cast<T*>(ZUniMemory::Allocate(sizeof(T) * capacity));
                ZASSERT(array);
                max = capacity;
            }
            cur = 0;
        }

        void expandArray()
        {
            // Фикс для max == 0
            const uint32_t lNewCapacity = (max == 0) ? 4u : (max * 2u);
            
            auto* pNewBlk = static_cast<T*>(ZUniMemory::Allocate(sizeof(T) * lNewCapacity));
            ZASSERT(pNewBlk);

            if (array)
            {
                memcpy(pNewBlk, array, sizeof(T) * max);
                ZUniMemory::Free(array);
            }

            array = pNewBlk;
            max = lNewCapacity;
        }

        T* getNextElm()
        {
            if (cur >= max)
            {
                expandArray();
            }
            
            return &array[cur++];
        }

        // members
        uint32_t max{0u};
        uint32_t cur{0u};
        T* array{nullptr};
    };
}