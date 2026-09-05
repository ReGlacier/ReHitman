#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <algorithm>
#include <cstdint>
#include <new>


namespace Glacier
{
    /**
     * @brief Minimal owning dynamic array used by old Glacier container code.
     *
     * @tparam T Element type stored contiguously in the array.
     *
     * @details ZSimpleArray owns a flat heap allocation and stores only a pointer and a
     * size. It intentionally has no capacity field: Resize() always reallocates when the
     * requested size changes. Existing elements are copy-constructed into the new buffer,
     * new slots are filled from the supplied init value, then the old buffer is destroyed
     * and freed.
     */
    template <typename T>
    struct ZSimpleArray
    {
        // members
        T* m_Array;
        uint32_t m_Size;

        // methods
        ZSimpleArray() : m_Array(nullptr), m_Size(0) {}

        ~ZSimpleArray()
        {
            Destruct();
            ZUniMemory::Free(m_Array);
        }

        T& operator[](uint32_t i)
        {
            ZASSERT(i < m_Size);
            return m_Array[i];
        }

        const T& operator[](uint32_t i) const
        {
            ZASSERT(i < m_Size);
            return m_Array[i];
        }

        void Resize(uint32_t size, const T& init)
        {
            if (size == m_Size)
                return;

            T* pNewArray = nullptr;
            uint32_t copyCount = m_Size < size ? m_Size : size;

            if (size)
            {
                pNewArray = static_cast<T*>(ZUniMemory::Allocate(sizeof(T) * size));
                Copy(m_Array, pNewArray, copyCount);
            }

            Destruct();
            ZUniMemory::Free(m_Array);

            if (m_Size < size)
                Fill(pNewArray + m_Size, size - m_Size, init);

            m_Array = pNewArray;
            m_Size = size;
        }

        void Allocate(uint32_t size)
        {
            m_Array = size ? static_cast<T*>(ZUniMemory::Allocate(sizeof(T) * size)) : nullptr;
            m_Size = size;
        }

        T* Begin() { return m_Array; }
        const T* Begin() const { return m_Array; }
        T* End() { return m_Array + m_Size; }
        const T* End() const { return m_Array + m_Size; }
        T* begin() { return Begin(); }
        const T* begin() const { return Begin(); }
        const T* cbegin() const { return Begin(); }
        T* end() { return End(); }
        const T* end() const { return End(); }
        const T* cend() const { return End(); }
        uint32_t GetSize() const { return m_Size; }

    private:
        static void Fill(T* pDst, uint32_t count, const T& init)
        {
            for (uint32_t i = 0; i < count; ++i)
                new (&pDst[i]) T(init);
        }

        void Destruct()
        {
            for (uint32_t i = 0; i < m_Size; ++i)
                m_Array[i].~T();
        }

        static void Copy(const T* pSrc, T* pDst, uint32_t count)
        {
            for (uint32_t i = 0; i < count; ++i)
                new (&pDst[i]) T(pSrc[i]);
        }
    };
    RE_VERIFY_SIZE(ZSimpleArray<void*>, 0x8);
}
