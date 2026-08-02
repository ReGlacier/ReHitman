#include <Glacier/ZSTL/ZMallocSimple.h>
#include <Glacier/ZUniAssert.h>


// ---------------------------------------------------------------------------
// ZMallocSimple allocation memory layout
//
// Every block produced by Alloc() consists of an m_lAlignment-byte size
// header followed by the payload. The pointer returned to the caller (and
// later passed back to Free()/AllocSize()) always addresses the payload,
// i.e. block + m_lAlignment:
//
//   block start (pBlockStart)        public pointer (pRam)
//   |                                |
//   v                                v
//   +------------------+--------------------------------------------+
//   | uint32_t header  | payload (capacity = header - m_lAlignment) |
//   +------------------+--------------------------------------------+
//   |<------------------------- lAlignedSize ---------------------->|
//
// The header stores lAlignedSize = align4(lRequestedSize + m_lAlignment),
// that is the size of the WHOLE block, header included. This yields the
// invariants (m_lAlignment == 4 for all known placements):
//   - block start       = pRam - m_lAlignment       (see Free)
//   - payload capacity  = header - m_lAlignment     (see AllocSize)
//
// The pBlockStart/pBlockEnd out-params of Alloc() bracket the whole block,
// header included: [pBlockStart, pBlockEnd) = [block, block + lAlignedSize).
// ---------------------------------------------------------------------------

namespace Glacier
{
    SFreeLink::SFreeLink() = default;
    
    SFreeLink::SFreeLink(char* pFreeRam, uint32_t lFreeSize)
        : m_pFreeRam(pFreeRam)
        , m_lFreeSize(lFreeSize)
    {
    }

    ZMallocSimple::ZMallocSimple(uint32_t lMaxNrFreeLinks, uint32_t lAlignment)
        : m_lNrFreeLinks(0u)
        , m_lTotalSize(0u)
        , m_lAlignment(lAlignment)
        , m_lMaxNrFreeLinks(lMaxNrFreeLinks)
    {
    }

    ZMallocSimple::~ZMallocSimple() = default;

    void ZMallocSimple::AddBlock(char* pRamPtr, uint32_t lFreeSize)
    {
        m_lTotalSize += lFreeSize;

        auto* pFreeLink = AllocFreeLink();
        ZASSERT(pFreeLink);

        pFreeLink->m_pFreeRam = pRamPtr;
        pFreeLink->m_lFreeSize = lFreeSize;
    }

    void ZMallocSimple::RemoveBlock(char* pRamPtr)
    {
        // Counterpart of AddBlock(): drop the free link that starts at
        // pRamPtr. Only valid while the whole block is still free, i.e.
        // before anything was allocated from it.
        for (uint32_t i = 0u; i < m_lNrFreeLinks; ++i)
        {
            if (m_pFreeLinks[i].m_pFreeRam == pRamPtr)
            {
                m_lTotalSize -= m_pFreeLinks[i].m_lFreeSize;
                FreeFreeLink(&m_pFreeLinks[i]);
                return;
            }
        }
    }

    bool ZMallocSimple::CheckAllFreed()
    {
        // Everything allocated so far has been freed iff the free links hold
        // the whole pool again (AddBlock() accumulates the pool size into
        // m_lTotalSize, GetFreeTotal() sums the free links).
        return GetFreeTotal() == m_lTotalSize;
    }

    uint32_t ZMallocSimple::GetFreeTotal()
    {
        uint32_t lFreeTotal = 0u;

        for (int i = 0; i != m_lNrFreeLinks; ++i)
        {
            lFreeTotal += m_pFreeLinks[i].m_lFreeSize;
        }

        return lFreeTotal;
    }
    
    char* ZMallocSimple::Alloc(uint32_t lSize, char*& pBlockStart, char*& pBlockEnd)
    {
        if (!lSize)
        {
            return nullptr;
        }

        // +m_lAlignment for the size header, then align the whole block up
        // to 4 bytes (see layout note at the top of this file).
        uint32_t lAlignedSize = (lSize + (m_lAlignment + 3u)) & 0xFFFFFFFCu;

        SFreeLink* pFoundLink = nullptr;
        int32_t lBestFreeDiff = -1;

        SFreeLink* pFreeLink = m_pFreeLinks;
        for (uint32_t i = 0u; i < m_lNrFreeLinks; ++i, ++pFreeLink)
        {
            if (pFreeLink->m_lFreeSize >= lAlignedSize)
            {
                const uint32_t lFreeDiff = pFreeLink->m_lFreeSize - lAlignedSize;
                if (lFreeDiff < static_cast<uint32_t>(lBestFreeDiff))
                {
                    pFoundLink = pFreeLink;
                    lBestFreeDiff = static_cast<int32_t>(lFreeDiff);

                    if (lFreeDiff < 8u)
                    {
                        break;
                    }
                }
            }
        }

        if (!pFoundLink)
        {
            return nullptr;
        }

        if (static_cast<uint32_t>(lBestFreeDiff) < 8u)
        {
            lAlignedSize += static_cast<uint32_t>(lBestFreeDiff);
        }

        // Store the whole-block size in the 4-byte header at the block start;
        // the public pointer returned below skips past it.
        char* pBlock = pFoundLink->m_pFreeRam;
        *reinterpret_cast<uint32_t*>(pFoundLink->m_pFreeRam) = lAlignedSize;
        pFoundLink->m_pFreeRam += lAlignedSize;
        pFoundLink->m_lFreeSize -= lAlignedSize;

        if (!pFoundLink->m_lFreeSize)
        {
            FreeFreeLink(pFoundLink);
        }

        pBlockStart = pBlock;
        pBlockEnd = pBlock + lAlignedSize;
        return pBlock + m_lAlignment;
    }
    
