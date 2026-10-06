#include <Glacier/ZDistCheck.h>
#include <Glacier/ZAction.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/ZSTL/ZMath.h>

#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/GameBase/ZPlayer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/ReGlacier.h>

#include <cstring>

namespace Glacier
{
    namespace
    {
        constexpr int DISTCHECK_NUM_SECTIONS = 8;

        // The original engine stores per-section inclusive upper indices; an empty
        // table is represented by limits[i] == -1, so the live element count is
        // limits[7] + 1 (0 when nothing is registered).
        inline int MaxElements() { return 1024; }
    }

    int32_t ZDistCheck::GetSectionDist(int lSectionNr)
    {
        // action.cpp line ~ (engine): distance boundary for a section is 500 << s.
        if (lSectionNr >= 0)
            return 500 << lSectionNr;
        return 0;
    }

    ZDistCheck::ZDistCheck()
    {
        m_rOtherObject = 0;
        for (int i = 0; i < DISTCHECK_NUM_SECTIONS; ++i)
        {
            m_aLimits[i] = -1;
            m_fPrio[i] = 1.0f;
        }
        memset(m_pArray, 0, sizeof(m_pArray));
    }

    ZDistCheck::~ZDistCheck()
    {
    }

    void ZDistCheck::Init()
    {
        if (g_pGameData)
        {
            ZPlayer* pPlayer = g_pGameData->GetPlayer(0);
            if (pPlayer)
                m_rOtherObject = pPlayer->GetRef();
        }
    }

    void ZDistCheck::SetOtherObj(unsigned int lRef)
    {
        m_rOtherObject = lRef;
    }

    ZREF ZDistCheck::GetOtherObj()
    {
        return m_rOtherObject;
    }

    ZREF ZDistCheck::Get(int lIndex)
    {
        return m_pArray[lIndex];
    }

    int32_t ZDistCheck::Count(int lSectionNr)
    {
        // Cumulative count of elements up to and including the given section.
        // Section 7 therefore returns the total table size.
        return m_aLimits[lSectionNr] + 1;
    }

    int32_t ZDistCheck::Find(unsigned int lRef)
    {
        for (int i = 0; i < Count(7); ++i)
        {
            if (m_pArray[i] == lRef)
                return i;
        }
        return -1;
    }

    bool ZDistCheck::Exists(unsigned int lRef)
    {
        return Find(lRef) != -1;
    }

    int32_t ZDistCheck::GetSectionNr(int lIndex)
    {
        for (int i = 0; i < DISTCHECK_NUM_SECTIONS; ++i)
        {
            if (i >= DISTCHECK_NUM_SECTIONS)
            {
                // INT3 in action.cpp line 111: index fell outside every section.
                ZASSERT(!"GetSectionNr: index out of all sections");
                return -1;
            }

            const int lLower = (i == 0) ? 0 : m_aLimits[i - 1] + 1;
            if (lIndex >= lLower && m_aLimits[i] >= lIndex)
                return i;
        }

        ZASSERT(!"GetSectionNr: index out of all sections");
        return -1;
    }

    void ZDistCheck::Swap(int lIndexA, int lIndexB)
    {
        const ZREF rTmp = m_pArray[lIndexA];
        m_pArray[lIndexA] = m_pArray[lIndexB];
        m_pArray[lIndexB] = rTmp;
    }

    int32_t ZDistCheck::Promote(int lIndex)
    {
        Verify();

        const int lSection = GetSectionNr(lIndex);
        ZASSERT(lSection > 0); // can only move up from a non-first section

        const int lTarget = (lSection == 0) ? 0 : m_aLimits[lSection - 1];
        if (lTarget != lIndex)
            Swap(lIndex, lTarget);

        for (int i = lSection - 1; i > 0 && m_aLimits[i - 1] == m_aLimits[lSection - 1]; --i)
            ++m_aLimits[i - 1];
        ++m_aLimits[lSection - 1];

        Verify();
        return lTarget;
    }

