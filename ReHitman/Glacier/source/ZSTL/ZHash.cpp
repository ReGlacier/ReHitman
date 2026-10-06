#include <Glacier/ZSTL/ZHash.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cmath>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t kMinHashSize = 32;
        constexpr uint32_t kGoldenRatioHashMultiplier = 2654435769u;

        uint32_t NextPowerOfTwoAtLeast(uint32_t value)
        {
            uint32_t size = 1;

            while (size < value)
                size <<= 1;

            return size;
        }

    }

    ZHashBase::ZHashBase(uint32_t lMinSize)
    {
        m_aTaken = nullptr;
        m_lSize = 0;
        m_lCount = 0;
        m_lLongestSequence = 0;

        if (lMinSize < kMinHashSize)
            lMinSize = kMinHashSize;

        m_lMinSize = NextPowerOfTwoAtLeast(lMinSize);
    }

    ZHashBase::~ZHashBase()
    {
        ZUniMemory::Free(m_aTaken);
        m_aTaken = nullptr;
    }

    uint32_t ZHashBase::hf1(uint32_t iSeed) const
    {
        return static_cast<uint32_t>((static_cast<uint64_t>(kGoldenRatioHashMultiplier * iSeed) * m_lSize) >> 32);
    }

    uint32_t ZHashBase::hf2(uint32_t iSeed) const
    {
        return iSeed | 1;
    }

    void ZHashBase::Shrink()
    {
        if (m_lSize > m_lMinSize && m_lSize > kMinHashSize)
            SetSize(m_lSize >> 1);
    }

    void ZHashBase::Grow()
    {
        SetSize(2 * m_lSize);
    }

    uint32_t ZHashBase::Count() const
    {
        return m_lCount;
    }

    void ZHashBase::ClearTaken(uint32_t iIndex)
    {
        const uint32_t mask = ~(1u << (iIndex & 31));
        ZASSERT(mask == _rotl(~1u, iIndex));

        m_aTaken[iIndex >> 5] &= mask;
    }

    bool ZHashBase::IsTaken(uint32_t iIndex) const
    {
        return (m_aTaken[iIndex >> 5] & (1u << (iIndex & 31))) != 0;
    }

    void ZHashBase::SetTaken(uint32_t iIndex)
    {
        m_aTaken[iIndex >> 5] |= 1u << (iIndex & 31);
    }

}
