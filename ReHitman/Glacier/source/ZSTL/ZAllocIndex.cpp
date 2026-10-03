#include <Glacier/ZSTL/ZAllocIndex.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZAllocIndex::ZAllocIndex(uint32_t lNrIndexBits)
    {
        m_lNrElements = 1 << lNrIndexBits;
        m_pArray = (uint32_t*)ZUniMemory::Allocate(sizeof(uint32_t) * m_lNrElements);

        Clean();
    }

    ZAllocIndex::~ZAllocIndex()
    {
        if (m_pArray)
        {
            ZUniMemory::Free(m_pArray);
            m_pArray = nullptr;
        }
    }

    void ZAllocIndex::Clean()
    {
        m_lFirstFreeIndex = 0;
        m_lActiveCount = 0;

        int i = 0;
        for (; i != (m_lNrElements - 1); ++i)
        {
            m_pArray[i] = i + 1;
        }

        m_pArray[i] = i;
    }

    uint32_t ZAllocIndex::New()
    {
        ZASSERT(m_lActiveCount != m_lNrElements);

        const uint32_t lIndexMask = m_lNrElements - 1;
        const uint32_t lGenMask = ~lIndexMask;
        uint32_t lOldFreeIndex = m_lFirstFreeIndex;

        // Extract next free elem
        m_lFirstFreeIndex = m_pArray[lOldFreeIndex] & lIndexMask;
        if (m_lFirstFreeIndex == lOldFreeIndex)
        {
            m_lFirstFreeIndex = 0xFFFFFFFF; // -1
        }

        // Increase generation
        m_pArray[lOldFreeIndex] = ((m_pArray[lOldFreeIndex] + m_lNrElements) & lGenMask) | lOldFreeIndex;

        // Check gen for overflow
        if ((m_pArray[lOldFreeIndex] & lGenMask) != 0)
        {
            ++m_lActiveCount;
            return m_pArray[lOldFreeIndex] & lIndexMask;
        }

        // Overflow - generate again
        return New();
    }

    void ZAllocIndex::Free(uint32_t lIndex)
    {
        ZASSERT(m_lActiveCount);

        const uint32_t lSlot = lIndex & (m_lNrElements - 1);
        m_pArray[lSlot] &= -m_lNrElements;
        if (m_lFirstFreeIndex == -1)
        {
            m_lFirstFreeIndex = lSlot;
        }

        m_pArray[lSlot] |= m_lFirstFreeIndex;
        m_lFirstFreeIndex = lSlot;
        --m_lActiveCount;
    }

    uint32_t ZAllocIndex::Count() const
    {
        return m_lActiveCount;
    }
}