    int32_t ZDistCheck::Demote(int lIndex)
    {
        Verify();

        const int lSection = GetSectionNr(lIndex);
        if (lSection >= DISTCHECK_NUM_SECTIONS - 1)
        {
            // action.cpp line 146: ZASSERT(iSection < DISTCHECK_NUM_SECTIONS-1)
            ZASSERT(lSection < DISTCHECK_NUM_SECTIONS - 1);
        }

        const int lTarget = m_aLimits[lSection];
        if (lTarget != lIndex)
            Swap(lIndex, lTarget);

        for (int i = lSection - 1; i > 0 && m_aLimits[i] == m_aLimits[lSection]; --i)
            --m_aLimits[i];
        --m_aLimits[lSection];

        Verify();
        return lTarget;
    }

    void ZDistCheck::Verify()
    {
        // Section bounds must be monotonically non-decreasing and never exceed the table.
        for (int i = 0; i < DISTCHECK_NUM_SECTIONS; ++i)
        {
            if (i > 0 && m_aLimits[i] < m_aLimits[i - 1])
                ZASSERT(!"Verify: section limits not monotonic");
        }
    }

    int32_t ZDistCheck::Add(unsigned int lRef)
    {
        if (Exists(lRef))
            ZASSERT(!"Add: reference already present");

        if (Count(7) >= MaxElements())
            ZASSERT(!"Add: table overflow");

        const int lIndex = ++m_aLimits[7];
        m_pArray[lIndex] = lRef;
        m_fPrio[7] = 1.0f;
        return lIndex;
    }

    void ZDistCheck::Remove(int lIndex)
    {
        const int lTotal = Count(7);
        if (lIndex >= lTotal)
            ZASSERT(!"Remove: index out of range");

        const int lLast = m_aLimits[7];

        // Pull the removed element to the end of its section chain, then collapse
        // every section boundary that pointed at the vacated final slot.
        const int lSection = GetSectionNr(lIndex);
        int lIdx = lIndex;
        for (int i = lSection; i < DISTCHECK_NUM_SECTIONS && lIdx < m_aLimits[7]; ++i)
        {
            if (lIdx >= m_aLimits[i])
                break;
            lIdx = Promote(lIdx);
        }

        if (lIdx < lLast)
            Swap(lIdx, lLast);
        m_pArray[lLast] = 0xDEADBEEF;

        int* pLimit = &m_aLimits[DISTCHECK_NUM_SECTIONS - 1];
        while (pLimit >= m_aLimits && *pLimit == lLast)
        {
            --*pLimit;
            --pLimit;
        }
    }

    void ZDistCheck::Migrate(ZDistCheck* pOther)
    {
        m_rOtherObject = pOther->m_rOtherObject;
        memcpy(m_pArray, pOther->m_pArray, sizeof(m_pArray));
        memcpy(m_aLimits, pOther->m_aLimits, sizeof(m_aLimits));
        memcpy(m_fPrio, pOther->m_fPrio, sizeof(m_fPrio));
    }

    void ZDistCheck::Update()
    {
        // Pick the section that has waited the longest (highest accumulated age) and
        // refresh that single distance band this frame; the per-section age grows
        // faster for nearer sections so near ranges are refreshed more often.
        int lSection = 0;
        float fBest = 0.0f;
        for (int i = 0; i < DISTCHECK_NUM_SECTIONS; ++i)
        {
            m_fPrio[i] += 1.0f / static_cast<float>(GetSectionDist(i) + 1);
            if (m_fPrio[i] > fBest)
            {
                fBest = m_fPrio[i];
                lSection = i;
            }
        }
        m_fPrio[lSection] = 0.0f;

        ZGEOM* pOther = ZGEOM::RefToPtr(m_rOtherObject);
        if (pOther)
        {
            ZVector3 vPos;
            pOther->GetRootPoint(vPos);
            UpdateSection(lSection, vPos.Get());
        }
    }

