#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Fixed-size element pool (reversed from common/basic/fixedalloc.h).
     *
     * The elements are stored inline followed by a free-list of 1-based pool
     * indices. Element()/Index() are 1-based; Alloc()/Free() are O(1).
     */
    template <typename T, uint32_t N>
    struct ZTFixedAlloc
    {
        T m_Elements[N];
        uint32_t m_iFreeElementsOffset[N];
        uint32_t m_lFreeElementCount;

        ZTFixedAlloc()
        {
            for (uint32_t i = 0; i < N; ++i)
                m_iFreeElementsOffset[i] = i;
            m_lFreeElementCount = N;
        }

        // 1-based accessor; index 0 and out-of-range values return nullptr.
        T* Element(uint32_t lIndex)
        {
            if (lIndex == 0 || lIndex > N)
                return nullptr;
            return &m_Elements[lIndex - 1];
        }

        const T* Element(uint32_t lIndex) const
        {
            if (lIndex == 0 || lIndex > N)
                return nullptr;
            return &m_Elements[lIndex - 1];
        }

        // 1-based index of an element that belongs to this pool.
        uint32_t Index(const T* pElement) const
        {
            ZASSERT(pElement >= m_Elements && pElement < m_Elements + N);
            return static_cast<uint32_t>(pElement - m_Elements) + 1;
        }

        uint32_t FreeCount() const
        {
            return m_lFreeElementCount;
        }

        T* Alloc()
        {
            ZASSERT(m_lFreeElementCount != 0);
            if (m_lFreeElementCount == 0)
                return nullptr;
            const uint32_t lSlot = --m_lFreeElementCount;
            return &m_Elements[m_iFreeElementsOffset[lSlot]];
        }

        void Free(T* pElement)
        {
            ZASSERT(m_lFreeElementCount < N);
            m_iFreeElementsOffset[m_lFreeElementCount++] = Index(pElement) - 1;
        }
    };
}
