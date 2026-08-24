#include <Glacier/Geom/ZGeomListTypeUtils.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Geom/ZBOUND.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/ZSTL/StringUtils.h> // striwcmp
#include <cstring>


namespace Glacier
{
    ZGROUP::ZGROUP(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGEOM(psName, pBaseGeom)
    {
        m_pZBounds = nullptr;
        m_OverRideNearFar[0] = 0.0f;
        m_OverRideNearFar[1] = 0.0f;
        m_lGroupCon = ZGRPCF_LIGHT_SHINES_IN | ZGRPCF_LIGHT_SHINES_OUT;
        m_fPFResMultiplier = 1.0f;
        m_vSizeInsideCheck[0] = 0.0f;
        m_vSizeInsideCheck[1] = 0.0f;
        m_vSizeInsideCheck[2] = 0.0f;
        m_vCenInsideCheck[0] = 0.0f;
        m_vCenInsideCheck[1] = 0.0f;
        m_vCenInsideCheck[2] = 0.0f;
    }

    ZGROUP::~ZGROUP()
    {
        while (m_pGroupFirst)
        {
            ZUniMemory::Delete(m_pGroupFirst);
        }

        if (m_pZBounds)
        {
            REFTAB::DeleteReftab(m_pZBounds);
            m_pZBounds = nullptr;
        }
    }

    void ZGROUP::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        if (bSaving)
        {
            uint32_t lEntriesNr = 0u;
            for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
                ++lEntriesNr;

            stream.ExchangeContainer("REFs", lEntriesNr);

            auto* paREFs = (ZREF*)alloca(sizeof(ZREF) * lEntriesNr);
            auto* pCurrentREF = paREFs;
            for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
                *(pCurrentREF++) = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);

            stream.ExchangeArray("REFs", paREFs, lEntriesNr);
            ZGEOM::LoadSave(stream, bSaving);

            for (auto* base_geom = m_pGroupFirst; base_geom; base_geom = base_geom->Next())
            {
                ZASSERT(base_geom->GetGeom() && base_geom->GetGeom()->BaseGeom() == base_geom);

                auto* pGeom = base_geom->GetGeom();
                stream.Exchange(pGeom->Name(), *pGeom);
            }
        }
        else
        {
            uint32_t lSavedCount = 0u;
            stream.ExchangeContainer("REFs", lSavedCount);

            auto* paSavedRefs = (ZREF*)alloca(sizeof(ZREF) * lSavedCount);
            stream.ExchangeArray("REFs", paSavedRefs, lSavedCount);

            uint32_t lCurrentCount = 0u;
            for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
                ++lCurrentCount;

            auto* paToRemove = (ZBaseGeom**)alloca(sizeof(ZBaseGeom*) * lCurrentCount);
            uint32_t lRemoveCount = 0u;

            for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
            {
                const ZREF rRef = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);

                bool bFound = false;
                if (lSavedCount)
                {
                    for (uint32_t j = 0u; j < lSavedCount; ++j)
                    {
                        if (paSavedRefs[j] == rRef)
                        {
                            bFound = true;
                            break;
                        }
                    }
                }

                if (!bFound && rRef)
                    paToRemove[lRemoveCount++] = pBaseGeom;
            }

            for (uint32_t i = 0u; i < lRemoveCount; ++i)
                DetachGeom(paToRemove[i], false);

            ZGEOM::LoadSave(stream, bSaving);

