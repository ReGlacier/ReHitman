#pragma once

#include <Glacier/ZSTL/ZBlockArrayBase.h>
#include <Glacier/ZUniAssert.h>
#include <utility>
#include <new>

namespace Glacier
{
    template <typename T>
    struct ZBlockArrayBase2 : public ZBlockArrayBase
    {
        ZBlockArrayBase2(uint32_t block_size_exhibitor) : ZBlockArrayBase(sizeof(T), block_size_exhibitor)
        {
        }

        ~ZBlockArrayBase2() override
        {
            Cleanup();
        }

        T& operator[](uint32_t idx)
        {
            return *reinterpret_cast<T*>(GetItem(idx));
        }

        const T& operator[](uint32_t idx) const
        {
            return *reinterpret_cast<const T*>(const_cast<ZBlockArrayBase2*>(this)->GetItem(idx));
        }

        void Construct(void* pMemory) override
        {
            ::new (pMemory) T();
        }

        void Destruct(void* pMemory) override
        {
            if constexpr (std::is_destructible_v<T>) 
            {
                reinterpret_cast<T*>(pMemory)->~T();
            }
        }

        void Copy(void* pMemory, const void* pSource) override
        {
            ::new (pMemory) T(*reinterpret_cast<const T*>(pSource));
        }
    };

    template <typename T, uint32_t BlockSizeShift = 5>
    struct ZBlockArray : public ZBlockArrayBase2<T>
    {
        ZBlockArray() : ZBlockArrayBase2<T>(BlockSizeShift)
        {
        }
    };

    template <typename T>
    struct ZArray : public ZBlockArray<T, 5>
    {
        ZArray() : ZBlockArray<T, 5>()
        {
        }

        uint32_t Count() const
        {
            return this->GetSize();
        }

        T& Add(const T& item)
        {
            uint32_t current_size = this->GetSize();
            T& new_item = this->operator[](current_size);
            
            new_item = item;
            return new_item;
        }

        void RemoveByIdx(uint32_t item_idx)
        {
            uint32_t current_size = this->GetSize();
            ZASSERT(item_idx < current_size);

            uint32_t last_valid_idx = current_size - 1;

            for (uint32_t i = item_idx; i < last_valid_idx; ++i)
            {
                this->operator[](i) = this->operator[](i + 1);
            }

            this->Resize(last_valid_idx);
        }
    };
    RE_VERIFY_SIZE(ZArray<int>, 0x1C);
}
