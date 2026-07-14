#include <Glacier/ZSTL/ZRefAlloc.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZRefAlloc::ZRefAlloc()
    {
        Initialize();
    }

    void ZRefAlloc::Initialize()
    {
        m_lNumLinks = 0;
        m_lNextFree = FIRST_REF_LINK;

        for (uint32_t i = FIRST_REF_LINK; i < LAST_REF_LINK; ++i)
            m_Links[i].m_lNext = (i + 1) | FREE_LINK_FLAG;

        m_Links[LAST_REF_LINK].m_lNext = 0;
    }

    bool ZRefAlloc::IsEmpty() const
    {
        return m_lNumLinks == 0;
    }

    uint32_t ZRefAlloc::AllocRef()
    {
        ZASSERT(m_lNextFree != 0);

        SRefLink* pLink = &m_Links[m_lNextFree];
        uint32_t lIndex = static_cast<uint32_t>(m_lNextFree);
        m_lNextFree = pLink->m_lNext & ~FREE_LINK_FLAG;
        ++m_lNumLinks;
        pLink->m_lNext = 0xFFFFFFFEu;

        return lIndex;
    }

    void ZRefAlloc::FreeRef(uint32_t lIndex)
    {
        ZASSERT(lIndex && lIndex != REFCHAIN_END);

        m_Links[lIndex].m_lNext = static_cast<uint32_t>(m_lNextFree) | FREE_LINK_FLAG;
        m_lNextFree = static_cast<int>(lIndex);
        --m_lNumLinks;
    }

    uint32_t ZRefAlloc::AddToChain(uint32_t lChainEntry, uint32_t rValue)
    {
        uint32_t lIndex = AllocRef();
        m_Links[lIndex].m_rRef = rValue;

        if (lChainEntry != REFCHAIN_END && lChainEntry)
        {
            m_Links[lIndex].m_lNext = m_Links[lChainEntry].m_lNext;
            m_Links[lChainEntry].m_lNext = lIndex;
            return lChainEntry;
        }

        m_Links[lIndex].m_lNext = REFCHAIN_END;
        return lIndex;
    }

    void ZRefAlloc::FreeChain(uint32_t lChainIdx)
    {
        if (IsEmpty())
            return;

        while (lChainIdx != REFCHAIN_END)
        {
            if (!lChainIdx)
                break;

            uint32_t lCurrent = lChainIdx;
            lChainIdx = m_Links[lChainIdx].m_lNext;
            FreeRef(lCurrent);
        }
    }

    SRefLink* ZRefAlloc::GetLink(uint32_t lIndex)
    {
        if (!lIndex || lIndex == REFCHAIN_END)
            return nullptr;

        return &m_Links[lIndex];
    }

    const SRefLink* ZRefAlloc::GetLink(uint32_t lIndex) const
    {
        if (!lIndex || lIndex == REFCHAIN_END)
            return nullptr;

        return &m_Links[lIndex];
    }

    uint32_t ZRefAlloc::GetChainLength(uint32_t lChainIdx) const
    {
        uint32_t lLength = 0;

        for (auto* pLink = GetLink(lChainIdx); pLink; pLink = GetLink(pLink->m_lNext))
            ++lLength;

        return lLength;
    }

    void ZRefAlloc::SaveRefChain(IOutputSerializerStream*, uint32_t)
    {
        // Not reversed yet: depends on serializer token/exchange helpers that are not implemented.
        ZASSERT(false);
    }

    uint32_t ZRefAlloc::LoadRefChain(IInputSerializerStream*)
    {
        // Not reversed yet: depends on serializer token/exchange helpers that are not implemented.
        ZASSERT(false);
        return 0;
    }
}