            for (uint32_t i = 0u; i < lSavedCount; ++i)
            {
                auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(paSavedRefs[i]);
                ZASSERT(pBaseGeom->m_pExtraGeom);

                auto* pThisBaseGeom = this->BaseGeom();

                if (pBaseGeom->m_pParent != pThisBaseGeom)
                {
                    if ((pBaseGeom->m_lControl & ZCDYNAMIC) != 0 && (pThisBaseGeom->m_lControl & (ZCDYNAMIC | ZCHASDYNAMICPARENT)) != 0)
                        pBaseGeom->SetControl(0u, ZCDYNAMIC);

                    AttachGeom(pBaseGeom, true);
                }

                auto* pGeom = pBaseGeom->m_pExtraGeom;
                const char* pszName = pBaseGeom->Name();
                stream.Exchange(pszName, *pGeom);
            }
        }
    }

    const RTP::ZPropertyInfo& ZGROUP::GetProperties() const
    {
        return ZGROUP::Info;
    }

    uint32_t ZGROUP::GetObjectId() const
    {
        return ZGROUP::m_Id;
    }

    void ZGROUP::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZGROUP::m_Id;
        mask = ZGROUP::m_Mask;
    }

    ZGEOMCLASSINFO* ZGROUP::GetOldClassInfo() const
    {
        return ZGROUP::m_OldClassInfo;
    }

    bool ZGROUP::DoInit()
    {
        if (!ZGEOM::DoInit())
            return false;

        uint32_t iCount = 0;
        for (ZBaseGeom* pGeom = m_pGroupFirst; pGeom; pGeom = pGeom->Next())
        {
            ++iCount;
        }

        ZREF* aGeomRefs = static_cast<ZREF*>(alloca(sizeof(ZREF) * iCount));
        ZREF* pGeomRef = aGeomRefs;
        for (ZBaseGeom* pGeom = m_pGroupFirst; pGeom; pGeom = pGeom->Next())
        {
            *pGeomRef++ = ZGeomBuffer::Instance().GeomPtrToRef(pGeom);
        }

        while (iCount)
        {
            ZREF rGeomRef = aGeomRefs[--iCount];
            if (ZBaseGeom* pGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(rGeomRef))
            {
                pGeom->DoInit();
            }
        }

        return true;
    }

    void ZGROUP::CalcCenSize()
    {
        ZVector3 vMax(-9.9999997e37f);
        ZVector3 vMin(9.9999997e37f);
        ZVector3 vMaxInside(-9.9999997e37f);
        ZVector3 vMinInside(9.9999997e37f);

        if (!BaseGeom()->ParentGroup() && (GeomControl() & 1) != 0)
        {
            vMax = ZVector3(2500.0f, 0.0f, 2500.0f);
            vMin = ZVector3(-2500.0f, 0.0f, -2500.0f);
        }

        for (ZBaseGeom* pGeom = m_pGroupFirst; pGeom; pGeom = pGeom->Next())
        {
            if (!pGeom->DisableParentBoundAdjust())
            {
                const char* pName = pGeom->Name();
                if (strcmp(pName, "MOVETOCREATION") != 0)
                {
                    ZVector3 vCen;
                    ZVector3 vSize;

                    if (pGeom->GetParentBox(vCen, vSize))
                    {
                        ZVector3 vChildMax;
                        ZVector3 vChildMin;

                        vadd(vChildMax, vCen, vSize);
                        vsub(vChildMin, vCen, vSize);

                        vmax(vMax, vChildMax);
                        vmin(vMin, vChildMin);
                        vmax(vMaxInside, vChildMax);
                        vmin(vMinInside, vChildMin);
                    }
                }
            }
        }

        if (vMax.x == -9.9999997e37f)
        {
            BaseGeom()->SetCen(0.0f, 0.0f, 0.0f);
            BaseGeom()->SetSize({ 0.0f });
            BaseGeom()->SetRadius(0.0f);
        }
        else
        {
            ZVector3 vCen;
            vadd(vCen, vMax, vMin);
            vscalar(vCen, 0.5f);
            BaseGeom()->SetCen(vCen);

            ZVector3 vSize;
            vsub(vSize, vMax, vCen);
            BaseGeom()->SetSize(vSize);
            BaseGeom()->SetRadius(vSize.Length() + 1.0f);
        }

        if (vMaxInside.x == -9.9999997e37f)
        {
            vreset(m_vSizeInsideCheck);
            vreset(m_vCenInsideCheck);
        }
        else
        {
            vadd(m_vCenInsideCheck, vMaxInside.Get(), vMinInside.Get());
            vscalar(m_vCenInsideCheck, 0.5f);
            vsub(m_vSizeInsideCheck, vMaxInside.Get(), m_vCenInsideCheck);
        }
    }

    void ZGROUP::PreSaveGame()
    {
        ZGEOM::PreSaveGame();

        for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
            pBaseGeom->GetGeom()->PreSaveGame();
    }

    eGlobalTreeType ZGROUP::GetBoundTreeType() const
    {
        return eGlobalTreeType::GT_Groups;
    }

    void ZGROUP::RecurGetNext(ZBaseGeom** ZGeom)
    {
        if (!m_NrAttachGeom && *ZGeom == BaseGeom())
        {
            *ZGeom = nullptr;
            return;
        }

        auto* pBaseGeom = *ZGeom;

        ZBaseGeom* pNext = nullptr;
        if (pBaseGeom->IsDerivedFrom<ZGROUP>())
        {
            pNext = static_cast<ZGROUP*>(pBaseGeom->GetGeom())->m_pGroupFirst;
        }

        if (!pNext)
        {
            while (true)
            {
                pNext = pBaseGeom->Next();
                if (pNext)
                {
                    break;
                }

                pBaseGeom = pBaseGeom->m_pParent;
                *ZGeom = pBaseGeom;

                if (pBaseGeom == BaseGeom())
                {
                    *ZGeom = nullptr;
                    return;
                }

                if (!pBaseGeom)
                {
                    return;
                }
            }
        }

        *ZGeom = pNext;
    }

    void ZGROUP::SetRootTM(const ZMat3x3& RTMat, const ZVector3& RTPos)
    {
        ZGEOM::SetRootTM(RTMat, RTPos);
    }

    void ZGROUP::Freeze(bool OnOff)
    {
        ZGEOM::Freeze(OnOff);

        for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
            pBaseGeom->GetGeom()->Freeze(OnOff);
    }

    void ZGROUP::SetMoving(bool bMoving)
    {
        for (auto* pBaseGeom = BaseGeom(); pBaseGeom;)
        {
            auto* pGeom = pBaseGeom->GetGeom();
            pGeom->CreateExData();
            pGeom->SetMoving(true); // Weird! In PC build it's always assing true
            pGeom->RecurGetNext(&pBaseGeom);
        }
    }

    void ZGROUP::SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget)
    {
        ZGEOM::SendCommandRecursive(Msg, pData, pTarget);

        for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
        {
            auto* pGeom = pBaseGeom->GetGeom();
            if (pGeom)
            {
                pGeom->SendCommandRecursive(Msg, pData, pTarget);
            }
        }
    }

    bool ZGROUP::CheckPointInside(ZVector3& pPoint, float fDotDist)
    {
        ZVector3 vPoint {};
        vsub(vPoint, m_vCenInsideCheck, pPoint);

        if (m_vSizeInsideCheck[0] - fDotDist < fabs(vPoint.x) ||
            m_vSizeInsideCheck[1] - fDotDist < fabs(vPoint.y) ||
            m_vSizeInsideCheck[2] - fDotDist < fabs(vPoint.z))
        {
            return false;
        }

        if (!m_pZBounds)
            return true;

        for (auto it = m_pZBounds->As<ZREF>().begin(); it != m_pZBounds->As<ZREF>().end();)
        {
            auto rGeom = *it;
            auto* pGeom = ZGEOM::RefToPtr(rGeom);
            if (!pGeom)
            {
                it.Erase();
            }
            else
            {
                vsub(vPoint, pPoint, pGeom->Pos());
                vmtmul(vPoint, pGeom->Mat());
                if (pGeom->CheckPointInside(vPoint, fDotDist))
                {
                    return true;
                }

                ++it;
            }
        }

        return false;
    }

    bool ZGROUP::CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* vHalfSize)
    {
        ZMat3x3 mIdent;
        mreset(mIdent.data);

        if (rectBoxColi(mIdent.data, m_vCenInsideCheck, m_vSizeInsideCheck,
                        mMat.data, vPos, vHalfSize))
        {
            if (!m_pZBounds)
                return true;

            ZMat3x3 mLocalMat;
            ZVector3 vLocalPos;

            for (auto it = m_pZBounds->As<ZREF>().begin(); it != m_pZBounds->As<ZREF>().end();)
            {
                auto rGeom = *it;
                auto* pGeom = ZGEOM::RefToPtr(rGeom);
                if (!pGeom)
                {
                    it.Erase();
                }
                else
                {
                    vsub(vLocalPos, vPos, pGeom->Pos());
                    vmtmul(vLocalPos, pGeom->Mat());
                    mmtmul(mLocalMat, mMat, pGeom->Mat());

                    if (pGeom->CheckBoxInside(mLocalMat, vLocalPos, vHalfSize))
                        return true;

                    ++it;
                }
            }
        }

        return false;
    }

    float ZGROUP::GetPointInsideDistance(const ZVector3& vPos)
    {
        if (m_pZBounds)
        {
            float fMinDist = 9.9999997e37f;
            ZVector3 vP;

            for (auto it = m_pZBounds->As<ZREF>().begin(); it != m_pZBounds->As<ZREF>().end();)
            {
                auto rGeom = *it;
                auto* pGeom = ZGEOM::RefToPtr(rGeom);
                if (!pGeom)
                {
                    it.Erase();
                }
                else
                {
                    vsub(vP, vPos, pGeom->Pos());
                    vmtmul(vP, pGeom->Mat());

                    const float fDist = pGeom->GetPointInsideDistance(vP);
                    if (fDist < fMinDist)
                        fMinDist = fDist;

                    ++it;
                }
            }

            return fMinDist;
        }

        ZVector3 vP;
        vsub(vP, m_vCenInsideCheck, vPos);
        return ZCommonAlgorithms::DistanceBoxAndPoint(vP, m_vSizeInsideCheck);
    }

    ZGEOM* ZGROUP::Duplicate(ZGROUP* DestGroup, const char* DupName, bool Recursive)
    {
        auto* pDup = ZGEOM::Duplicate(DestGroup, DupName, false);
        if (pDup && Recursive)
        {
            for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
            {
                auto* pGeom = pBaseGeom->GetGeom();
                if (pGeom)
                {
                    const char* pszName = pBaseGeom->Name();
                    pGeom->Duplicate(reinterpret_cast<ZGROUP*>(pDup), pszName, true);
                }
            }
        }

        return pDup;
    }

    ZGEOM* ZGROUP::DuplicateToResource(ZGROUP* DestGroup, uint32_t lGeomResourceId, const char* DupName, bool Recursive)
    {
        auto* pDup = ZGEOM::DuplicateToResource(DestGroup, lGeomResourceId, DupName, false);
        if (pDup && Recursive)
        {
            for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
            {
                auto* pGeom = pBaseGeom->GetGeom();
                if (pGeom)
                {
                    const char* pszName = pBaseGeom->Name();
                    pGeom->DuplicateToResource(reinterpret_cast<ZGROUP*>(pDup), lGeomResourceId, pszName, true);
                }
            }
        }

        return pDup;
    }

    ZGEOM* ZGROUP::DuplicateToResourceInit(ZGROUP* DestGroup, uint32_t lGeomResourceId, const ZMat3x3* mMat, const ZVector3* vPos, const char* DupName, bool Recursive)
    {
        auto* pDup = ZGEOM::DuplicateToResourceInit(DestGroup, lGeomResourceId, mMat, vPos, DupName, Recursive);
        if (pDup)
        {
            if (mMat && vPos)
            {
                pDup->SetMatPos(*mMat, *vPos);
            }

            // WEIRD!!! MEMORY LEAK
            return pDup->DoInit() ? pDup : nullptr;
        }

        return pDup;
    }

    void ZGROUP::CopyData(const ZGEOM* Source)
    {
        auto* pSource = static_cast<const ZGROUP*>(Source);

        m_OverRideNearFar[0] = pSource->m_OverRideNearFar[0];
        m_OverRideNearFar[1] = pSource->m_OverRideNearFar[1];
        m_lGroupCon = pSource->m_lGroupCon & 0x800u; // Weird, I know
        m_lGroupCon |= (pSource->m_lGroupCon & 0xFFFFF7FFu);
        ZGEOM::CopyData(Source);
    }

    bool ZGROUP::IsRecursiveActivateAllowed()
    {
        return true;
    }

    bool ZGROUP::DynamicGroupOnScreen()
    {
        return false;
    }

    ZGEOM* ZGROUP::FindLoadWorldGeom(const char* pSearchName) const
    {
        return FindMaskGeom(pSearchName, 0x800);
    }

    ZGEOM* ZGROUP::FindMasterGeom(const char* pSearchName) const
    {
        return FindMaskGeom(pSearchName, 0x2);
    }

    ZGEOM* ZGROUP::FindGeom(const char* GName, ZBaseGeom* pZGeomContinue)
    {
        if (!GName || !*GName)
            return nullptr;

        if (!BaseGeom()->ParentGroup() && !_stricmp(GName, "ROOT"))
            return static_cast<ZGEOM*>(this);

        if (pZGeomContinue)
        {
            const char* pszSlash = strrchr(GName, '\\');
            const char* pszSearchName = pszSlash ? pszSlash + 1 : GName;

            auto* pBaseGeom = pZGeomContinue->Next();
            if (!pBaseGeom)
                return nullptr;

            while (pBaseGeom)
            {
                if (!pBaseGeom->IsDerivedFrom<ZGROUP>()
                    || (static_cast<ZGROUP*>(pBaseGeom->GetGeom())->m_lGroupCon & 0x802) == 0)
                {
                    auto* pGeom = pBaseGeom->GetGeom();
                    if (pGeom)
                    {
                        const char* pszName = pBaseGeom->Name();
                        if (!pszName)
                            pszName = "<NONAME>";

                        if (!striwcmp(pszName, pszSearchName))
                            return pGeom;
                    }
                }
                pBaseGeom = pBaseGeom->Next();
                if (!pBaseGeom)
                    return nullptr;
            }
        }

        if (strlen(GName) > 5 && !memcmp(GName, "ROOT\\", 5))
            GName += 5;

        const char* pszSlash = strchr(GName, '\\');
        if (pszSlash)
        {
            auto* pGroupFirst = m_pGroupFirst;
            const size_t lNameLen = static_cast<size_t>(pszSlash - GName);

            while (pGroupFirst && pGroupFirst->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroupGeom = static_cast<ZGROUP*>(pGroupFirst->GetGeom());
                if ((pGroupGeom->m_lGroupCon & 0x802) == 0)
                {
                    const char* pszName = pGroupFirst->Name();
                    if (!pszName)
                        pszName = "<NONAME>";

                    if (strlen(pszName) == lNameLen && !_memicmp(pszName, GName, lNameLen))
                    {
                        auto* pResult = pGroupGeom->FindGeom(pszSlash + 1, nullptr);
                        if (pResult)
                            return pResult;
                    }
                }
                pGroupFirst = pGroupFirst->Next();
                if (!pGroupFirst || !pGroupFirst->IsDerivedFrom<ZGROUP>())
                    return nullptr;
            }
            return nullptr;
        }

        auto* pGroupFirst = m_pGroupFirst;
        while (pGroupFirst)
        {
            if (!pGroupFirst->IsDerivedFrom<ZGROUP>() || (static_cast<ZGROUP*>(pGroupFirst->GetGeom())->m_lGroupCon & 0x802) == 0)
            {
                auto* pGeom = pGroupFirst->GetGeom();
                if (pGeom)
                {
                    const char* pszName = pGroupFirst->Name();
                    if (!pszName)
                        pszName = "<NONAME>";

                    if (!striwcmp(pszName, GName))
                        return pGeom;
                }
            }
            pGroupFirst = pGroupFirst->Next();
            if (!pGroupFirst)
                return nullptr;
        }

        return nullptr;
    }

    int ZGROUP::GroupDepth()
    {
        int lDepth = 0;
        const auto* pBaseGeom = BaseGeom();

        for (; pBaseGeom; ++lDepth)
        {
            pBaseGeom = pBaseGeom->Parent();
        }

        return lDepth;
    }

    float ZGROUP::GetPFResMultiplier() const
    {
        return m_fPFResMultiplier;
    }

    void ZGROUP::LinkBound(ZREF rBound)
    {
        if (m_pZBounds)
        {
            if (m_pZBounds->Exists(rBound))
                return;
        }

        m_pZBounds = ZUniMemory::New<REFTAB>(32, 0);
        m_pZBounds->Add(rBound);
    }

    void ZGROUP::RemoveBound(ZREF rBound)
    {
        if (m_pZBounds)
        {
            m_pZBounds->Remove(rBound);
            if (!m_pZBounds->Count())
            {
                ZUniMemory::Delete(m_pZBounds);
                m_pZBounds = nullptr;
            }
        }
    }

    void ZGROUP::GetAmbientSettings(const ZVector3& p0, float* AmbientDir, float* AmbientLowColor, float* AmbientHiColor) const
    {
        ZERROR("ZGROUP::GetAmbientSettings is disabled - talk to hs@ioi.dk");
    }

    void ZGROUP::CreateParentsRecur(ZGROUP* ZParent, ZGROUP** ZDest, bool bAllowLoadWorlds)
    {
        if (auto* pParentGroup = ZParent->BaseGeom()->ParentGroup())
        {
            CreateParentsRecur(pParentGroup, ZDest, true);

            const char* pszParentName = ZParent->Name();
            auto* pFoundGroup = static_cast<ZGROUP*>((*ZDest)->FindGeom(pszParentName, nullptr));
            if (pFoundGroup)
            {
                while ((pFoundGroup->m_lGroupCon & 0x800u) != 0)
                {
                    pFoundGroup = static_cast<ZGROUP*>((*ZDest)->FindGeom(pszParentName, pFoundGroup->BaseGeom()));
                    if (!pFoundGroup)
                        break;
                }
            }

            if (!pFoundGroup)
            {
                pFoundGroup = static_cast<ZGROUP*>(ZParent->Duplicate(*ZDest, nullptr, false));
            }

            *ZDest = pFoundGroup;
        }
    }

    void ZGROUP::SetOverRideNearFar(const ZVector2& NearFar)
    {
        m_OverRideNearFar[0] = NearFar.x;
        m_OverRideNearFar[1] = NearFar.y;

        if (m_OverRideNearFar[0] == 0.0f && m_OverRideNearFar[1] == 0.0f)
        {
            m_lGroupCon &= ~ZGRPCF_OVERRIDE_NEAR_FAR;
        }
        else
        {
            m_lGroupCon |= ZGRPCF_OVERRIDE_NEAR_FAR;
        }
    }

    void ZGROUP::CorrectCenSizeRecur()
    {
        for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
        {
            auto* pGeom = pBaseGeom->GetGeom();
            if (!(pGeom && pGeom->IsDerivedFrom<ZGROUP>()))
            {
                break;
            }

            static_cast<ZGROUP*>(pGeom)->CorrectCenSize();
        }

        CorrectCenSize();
    }

    void ZGROUP::CorrectCenSize()
    {
        ZVector3 v0, v1, v2, v3;

        vmmul(v0, Cen(), Size());
        vsub(v1, Cen(), Size());
        CalcCenSize();
        if (Radius() != 0.0f)
        {
            vmmul(v2, Cen(), Size());
            vsub(v3, Cen(), Size());
        }
    }

    void ZGROUP::InvalidateBounds()
    {
        for (auto* pIt = this; pIt; pIt = pIt->BaseGeom()->ParentGroup())
        {
            pIt->m_lGroupCon |= ZGRPCF_INVALID_BOUNDS;
        }
    }

    void ZGROUP::AttachGeom(ZGEOM* pGeom, bool bCalcMinMax)
    {
        AttachGeom(pGeom->BaseGeom(), bCalcMinMax);
    }

    void ZGROUP::AttachGeom(ZBaseGeom* pBaseGeom, bool bCalcMinMax)
    {
        if (pBaseGeom->m_pParent)
        {
            auto* pParentGroup = pBaseGeom->ParentGroup();
            pParentGroup->DetachGeom(pBaseGeom, false);
        }

        if ((BaseGeom()->Control() & ZCINACTIVE) != 0 && (pBaseGeom->Control() & ZCINACTIVE) == 0)
            pBaseGeom->MakeInactive();

        if (GetBaseGeomListType(pBaseGeom) == BGLT_Light)
            m_LightList = ZGeomBuffer::Instance().AddGeoms(m_LightList, pBaseGeom, pBaseGeom);

        pBaseGeom->SetParent(BaseGeom());

        if (pBaseGeom->IsDerivedFrom<ZGROUP>())
        {
            pBaseGeom->SetNext(m_pGroupFirst);
            pBaseGeom->SetPrev(nullptr);

            if (m_pGroupFirst)
                m_pGroupFirst->SetPrev(pBaseGeom);
            else
                m_pGroupLast = pBaseGeom;

            m_pGroupFirst = pBaseGeom;
        }
        else
        {
            pBaseGeom->SetPrev(m_pGroupLast);
            pBaseGeom->SetNext(nullptr);

            if (m_pGroupLast)
                m_pGroupLast->SetNext(pBaseGeom);
            else
                m_pGroupFirst = pBaseGeom;

            m_pGroupLast = pBaseGeom;
        }

        ++m_NrAttachGeom;

        if ((BaseGeom()->Control() & ZCHIDDEN) != 0 && !g_pSysInterface->m_pEngineData->m_LoadingGame)
            pBaseGeom->Hide(true);

        if (pBaseGeom->IsDerivedFrom<ZBOUND>())
        {
            const ZREF rRef = ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom);
            LinkBound(rRef);
        }

        auto* pTreeGroup = GetTreeGroup();
        if (pTreeGroup)
            pTreeGroup->AttachTreeGroupChilds(pBaseGeom);

        auto* pDynTreeGroup = GetDynamicTreeGroup();
        if (pDynTreeGroup)
            pDynTreeGroup->AttachDynamicGeoms(pBaseGeom);

        if ((pBaseGeom->Control() & ZCDYNAMIC) != 0
            || (pBaseGeom->IsDerivedFrom<ZGROUP>() && (static_cast<ZGROUP*>(pBaseGeom->GetGeom())->m_lGroupCon & ZGRPCF_INVALID_BOUNDS) != 0))
        {
            InvalidateBounds();
        }

        if (pBaseGeom->IsDerivedFrom<ZLIGHT>()
            || (pBaseGeom->IsDerivedFrom<ZGROUP>() && (static_cast<ZGROUP*>(pBaseGeom->GetGeom())->m_lGroupCon & ZGRPCF_GROUP_CONTAINS_LIGHT) != 0))
        {
            GroupContainsLight();
        }

        auto* pOurBaseGeom = BaseGeom();
        const uint32_t lControl = pOurBaseGeom->Control();
        if ((lControl & (ZCDYNAMIC | ZCHASDYNAMICPARENT)) != 0)
        {
            if ((lControl & ZCDYNAMIC) != 0)
            {
                pBaseGeom->SetDynamicParent(pOurBaseGeom);
            }
            else
            {
                auto* pDynamicParent = pOurBaseGeom->GetDynamicParent();
                ZASSERT(pDynamicParent);
                pBaseGeom->SetDynamicParent(pDynamicParent);
            }
        }

        const uint32_t lGeomControl = pBaseGeom->Control();
        if ((lGeomControl & (ZCOWNERDRAW | ZCHIDDEN | ZCINACTIVE)) == 0 && (lGeomControl & ZCDYNAMIC) != 0)
        {
            if (!pBaseGeom->m_pDynId)
                pBaseGeom->AttachToDynamicContainer();

            pBaseGeom->AssignToRooms();
        }

        pBaseGeom->AttachToRoomsDrawLists(nullptr);
    }

    void ZGROUP::DetachGeom(ZBaseGeom* pBaseGeom, bool bDestroying)
    {
        pBaseGeom->DetachFromRoomsDrawLists(nullptr);

        if ((pBaseGeom->Control() & (ZCOWNERDRAW | 0xC00)) == 0 && (pBaseGeom->Control() & ZCDYNAMIC) != 0)
        {
            if (!pBaseGeom->m_pDynId)
                pBaseGeom->DetachFromDynamicContainer(nullptr);

            pBaseGeom->FreeRoomList();
        }

        if ((pBaseGeom->Control() & (ZCHASDYNAMICPARENT | ZCDYNAMIC)) != 0)
            pBaseGeom->RemoveDynamicParent();

        if (!bDestroying)
        {
            auto* pDynTreeGroup = GetDynamicTreeGroup();
            if (pDynTreeGroup)
                pDynTreeGroup->DetachDynamicGeoms(pBaseGeom);

            auto* pTreeGroup = GetTreeGroup();
            if (pTreeGroup)
                pTreeGroup->DetachTreeGroupChilds(pBaseGeom);
        }

        if (GetBaseGeomListType(pBaseGeom) == BGLT_Light)
        {
            if (g_pSysInterface->WindowFirst)
            {
                auto* pBaseGeomToUpdate = pBaseGeom;

                do
                {
                    if (pBaseGeomToUpdate->GetGeom()->IsDerivedFrom<ZROOM>())
                    {
                        g_pSysInterface->WindowFirst->UpdateBaseGeom(pBaseGeomToUpdate);
                    }

                    pBaseGeomToUpdate = pBaseGeomToUpdate->Parent();
                }
                while (pBaseGeomToUpdate);
            }
            m_LightList = ZGeomBuffer::Instance().RemoveGeoms(m_LightList, pBaseGeom, pBaseGeom);
        }

        // Exclude tail
        if (pBaseGeom->GetPrev())
        {
            auto* pNext = pBaseGeom->Next();
            auto* pPrev = pBaseGeom->GetPrev();
            pPrev->SetNext(pNext);
        }
        else
        {
            m_pGroupFirst = pBaseGeom->Next();
        }

        // Exclude front
        if (pBaseGeom->Next())
        {
            auto* pPrev = pBaseGeom->GetPrev();
            auto* pNext = pBaseGeom->Next();
            pNext->SetPrev(pPrev);
        }
        else
        {
            m_pGroupLast = pBaseGeom->GetPrev();
        }

        // Weird drop tech
        pBaseGeom->SetNext(nullptr);
        pBaseGeom->SetPrev(nullptr);
        pBaseGeom->SetPrev(nullptr); // Idk why twice
        pBaseGeom->SetNext(nullptr); // Idk why twice
        pBaseGeom->SetParent(nullptr);

        --m_NrAttachGeom;

        // Drop bounds
        if (auto* pBound = geom_cast<ZBOUND>(pBaseGeom->GetGeom()))
        {
            RemoveBound(pBound->GetRef());
        }

        // Weird final tech
        pBaseGeom->SetParent(nullptr); // Idk why trice
    }

    void ZGROUP::RecurGetNextGroup(const ZBaseGeom** pGroup) const
    {
        auto* pFirstChild = static_cast<const ZGROUP*>((*pGroup)->GetGeom())->m_pGroupFirst;
        if (pFirstChild && pFirstChild->IsDerivedFrom<ZGROUP>())
        {
            *pGroup = pFirstChild;
        }
        else if (*pGroup == BaseGeom())
        {
            *pGroup = nullptr;
        }
        else
        {
            while (true)
            {
                if (auto* pNext = (*pGroup)->Next())
                {
                    *pGroup = pNext;
                    if (pNext->IsDerivedFrom<ZGROUP>())
                    {
                        break;
                    }
                }
                else
                {
                    auto* pParent = (*pGroup)->Parent();
                    *pGroup = pParent;

                    if (pParent == BaseGeom())
                    {
                        *pGroup = nullptr;
                        break;
                    }

                    if (!pParent)
                    {
                        break;
                    }
                }
            }
        }
    }

    void ZGROUP::RecurGetNextExclRoom(const ZBaseGeom** ZGeom) const
    {
        if (!m_NrAttachGeom && (*ZGeom) == BaseGeom())
        {
            *ZGeom = nullptr;
            return;
        }

        const bool bIsGroup = (*ZGeom)->IsDerivedFrom<ZGROUP>();

        if (!bIsGroup
            || (*ZGeom != BaseGeom() && (*ZGeom)->IsDerivedFrom<ZROOM>())
            || static_cast<const ZGROUP*>((*ZGeom)->GetGeom())->m_pGroupFirst == nullptr)
        {
            while (true)
            {
                auto* pNext = (*ZGeom)->Next();
                if (pNext)
                {
                    *ZGeom = pNext;
                    break;
                }

                auto* pParent = (*ZGeom)->Parent();
                *ZGeom = pParent;

                if (pParent == BaseGeom())
                {
                    *ZGeom = nullptr;
                    return;
                }

                if (!pParent)
                    return;
            }
        }
        else
        {
            *ZGeom = static_cast<const ZGROUP*>((*ZGeom)->GetGeom())->m_pGroupFirst;
        }
    }

    void ZGROUP::SetGroupControl(uint32_t lAddBits, uint32_t lRemBits)
    {
        m_lGroupCon = lAddBits | (m_lGroupCon & ~lRemBits);
    }

    uint32_t ZGROUP::GroupControl() const
    {
        return m_lGroupCon;
    }

    void ZGROUP::ResetGroupPosition(bool bReset)
    {
        if (bReset)
        {
            if ((GroupControl() & ZGRPCF_RESET) != 0)
                return;

            SetGroupControl(ZGRPCF_RESET, 0u);
        }
        else
        {
            if ((GroupControl() & ZGRPCF_RESET) == 0)
                return;

            SetGroupControl(0u, ZGRPCF_RESET);
        }

        ZVector3 vOldCen = BaseGeom()->m_vCen;
        BaseGeom()->m_vCen = BaseGeom()->m_vPos;
        BaseGeom()->m_vPos = vOldCen;

        ZVector3 vDelta;
        vsub(vDelta, BaseGeom()->m_vPos, BaseGeom()->m_vCen);

        for (ZBaseGeom* pGeom = m_pGroupFirst; pGeom; pGeom = pGeom->Next())
        {
            vsub(pGeom->m_vPos, vDelta);
        }
    }

    void ZGROUP::MakeActiveRecursive()
    {
        for (auto* pBaseGeom = BaseGeom(); pBaseGeom; RecurGetNext(&pBaseGeom))
        {
            pBaseGeom->MakeActive();
        }
    }

    ZBaseGeom** ZGROUP::GetStaticLights(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd)
    {
        SGeomPairRecursion sGeomPairRecur {};
        sGeomPairRecur.InitPair(m_LightList);

        while (sGeomPairRecur.DpInsertList && sGeomPairRecur.m_cCur != sGeomPairRecur.m_cCurEnd)
        {
            auto* pPayload = reinterpret_cast<uintptr_t*>(
                reinterpret_cast<char*>(sGeomPairRecur.DpInsertList) + sizeof(SBaseGeomListHeader));

            const uintptr_t lFirstPacked = pPayload[sGeomPairRecur.m_cCur];
            const uintptr_t lLastPacked = pPayload[sGeomPairRecur.m_cCur + 1];
            sGeomPairRecur.m_cCur += 2;

            if ((lLastPacked & 7) == 2)
            {
                if (pDrawGeomsListEnd <= pDrawGeomsList)
                    return pDrawGeomsList;

                *pDrawGeomsList++ = reinterpret_cast<ZBaseGeom*>(lFirstPacked & ~7u);
                *pDrawGeomsList++ = reinterpret_cast<ZBaseGeom*>(lLastPacked & ~7u);
            }

            sGeomPairRecur.NextPair();
        }

        return pDrawGeomsList;
    }

    void ZGROUP::CalcCenSizeRecur()
    {
        for (ZBaseGeom* pGeom = m_pGroupFirst; pGeom; pGeom = pGeom->Next())
        {
            if (pGeom->IsDerivedFrom<ZGROUP>())
            {
                static_cast<ZGROUP*>(pGeom->GetGeom())->CalcCenSizeRecur();
            }
            else
            {
                pGeom->CalcCenSize(false);
            }
        }

        CalcCenSize();
    }

    void ZGROUP::GetCenSizeRecur(ZVector3& vCen, ZVector3& vSize, bool bIgnoreHidden)
    {
        ZVector3 vMaxAccum { -9.9999997e37f };
        ZVector3 vMinAccum { 9.9999997e37f };
        bool bFoundAny = false;

        for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
        {
            if (pBaseGeom->DisableParentBoundAdjust() || (bIgnoreHidden && (pBaseGeom->Control() & (ZCHIDDEN | ZCINACTIVE)) != 0))
                continue;

            ZVector3 vChildCen;
            ZVector3 vChildSize;
            bFoundAny = true;

            if (pBaseGeom->IsDerivedFrom<ZGROUP>())
            {
                static_cast<ZGROUP*>(pBaseGeom->GetGeom())->GetCenSizeRecur(vChildCen, vChildSize, bIgnoreHidden);
            }
            else
            {
                pBaseGeom->GetCen(vChildCen);
                pBaseGeom->GetSize(vChildSize);
            }

            TransformBox(pBaseGeom->m_mMat.data, vChildSize);
            TransformRootVector(vChildCen, pBaseGeom->m_mMat);
            vadd(vChildCen, pBaseGeom->m_vPos);

            ZVector3 vChildMax;
            vadd(vChildMax, vChildCen, vChildSize);
            vmax(vMaxAccum, vChildMax);

            ZVector3 vChildMin;
            vsub(vChildMin, vChildCen, vChildSize);
            vmin(vMinAccum, vChildMin);
        }

        if (bFoundAny)
        {
            vadd(vCen, vMaxAccum, vMinAccum);
            vscalar(vCen, 0.5f);
            vsub(vSize, vMaxAccum, vCen);
        }
        else
        {
            GetCen(vCen);
            GetSize(vSize);
        }
    }

    ZGEOM* ZGROUP::FindMaskGeom(const char* pSearchName, int32_t lMask) const
    {
        const char* pszSlash = strchr(pSearchName, '\\');
        const char* pszRemaining = pszSlash;
        if (!pszSlash)
        {
            pszSlash = pSearchName + strlen(pSearchName);
            pszRemaining = pszSlash;
        }

        const size_t lNameLen = static_cast<size_t>(pszSlash - pSearchName);

        auto* pGroupFirst = m_pGroupFirst;
        while (pGroupFirst)
        {
            if (!pGroupFirst->IsDerivedFrom<ZGROUP>())
                return nullptr;

            auto* pGroup = static_cast<ZGROUP*>(pGroupFirst->GetGeom());

            if ((lMask & static_cast<int32_t>(pGroup->m_lGroupCon)) != 0)
            {
                const char* pszName = pGroupFirst->Name();
                if (!pszName)
                    pszName = "<NONAME>";

                if (strlen(pszName) == lNameLen && !_memicmp(pszName, pSearchName, lNameLen))
                    break;
            }

            pGroupFirst = pGroupFirst->Next();
        }

        if (!pGroupFirst)
            return nullptr;

        auto* pGroup = static_cast<ZGROUP*>(pGroupFirst->GetGeom());

        if (*pszRemaining)
            return pGroup->FindGeom(pszRemaining + 1, nullptr);
        else
            return pGroup;
    }

    ZGEOM* ZGROUP::CreateResourceGeom(const char* pName, uint32_t iGeomResourceId, uint32_t lGeomClassType, bool bCalcMinMax)
    {
        if (g_pEngineData->ResourcesDisabled())
        {
            return CreateGeom(pName, lGeomClassType, bCalcMinMax);
        }

        auto* pResourceBaseGeom = ZGeomBuffer::Instance().AllocResourceGeom(pName, iGeomResourceId, lGeomClassType);
        if (!pResourceBaseGeom)
        {
            return nullptr;
        }

        AttachGeom(pResourceBaseGeom, bCalcMinMax);
        return pResourceBaseGeom->GetGeom();
    }

    ZGEOM* ZGROUP::CreateGeom(const char* pName, uint32_t iGeomClassId, bool bCalcMinMax)
    {
        auto* pGeom = ZGeomBuffer::Instance().AllocGeom(pName, iGeomClassId, nullptr);
        if (!pGeom)
            return nullptr;

        AttachGeom(pGeom, bCalcMinMax);
        return pGeom->GetGeom();
    }

    bool ZGROUP::IsRoot() const
    {
        return reinterpret_cast<const ZGROUP*>(g_pEngineData->m_pRoot) == this;
    }

    void ZGROUP::GroupContainsLight()
    {
        for (auto* pIt = this; pIt; pIt = pIt->BaseGeom()->ParentGroup())
        {
            pIt->m_lGroupCon |= ZGRPCF_GROUP_CONTAINS_LIGHT;
        }
    }

    void ZGROUP::GetLightShinesIn(bool& bLightShinesIn)
    {
        bLightShinesIn = (m_lGroupCon & ZGRPCF_LIGHT_SHINES_IN) != 0;
    }

    void ZGROUP::SetLightShinesIn(const bool& bLightShinesIn)
    {
        if (bLightShinesIn)
        {
            m_lGroupCon |= ZGRPCF_LIGHT_SHINES_IN;
        }
        else
        {
            m_lGroupCon &= ~ZGRPCF_LIGHT_SHINES_IN;
        }
    }

    void ZGROUP::GetLightShinesOut(bool& bLightShinesOut)
    {
        bLightShinesOut = (m_lGroupCon & ZGRPCF_LIGHT_SHINES_OUT) != 0;
    }

    void ZGROUP::SetLightShinesOut(const bool& bLightShinesOut)
    {
        if (bLightShinesOut)
        {
            m_lGroupCon |= ZGRPCF_LIGHT_SHINES_OUT;
        }
        else
        {
            m_lGroupCon &= ~ZGRPCF_LIGHT_SHINES_OUT;
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<bool> NamespaceItem_4180 {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "LightShinesOut",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZGROUP::GetLightShinesOut,
            .m_Set = &ZGROUP::SetLightShinesOut
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_4179 {
            .m_Node = {
                .m_Next = NamespaceItem_4180,
                .m_Name = "LightShinesIn",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZGROUP::GetLightShinesIn,
            .m_Set = &ZGROUP::SetLightShinesIn
        };

        struct RTP::ZDataProperty<float> NamespaceItem_4176 {
            .m_Node = {
                .m_Next = NamespaceItem_4179,
                .m_Name = "m_fPFResMultiplier",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__11,
            .m_Offset = CLASS_PROPERTY(ZGROUP, m_fPFResMultiplier)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZGROUP, // ClassName
        ZGEOM, // BaseClassName
        0x00972A28,  // OldClassInfo addr
        "ZGROUP", // FactoryName
        0x0076A998, // FactoryName Addr
        cProperties::NamespaceItem_4176, // FirstProperty
        0x00806C00, // Properties Addr
        0x00972984, // ID Addr
        0x00972988  // Mask Addr
    );
#   pragma endregion

    bool ForGroupsCheck(ZBaseGeom* pBaseGeom)
    {
        if (!pBaseGeom)
            return false;

        return pBaseGeom->IsDerivedFrom<ZGROUP>();
    }
}
