#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    // ZRIndexContainer

    ZRIndexContainer::ZRIndexContainer() = default;

    void ZRIndexContainer::Create(const uint16_t* pPackedData, ZRX86AllocIf* pAllocator, uint32_t lFlags)
    {
        ZASSERT(!m_pSubRanges);

        m_pAllocator = pAllocator;
        m_lFlags = lFlags;

        uint16_t numSubRanges = *pPackedData;
        const uint16_t* pStream = pPackedData + 1;
        this->m_lSubRangeCount = numSubRanges;

        if (numSubRanges == 1)
        {
            m_pSubRanges = &m_InlineSubRange;
        }
        else
        {
            m_pSubRanges = (ZRIndexSubRange*)ZUniMemory::Allocate(sizeof(ZRIndexSubRange) * numSubRanges);
        }

        m_lTotalIndexCount = 0;

        for (uint32_t i = 0; i < m_lSubRangeCount; ++i)
        {
            uint16_t count = *pStream;
            
            m_pSubRanges[i].m_lIndexCount = count;
            m_pSubRanges[i].m_lIndexOffset = 0;

            m_lTotalIndexCount += count;
            pStream += (count + 1);
        }

        if (m_lTotalIndexCount == 0)
        {
            if (m_pSubRanges != &m_InlineSubRange)
            {
                ZUniMemory::Free(m_pSubRanges);
            }
                
            m_pSubRanges = nullptr;
            return;
        }

        uint32_t baseOffset = (uint32_t)pAllocator->m_Allocator.Alloc(m_lTotalIndexCount, true);

        if (baseOffset == (uint32_t)-1)
        {
            if (m_pSubRanges != &m_InlineSubRange)
            {
                ZUniMemory::Free(m_pSubRanges);
            }

            m_pSubRanges = nullptr;
            return;
        }

        uint32_t currentOffset = baseOffset;
        for (uint32_t i = 0; i < m_lSubRangeCount; ++i)
        {
            m_pSubRanges[i].m_lIndexOffset = currentOffset;
            currentOffset += m_pSubRanges[i].m_lIndexCount;
        }
    }

    void ZRIndexContainer::Release()
    {
        if (m_pSubRanges != nullptr)
        {
            uint32_t firstOffset = m_pSubRanges->m_lIndexOffset;

            if (firstOffset != 0)
            {
                m_pAllocator->m_Allocator.Free(
                    firstOffset & 0x7FFFFFFF, 
                    m_lTotalIndexCount
                );
            }

            if (m_pSubRanges != &m_InlineSubRange)
            {
                ZUniMemory::Free(m_pSubRanges);
            }

            m_pSubRanges = nullptr;
        }
    }
}