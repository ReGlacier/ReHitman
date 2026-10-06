#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZOffsetAlloc::ZOffsetAlloc(uint32_t lStartOffset, uint32_t lEndOffset, uint32_t lMaxNrAllocs, ZOffsetAlloc::ZLink *pBuffer, EAllocType eType)
    {
        m_bUserSuppliedLinkBuffer = pBuffer != nullptr;
        m_lMaxNrLinks = lMaxNrAllocs;
        m_lNrLinks = 0;
        
        if (m_bUserSuppliedLinkBuffer)
        {
            m_pLinks = pBuffer;
        }
        else
        {
            EAllocType eMemType = (eType == EAllocType::RENDERCPU_MEM) ? EAllocType::SLOW_MEM : EAllocType::DEFAULT_MEM;
            m_pLinks = (ZOffsetAlloc::ZLink*)ZUniMemory::Allocate(sizeof(ZOffsetAlloc::ZLink) * lMaxNrAllocs, eMemType);
        }

        Reset(lStartOffset, lEndOffset);
    }

    ZOffsetAlloc::~ZOffsetAlloc()
    {
        if (!m_bUserSuppliedLinkBuffer)
        {
            ZUniMemory::Free(m_pLinks);
            m_pLinks = nullptr;
        }
    }
    
    int ZOffsetAlloc::GetFreeTotal() const
    {
        int lTotalFree = 0;

        for (int i = 0; i < m_lNrLinks; ++i)
        {
            lTotalFree += m_pLinks[i].m_lSize;
        }

        return lTotalFree;
    }

    void ZOffsetAlloc::GetFreeList(REFTAB* pFreeList) const
    {
        ZASSERT(pFreeList != nullptr);

        for (int i = 0; i < m_lNrLinks; ++i)
        {
            const auto& link = m_pLinks[i];

            if (pFreeList->GetEleSize() == 1)
            {
                pFreeList->Add(link.m_lOffset);
                pFreeList->Add(link.m_lSize);
            }
            else
            {
                uint32_t* pData = pFreeList->Add(link.m_lSize);
                if (pData)
                {
                    *pData = link.m_lOffset;
                }
            }
        }
    }

    void ZOffsetAlloc::Reset(uint32_t lStartOffset, uint32_t lEndOffset)
    {
        m_lNrLinks = 0;

        ZASSERT(!(lStartOffset & 0x80000000));
        ZASSERT(!(lEndOffset & 0x80000000));
        ZASSERT(lEndOffset > lStartOffset);

        AddFreeSpace(lStartOffset, lEndOffset - lStartOffset);
    }
    
    uint32_t ZOffsetAlloc::GetNrFreeLinks() const
    {
        return m_lNrLinks;
    }

    uint32_t ZOffsetAlloc::GetLargestFreeBlock() const
    {
        uint32_t lLargestSize = 0;

        for (int i = 0; i < m_lNrLinks; ++i)
        {
            if (lLargestSize < m_pLinks[i].m_lSize)
            {
                lLargestSize = m_pLinks[i].m_lSize;
            }
        }

        return lLargestSize;
    }

    void* ZOffsetAlloc::Alloc(uint32_t lSize, bool bLargeUseHigh)
    {
        ZOffsetAlloc::ZLink* pBestFitLnk = nullptr;

        for (int i = 0; i < m_lNrLinks; ++i)
        {
            ZLink* pCurrentLnk = &m_pLinks[i];

            if (pCurrentLnk->m_lSize >= lSize)
            {
                if (pCurrentLnk->m_lSize == lSize)
                {
                    uint32_t lOffset = pCurrentLnk->m_lOffset;
                    CopyLastToCurrent(pCurrentLnk);
                    return reinterpret_cast<void*>(static_cast<uintptr_t>(lOffset));
                }

                if (!pBestFitLnk || pCurrentLnk->m_lSize < pBestFitLnk->m_lSize)
                {
                    pBestFitLnk = pCurrentLnk;
                }
            }
        }

        if (pBestFitLnk)
        {
            uint32_t lAllocatedOffset = pBestFitLnk->m_lOffset;
            
            pBestFitLnk->m_lOffset += lSize;
            pBestFitLnk->m_lSize -= lSize;
            
            return reinterpret_cast<void*>(static_cast<uintptr_t>(lAllocatedOffset));
        }

        // DronCode: Yep yep yep, C++ expect to have 0x0 (nullptr, NULL, nil) but IOI think different
        return reinterpret_cast<void*>(-1);
    }

    void* ZOffsetAlloc::AllocHi(uint32_t lSize)
    {
        ZLink* pHighAddressLnk = nullptr;

        for (int i = 0; i < m_lNrLinks; ++i)
        {
            ZLink* pCurrentLnk = &m_pLinks[i];

            if (pCurrentLnk->m_lSize >= lSize)
            {
                if (pCurrentLnk->m_lSize == lSize)
                {
                    uint32_t lOffset = pCurrentLnk->m_lOffset;
                    CopyLastToCurrent(pCurrentLnk);
                    return reinterpret_cast<void*>(static_cast<uintptr_t>(lOffset));
                }

                if (!pHighAddressLnk || pHighAddressLnk->m_lOffset < pCurrentLnk->m_lOffset)
                {
                    pHighAddressLnk = pCurrentLnk;
                }
            }
        }

        if (pHighAddressLnk)
        {
            pHighAddressLnk->m_lSize -= lSize;
            uint32_t lAllocatedOffset = pHighAddressLnk->m_lOffset + pHighAddressLnk->m_lSize;
            
            return reinterpret_cast<void*>(static_cast<uintptr_t>(lAllocatedOffset));
        }

        // DronCode: Yep yep yep, C++ expect to have 0x0 (nullptr, NULL, nil) but IOI think different
        return reinterpret_cast<void*>(-1);
    }

    void ZOffsetAlloc::Free(uint32_t lOffset, uint32_t lSize)
    {
        uint32_t lFreeEnd = lOffset + lSize;

        for (int i = 0; ; ++i)
        {
            // Case 1: No candidate been found for merge. Need to add this block as separated
            if (i == m_lNrLinks)
            {
                AddFreeSpace(lOffset, lSize);
                return;
            }

            ZLink* pCurrentLnk = &m_pLinks[i];

            // Case 2: Our block located BEFORE freed (first merge)
            if (pCurrentLnk->m_lOffset == lFreeEnd)
            {
                pCurrentLnk->m_lOffset -= lSize;
                pCurrentLnk->m_lSize += lSize;

                for (int j = i + 1; j < m_lNrLinks; ++j)
                {
                    ZLink* pOtherLnk = &m_pLinks[j];
                    
                    if (pOtherLnk->m_lOffset + pOtherLnk->m_lSize == lOffset)
                    {
                        // Merge with left block
                        pCurrentLnk->m_lOffset -= pOtherLnk->m_lSize;
                        pCurrentLnk->m_lSize += pOtherLnk->m_lSize;

                        CopyLastToCurrent(pOtherLnk);
                        return;
                    }
                }
                return;
            }

            // Case 3: Our block located AFTER freed (left merge)
            if (pCurrentLnk->m_lOffset + pCurrentLnk->m_lSize == lOffset)
            {
                pCurrentLnk->m_lSize += lSize;

                for (int k = i + 1; k < m_lNrLinks; ++k)
                {
                    ZLink* pOtherLnk = &m_pLinks[k];

                    if (pOtherLnk->m_lOffset == lFreeEnd)
                    {
                        // Merge with right
                        pCurrentLnk->m_lSize += pOtherLnk->m_lSize;

                        CopyLastToCurrent(pOtherLnk);
                        return;
                    }
                }
                return;
            }
        }
    }

    void ZOffsetAlloc::CopyLastToCurrent(ZOffsetAlloc::ZLink* pLnk)
    {
        m_lNrLinks--;

        const ZLink& lastLink = m_pLinks[m_lNrLinks];
        pLnk->m_lOffset = lastLink.m_lOffset;
        pLnk->m_lSize = lastLink.m_lSize;
    }

    void ZOffsetAlloc::AddFreeSpace(uint32_t lStartOffset, uint32_t lSize)
    {
        ZASSERT(m_lNrLinks != m_lMaxNrLinks);

        m_pLinks[m_lNrLinks].m_lOffset = lStartOffset;
        m_pLinks[m_lNrLinks].m_lSize = lSize;

        m_lNrLinks++;
    }
}