    void ZMallocSimple::Free(char* pRam)
    {
        if (!pRam)
        {
            return;
        }

        // Step back over the m_lAlignment-byte header to the block start and
        // read the whole-block size stored by Alloc() (see layout note).
        char* pBlock = pRam - m_lAlignment;
        uint32_t lBlockSize = *reinterpret_cast<uint32_t*>(pBlock);

        SFreeLink* pMergedLink = nullptr;

        SFreeLink* pFreeLink = m_pFreeLinks;
        for (uint32_t i = 0u; i < m_lNrFreeLinks; ++i, ++pFreeLink)
        {
            if (pBlock + lBlockSize == pFreeLink->m_pFreeRam)
            {
                lBlockSize += pFreeLink->m_lFreeSize;
                pFreeLink->m_pFreeRam = pBlock;
                pFreeLink->m_lFreeSize = lBlockSize;

                if (pMergedLink)
                {
                    FreeFreeLink(pMergedLink);
                    return;
                }

                pMergedLink = pFreeLink;
            }
            else if (pFreeLink->m_pFreeRam + pFreeLink->m_lFreeSize == pBlock)
            {
                pBlock = pFreeLink->m_pFreeRam;
                lBlockSize += pFreeLink->m_lFreeSize;
                pFreeLink->m_lFreeSize = lBlockSize;

                if (pMergedLink)
                {
                    FreeFreeLink(pMergedLink);
                    return;
                }

                pMergedLink = pFreeLink;
            }
        }

        if (!pMergedLink)
        {
            SFreeLink* pNewLink = AllocFreeLink();
            ZASSERT(pNewLink);

            pNewLink->m_pFreeRam = pBlock;
            pNewLink->m_lFreeSize = lBlockSize;
        }
    }
    
    bool ZMallocSimple::Shrink(char*, uint32_t)
    {
        // TODO: Reverse this method, PS2 version keeps it as a no-op.
        return false;
    }
    
    uint32_t ZMallocSimple::AllocSize(const char* pRam)
    {
        // Guard against pointers we do not own: the payload must lie inside
        // the pool's address span. Each free link knows its block end exactly
        // (m_pFreeRam + m_lFreeSize); the block start is that minus what has
        // ever been handed out, bounded below by (end - m_lTotalSize).
        // Without a separate block list this is a conservative span check.
        if (!pRam)
        {
            return 0u;
        }

        const char* pPoolLo = nullptr;
        const char* pPoolHi = nullptr;
        for (uint32_t i = 0u; i < m_lNrFreeLinks; ++i)
        {
            const char* pBlockEnd = m_pFreeLinks[i].m_pFreeRam + m_pFreeLinks[i].m_lFreeSize;
            const char* pBlockStart = pBlockEnd - m_lTotalSize; // conservative lower bound
            if (!pPoolLo || pBlockStart < pPoolLo) pPoolLo = pBlockStart;
            if (!pPoolHi || pBlockEnd > pPoolHi)   pPoolHi = pBlockEnd;
        }

        // No blocks registered (empty pool) or pointer outside the span.
        if (!pPoolLo || pRam < pPoolLo + m_lAlignment || pRam >= pPoolHi)
        {
            return 0u;
        }

        // Header holds the whole-block size; subtract the m_lAlignment header
        // bytes to get the payload capacity (see layout note at the top).
        return *reinterpret_cast<const uint32_t*>(pRam - m_lAlignment) - m_lAlignment;
    }
    
    SFreeLink* ZMallocSimple::AllocFreeLink()
    {
        ZASSERT(m_lNrFreeLinks < m_lMaxNrFreeLinks);
        
        return &m_pFreeLinks[m_lNrFreeLinks++];
    }
    
    void ZMallocSimple::FreeFreeLink(SFreeLink* pLink)
    {
        ZASSERT(0 < m_lNrFreeLinks);

        // Swap-remove: move the last used link into the freed slot, so used
        // links always stay packed at the front of m_pFreeLinks.
        SFreeLink* pLastLink = &m_pFreeLinks[--m_lNrFreeLinks];
        if (pLink != pLastLink)
        {
            *pLink = *pLastLink;
        }
    }
}