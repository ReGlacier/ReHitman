#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/Macro.h>

#include <G1ConfigurationService.h>

#include <cassert>
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
        // TODO: Finish me
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
                if (!pName)
                {
                    pName = "<NONAME>";
                }

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

    float ZGROUP::GetPointInsideDistance(const ZVector3& vPos)
    {
        if (m_pZBounds)
        {
            // TODO: Finish me
        }
        else
        {
            ZVector3 vP;
            vsub(vP, m_vCenInsideCheck, vPos);
            //return ZCommonAlgorithms::DistanceBoxAndPoint(vP, )
        }
        // TODO: Finish me
        return 0.0f;
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
        // TODO: Finish me
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

    void ZGROUP::AttachGeom(ZBaseGeom* pBaseGeom, bool bCalcMinMax)
    {
        // TODO: Finish me
    }

    void ZGROUP::AttachGeom(ZGEOM* pGeom, bool bCalcMinMax)
    {
        // TODO: Finish me
    }

    void ZGROUP::DetachGeom(ZBaseGeom* pBaseGeom, bool bCalcMinMax)
    {
        // TODO: Finish me
    }

    void ZGROUP::RecurGetNextGroup(const ZBaseGeom** pGroup) const
    {
        // TODO: Finish me
    }

    void ZGROUP::RecurGetNextExclRoom(const ZBaseGeom** ZGeom) const
    {
        // TODO: Finish me
    }

    void ZGROUP::SetGroupControl(uint32_t lAddBits, uint32_t lRemBits)
    {
        // TODO: Finish me
    }

    uint32_t ZGROUP::GroupControl() const
    {
        // TODO: Finish me
        return 0;
    }

    void ZGROUP::ResetGroupPosition(bool bReset)
    {
        // TODO: Finish me
    }

    void ZGROUP::MakeActiveRecursive()
    {
        // TODO: Finish me
    }

    void ZGROUP::GetStaticLights(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd)
    {
        // TODO: Finish me
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
        // TODO: Finish me
    }

    ZGEOM* ZGROUP::FindMaskGeom(const char* pSearchName, int32_t lMask) const
    {
        // TODO: Finish me
        return nullptr;
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

    STATIC_CLASS_VAR_IMPL(ZGROUP, RTP::ZPropertyInfo, Info, 0x00806C00, (RTP::ZPropertyInfo {
        .First = cProperties::NamespaceItem_4176,
        .Super = &ZGEOM::Info,
        .Name = ZGROUP::FactoryName
    }));
    STATIC_CLASS_VAR_IMPL(ZGROUP, const char*, FactoryName, 0x0076A998, "ZGROUP");
    DECLARE_ID_AND_MASK_IMPL(ZGROUP, 0x00972984, 0x00972988);
    REGISTER_GLACIER_GEOM_CLASS(ZGROUP, ZGEOM, ZGROUP::m_TypeId, 0x00972A28);
#   pragma endregion
}
