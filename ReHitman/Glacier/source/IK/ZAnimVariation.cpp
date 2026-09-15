#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        static constexpr int MAX_MATCHES = 11;
    }
    
    ZAnimVariation::ZAnimVariation() = default;

    void ZAnimVariation::SetData(const char* pData)
    {
        m_Data = pData;
    }

    int32_t ZAnimVariation::GetNumVariations() const
    {
        const SFlagOffsetPair* const pPairs = GetPairs();

        if (!pPairs)
        {
            return 0;
        }

        int32_t numVariations = 0;
        while (pPairs[numVariations].iFlags != 0)
        {
            ++numVariations;
        }

        return numVariations;
    }

    uint32_t ZAnimVariation::GetAnimOffset(uint32_t lId, float fRand) const
    {
        ZASSERT(fRand >= 0.0f && fRand <= 1.0f);

        struct SMatch
        {
            int32_t offset;
            float cumulativeWeight;
        };

        SMatch matches[MAX_MATCHES];
        float totalWeight = 0.0f;
        uint32_t matchedFlags = 0;
        int32_t matchesFound = 0;

        const SFlagOffsetPair* pPairs = GetPairs();
        if (!pPairs)
        {
            return 0;
        }

        for (;; ++pPairs)
        {
            const uint32_t flags = pPairs->iFlags;
            const uint32_t weightMask = flags & 7;
            const uint32_t requiredFlags = flags & ~7u;

            if ((requiredFlags & lId) == requiredFlags
                && (matchesFound == 0 || matchedFlags == requiredFlags))
            {
                totalWeight += AnimWeight(weightMask);
                matchedFlags = requiredFlags;
                matches[matchesFound].offset = pPairs->iOffset;
                matches[matchesFound].cumulativeWeight = totalWeight;
                ++matchesFound;

                ZASSERT(matchesFound <= MAX_MATCHES);
            }

            if (flags == 0)
            {
                break;
            }
        }

        ZASSERT(matchesFound != 0);

        if (matchesFound == 1)
        {
            return matches[0].offset;
        }

        const float randomWeight = fRand * totalWeight;
        for (int32_t i = 0; i < matchesFound; ++i)
        {
            if (randomWeight <= matches[i].cumulativeWeight)
            {
                return matches[i].offset;
            }
        }

        ZASSERT(false);
        return 0;
    }

    bool ZAnimVariation::IsValid() const
    {
        return m_Data != nullptr;
    }
    
    float ZAnimVariation::AnimWeight(uint32_t mask) const
    {
        float fRet = 1.0f;

        if (mask & 4)
        {
            fRet *= .1f;
        }

        if (mask & 2)
        {
            fRet *= .33f;
        }
        
        if (mask & 1)
        {
            fRet *= .5f;
        }

        return fRet;
    }
    
    SFlagOffsetPair* ZAnimVariation::GetPairs() const
    {
        return const_cast<SFlagOffsetPair*>(reinterpret_cast<const SFlagOffsetPair*>(m_Data));
    }
}