    void ZDistCheck::UpdateSection(int lSectionNr, float* pReference)
    {
        Verify();

        const int lFirst = (lSectionNr <= 0) ? 0 : m_aLimits[lSectionNr - 1] + 1;
        const float fLowerBound = static_cast<float>(GetSectionDist(lSectionNr - 1));
        const float fUpperBound = (lSectionNr >= DISTCHECK_NUM_SECTIONS - 1)
            ? 1e38f
            : static_cast<float>(GetSectionDist(lSectionNr));

        ZGEOM* pOther = ZGEOM::RefToPtr(m_rOtherObject);

        for (int lIdx = lFirst; m_aLimits[lSectionNr] >= lIdx; )
        {
            const ZREF rEntry = m_pArray[lIdx];
            ZEventBase* pEvent = ZEventBase::RefToPtr(rEntry);
            if (!pEvent)
            {
                Remove(lIdx);
                continue;
            }

            ZGEOM* pGeom = static_cast<ZAction*>(pEvent)->GetGeom();
            const float fDist = pOther ? pGeom->GetDistanceToObject(pOther) : 0.0f;

            if (fDist < fLowerBound)
            {
                const int lPromoted = Promote(lIdx);
                if (m_pArray[lPromoted] != rEntry)
                    ZASSERT(m_pArray[lPromoted] == rEntry);

                if (lIdx != lPromoted)
                    continue;
            }
            else if (fUpperBound < fDist)
            {
                const int lDemoted = Demote(lIdx);
                if (m_pArray[lDemoted] != rEntry)
                    ZASSERT(m_pArray[lDemoted] == rEntry);

                // PS2 vtable slot +196 = ZAction::FrameUpdate(ZGEOM*): the element that
                // just left the active range is re-evaluated against the target geom.
                static_cast<ZAction*>(pEvent)->ActionFrameUpdate(pOther);

                if (lIdx != lDemoted)
                    continue;
            }

            ++lIdx;
        }
    }

    void ZDistCheck::UpdateSingleObject(unsigned int lRef)
    {
        int lIdx = Find(lRef);
        if (lIdx == -1)
            ZASSERT(lIdx != -1);

        ZGEOM* pOther = ZGEOM::RefToPtr(m_rOtherObject);
        if (!pOther)
            return;

        ZEventBase* pEvent = ZEventBase::RefToPtr(lRef);
        if (!pEvent)
            return;

        ZGEOM* pGeom = static_cast<ZAction*>(pEvent)->GetGeom();
        if (!pGeom)
            return;

        const float fDist = pGeom->GetDistanceToObject(pOther);

        for (int lSection = GetSectionNr(lIdx); lSection > 0; --lSection)
        {
            if (fDist >= static_cast<float>(GetSectionDist(lSection - 1)))
                break;
            lIdx = Promote(lIdx);
        }
    }

    void ZDistCheck::LoadObject(IInputSerializerStream& stream)
    {
        // No PC/PS2 field-level serialization body is available for this table.
        // Round-trip the raw table so save/load stays self-consistent.
        for (int i = 0; i < DISTCHECK_NUM_SECTIONS; ++i)
        {
            stream.ExchangeData(m_aLimits[i]);
            stream.ExchangeData(m_fPrio[i]);
        }
        stream.ExchangeData(m_rOtherObject);
        for (int i = 0; i < MaxElements(); ++i)
            stream.ExchangeData(m_pArray[i]);
    }

    void ZDistCheck::SaveObject(IOutputSerializerStream& stream)
    {
        for (int i = 0; i < DISTCHECK_NUM_SECTIONS; ++i)
        {
            stream.ExchangeData(m_aLimits[i]);
            stream.ExchangeData(m_fPrio[i]);
        }
        stream.ExchangeData(m_rOtherObject);
        for (int i = 0; i < MaxElements(); ++i)
            stream.ExchangeData(m_pArray[i]);
    }
}

#pragma region " --- RTTI --- "
    // ZDistCheck is not an RTP-registered class (no cBase / factory entry); it is a
    // plain helper object owned by ZActionController, so no DEFINE_*_CLASS is emitted.
#pragma endregion
