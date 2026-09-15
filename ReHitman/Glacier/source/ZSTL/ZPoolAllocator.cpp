#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZPoolAllocator::ZPoolAllocator(char *pBuffer, unsigned int iSize, const char* pName, bool bDebug)
    {
        m_pName = pName;

        ZASSERT(s_iNumPoolAllocators < MAX_ALLOCATORS_NR);
        s_PoolAllocators[s_iNumPoolAllocators++] = this;

        Reset(pBuffer, iSize, bDebug);
    }

    ZPoolAllocator::~ZPoolAllocator()
    {
        for (size_t i = 0; i < s_iNumPoolAllocators; i++)
        {
            if (s_PoolAllocators[i] == this)
            {
                s_PoolAllocators[i] = nullptr;
            }
        }
    }

    void ZPoolAllocator::ReportHighWaterMarks()
    {
        const bool bNeedReport = true; // ZSysInterface::GetOption(g_pSysInterface, "PrintHighWaterMarks", 0);

        if (bNeedReport)
        {
            // This code from PS2 build
            printf("--------------------------------\nPOOL ALLOCATOR HIGH WATER MARKS:\n");
            for (uint8_t i = 0; i < s_iNumPoolAllocators; ++i)
            {
                auto* pPool = s_PoolAllocators[i];
                if (!pPool) continue;

                pPool->ReportHighWaterMark();
            }
            printf("--------------------------------\n");
        }
    }

    void ZPoolAllocator::AddBlock(char* pBlock, unsigned int iSize)
    {
        ZBlockHeader* pEnd = reinterpret_cast<ZBlockHeader*>(&m_pBuffer[m_iSize]);

        ZBlockHeader* pHeader;
        for (pHeader = pEnd - 1; pHeader >= m_pBlockDataBegin; --pHeader)
        {
            if (pHeader->pointer + pHeader->size == pBlock)
            {
                ExpandBlockRight(pHeader, iSize);
                break;
            }

            if (pHeader->pointer == pBlock + iSize)
            {
                ExpandBlockLeft(pHeader, iSize);
                break;
            }
        }

        if (pHeader == m_pBlockDataBegin - 1)
        {
            ZASSERT(m_pBlockDataBegin == pEnd || m_pBlockDataBegin->pointer + m_pBlockDataBegin->size == reinterpret_cast<char*>(m_pBlockDataBegin));
            --m_pBlockDataBegin;

            m_pBlockDataBegin->size = iSize;
            m_pBlockDataBegin->pointer = pBlock;

            SortBlockHeaders();
        }

        // Table may grow-down lol
        char* pBlockEnd = m_pBlockDataBegin->pointer + m_pBlockDataBegin->size;

        if (pBlockEnd > reinterpret_cast<char*>(m_pBlockDataBegin))
            m_pBlockDataBegin->size -= static_cast<unsigned int>(pBlockEnd - reinterpret_cast<char*>(m_pBlockDataBegin));
    }

    void ZPoolAllocator::RemoveBlock(ZBlockHeader* pBlockHeader)
    {
        ZBlockHeader* pNextHeader = GetNextHeader(pBlockHeader);
        ZBlockHeader* pPrevHeader = GetPreviousHeader(pBlockHeader);

        int removedHeaders = 0;

        if (pPrevHeader && pNextHeader && pPrevHeader->pointer + pPrevHeader->size == pNextHeader->pointer)
        {
            pNextHeader->size += pPrevHeader->size + sizeof(ZBlockHeader);

            for (ZBlockHeader* p = pBlockHeader - 2; p >= m_pBlockDataBegin; --p)
            {
                p[2] = *p;
            }

            removedHeaders = 2;
        }
        else
        {
            for (ZBlockHeader* p = pBlockHeader - 1; p >= m_pBlockDataBegin; --p)
            {
                p[1] = *p;
            }

            removedHeaders = 1;
        }

        if (m_pBlockDataBegin->pointer + m_pBlockDataBegin->size == reinterpret_cast<char*>(m_pBlockDataBegin))
        {
            m_pBlockDataBegin[removedHeaders].size += removedHeaders * sizeof(ZBlockHeader);
        }

        m_pBlockDataBegin += removedHeaders;
    }

    void ZPoolAllocator::ResetAll()
    {
        for (uint8_t i = 0; i < s_iNumPoolAllocators; ++i)
        {
            ZPoolAllocator* pPool = s_PoolAllocators[i];
            if (!pPool)
                continue;

            pPool->m_iAllocated = 0;
            pPool->m_iHighWaterMark = 0;

            pPool->m_pBlockDataBegin = reinterpret_cast<ZBlockHeader*>(&pPool->m_pBuffer[pPool->m_iSize]);
            pPool->AddBlock(pPool->m_pBuffer, pPool->m_iSize);
        }
    }

    void ZPoolAllocator::CheckAllPools()
    {
        for (uint8_t i = 0; i < s_iNumPoolAllocators; ++i)
        {
            if (auto* pPool = s_PoolAllocators[i])
            {
                pPool->CheckPool();
            }
        }
    }

    int ZPoolAllocator::BlockHeaderSort(const ZBlockHeader* a, const ZBlockHeader* b)
    {
        return b->pointer - a->pointer;
    }

    void* ZPoolAllocator::Alloc(uint32_t iSize)
    {
        iSize = (iSize + 3) & ~3u;
        const unsigned int allocSize = iSize + sizeof(ZAllocHeader);
        ZBlockHeader* pBlock = reinterpret_cast<ZBlockHeader*>(&m_pBuffer[m_iSize - sizeof(ZBlockHeader)]);

        while (pBlock >= m_pBlockDataBegin)
        {
            if (pBlock->size >= allocSize)
            {
                break;
            }

            --pBlock;
        }

        if (pBlock < m_pBlockDataBegin)
            return nullptr;

        m_iAllocated += allocSize;
        if (m_iAllocated > m_iHighWaterMark)
            m_iHighWaterMark = m_iAllocated;

        auto* pHeader = reinterpret_cast<ZAllocHeader*>(pBlock->pointer);
        pHeader->size = iSize;

        void* pMemory = pHeader + 1;
        ZASSERT(pBlock->size >= allocSize);

        if (pBlock->size == allocSize && pBlock != reinterpret_cast<ZBlockHeader*>(&m_pBuffer[m_iSize - sizeof(ZBlockHeader)]))
        {
            RemoveBlock(pBlock);
        }
        else
        {
            pBlock->pointer += allocSize;
            pBlock->size -= allocSize;
        }

        return pMemory;
    }

    void ZPoolAllocator::Free(void* p)
    {
        if (!p) return;

        ZASSERT(p > m_pBuffer);
        ZASSERT(reinterpret_cast<ZBlockHeader*>(p) < m_pBlockDataBegin);

        auto* pHeader = reinterpret_cast<ZAllocHeader*>(p) - 1;
        m_iAllocated -= pHeader->size + sizeof(ZAllocHeader);

        AddBlock(reinterpret_cast<char*>(pHeader), pHeader->size + sizeof(ZAllocHeader));
    }

    void ZPoolAllocator::CheckPool()
    {
        // Debug code from PS2
        const float fUsage = 100.0f * static_cast<float>(m_iAllocated) / static_cast<float>(m_iSize);
        
        if (fUsage > 70.0f)
        {
            printf(
                "PoolAllocator warning: %s is using %.1f%% of its %u byte buffer. "
                "Consider increasing the pool size.\n",
                m_pName,
                fUsage,
                m_iSize);
        }
    }

    void ZPoolAllocator::ReportHighWaterMark()
    {
        // Debug code from PS2
        const float fPercentage = (m_iSize != 0) ? (100.0f * static_cast<float>(m_iHighWaterMark) / static_cast<float>(m_iSize)) : 0.0f;
        printf("\t%s used max %u of %u bytes (%.1f%%)\n", m_pName, m_iHighWaterMark, m_iSize, fPercentage);
    }

    void ZPoolAllocator::ExpandBlockRight(ZBlockHeader* pBlockHeader, unsigned int iSize)
    {
        ZBlockHeader* pNextHeader =
            (pBlockHeader > m_pBlockDataBegin) ? pBlockHeader - 1 : nullptr;

        pBlockHeader->size += iSize;

        ZASSERT(pNextHeader == nullptr || pBlockHeader->pointer + pBlockHeader->size <= pNextHeader->pointer);

        if (pNextHeader && pNextHeader->pointer == pBlockHeader->pointer + pBlockHeader->size)
        {
            pBlockHeader->size += pNextHeader->size;
            RemoveBlock(pNextHeader);
        }
    }

    void ZPoolAllocator::Reset(char *pBuffer, unsigned int iSize, bool bDebug)
    {
        m_iAllocated = 0;
        m_iHighWaterMark = 0;

        if (pBuffer)
            m_pBuffer = pBuffer;

        if (iSize)
            m_iSize = iSize;

        m_pBlockDataBegin = reinterpret_cast<ZBlockHeader*>(&m_pBuffer[m_iSize]);

        if (m_pBuffer && m_iSize)
        {
            AddBlock(m_pBuffer, m_iSize);
        }

        // In PS2 this value stored inside class
        std::ignore = bDebug;
    }

    void ZPoolAllocator::ExpandBlockLeft(ZBlockHeader* pBlockHeader, unsigned int iSize)
    {
        ZBlockHeader* pPrevHeader = (pBlockHeader < reinterpret_cast<ZBlockHeader*>(&m_pBuffer[m_iSize - sizeof(ZBlockHeader)])) ? pBlockHeader + 1 : nullptr;
        ZASSERT(pPrevHeader == nullptr || pPrevHeader->pointer + pPrevHeader->size <= pBlockHeader->pointer);

        pBlockHeader->size += iSize;
        pBlockHeader->pointer -= iSize;

        if (pPrevHeader && pPrevHeader->pointer + pPrevHeader->size == pBlockHeader->pointer)
        {
            pBlockHeader->size += pPrevHeader->size;
            pBlockHeader->pointer -= pPrevHeader->size;

            RemoveBlock(pPrevHeader);
        }
    }

    void ZPoolAllocator::SortBlockHeaders()
    {
        const auto* pEnd = reinterpret_cast<const ZBlockHeader*>(m_pBuffer + m_iSize);
        const size_t numHeaders = pEnd - m_pBlockDataBegin;

        qsort(
            m_pBlockDataBegin, 
            numHeaders, 
            sizeof(ZBlockHeader), 
            [](const void* a, const void* b) -> int 
            { 
                return BlockHeaderSort((ZBlockHeader*)a, (ZBlockHeader*)b); 
            });
    }

    ZPoolAllocator::ZBlockHeader* ZPoolAllocator::GetPreviousHeader(ZBlockHeader* pBlockHeader)
    {
        auto* pLastHeader = reinterpret_cast<ZBlockHeader*>(&m_pBuffer[m_iSize - sizeof(ZBlockHeader)]);
        return (pBlockHeader < pLastHeader) ? pBlockHeader + 1 : nullptr;
    }

    ZPoolAllocator::ZBlockHeader* ZPoolAllocator::GetNextHeader(ZBlockHeader* pBlockHeader)
    {
        return (pBlockHeader > m_pBlockDataBegin) ? pBlockHeader - 1 : nullptr;
    }

    // Globals
    STATIC_CLASS_VAR_IMPL(ZPoolAllocator, uint8_t, s_iNumPoolAllocators, 0x008ACB08, 0);
    STATIC_CLASS_VAR_ARRAY_IMPL(ZPoolAllocator, ZPoolAllocator*, s_PoolAllocators, ZPoolAllocator::MAX_ALLOCATORS_NR, 0x008ACB10);
}