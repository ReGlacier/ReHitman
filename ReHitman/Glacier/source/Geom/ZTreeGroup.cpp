#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <Glacier/ZSTL/ZOctree.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/CHUNK.h>
#include <Glacier/Physics/ZOctreeCompiled.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZSTL/LINKREFTAB.h>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE(REFTAB*, g_pChildTreeGroupsList);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(REFTAB*, g_pChildTreeGroupsList, 0x0097B6F8, nullptr);

    struct STreeGetAll : public SRecurseInfoCompiled
    {
        // methods
        STreeGetAll() : SRecurseInfoCompiled()
        {
            pChkFunc = [](uint32_t lID, SOctreeChk* pDat) -> bool
            {
                static_cast<STreeGetAll*>(pDat)->Geoms->Add(lID);
                return true;
            };
        }

        // members
        REFTAB* Geoms { nullptr };
    };

    struct SGeomBoundCubeChk : public SRecurseInfoCompiled
    {
        // methods
        SGeomBoundCubeChk() : SRecurseInfoCompiled()
        {
            lNrGeomsInList = 0;
        }

        // members
        uint32_t GeomConOn;
        ZBaseGeom** pGeomList;
        ZBaseGeom** pGeomListEnd;
        uint32_t lNrGeomsInList;
    };

    ZTreeGroup::ZTreeGroup(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
    {
        m_pChildTreeGroupsLists = nullptr;
        m_pDynamicTree = nullptr;
        m_bPrivate = false;
        m_bStaticContainer = false;
        m_bDynamicContainer = false;
        InitGlobalBounds();
    }

    ZTreeGroup::~ZTreeGroup()
    {
        // Weird, it's duplicate of ZGROUP dtor
        while (m_pGroupFirst)
        {
            ZUniMemory::Delete(m_pGroupFirst);
        }

        if (auto* pParentGroup = BaseGeom()->ParentGroup(); pParentGroup && !IsPrivate())
        {
            if (auto* pTreeGroup = pParentGroup->GetTreeGroup())
            {
                pTreeGroup->RemoveChildTreeGroup(this);
            }
        }

        FreeGlobalBounds();

        if (m_pDynamicTree)
        {
            ZUniMemory::Delete(m_pDynamicTree);
            m_pDynamicTree = nullptr;
        }

        ZSAFE_ASSERT(!m_pChildTreeGroupsLists);
    }

    const RTP::ZPropertyInfo& ZTreeGroup::GetProperties() const
    {
        return ZTreeGroup::Info;
    }

    // ZGEOM
    uint32_t ZTreeGroup::GetObjectId() const
    {
        return ZTreeGroup::m_Id;
    }

    void ZTreeGroup::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZTreeGroup::m_Id;
        mask = ZTreeGroup::m_Mask;
    }

    ZGEOMCLASSINFO* ZTreeGroup::GetOldClassInfo() const
    {
        return ZTreeGroup::m_OldClassInfo;
    }

    void ZTreeGroup::ClassInit()
    {
        ZGROUP::ClassInit();

        if (auto* pDynTree = GetDynamicTreePtr())
        {
            REFTAB32 aCollected {};
            STreeGetAll sVisitor {};
            sVisitor.Geoms = &aCollected;

            pDynTree->GetEverything(&sVisitor);

            for (auto rGeom : aCollected.As<ZREF>())
            {
                auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(rGeom);
                if (pBaseGeom)
                {
                    pBaseGeom->DetachFromDynamicContainer(nullptr);
                    pBaseGeom->AttachToDynamicContainer();
                }
            }
        }
    }

    void ZTreeGroup::CopyData(const ZGEOM* Source)
    {
        Source->GetObjectId(); // Weird
        ZGROUP::CopyData(Source);
    }

    void ZTreeGroup::MakePrivate(bool bPrivate)
    {
        if (m_bPrivate == bPrivate)
            return;

        if (auto* pParentGroup = BaseGeom()->ParentGroup())
        {
            if (auto* pDynamicTreeGroup = pParentGroup->GetDynamicTreeGroup())
            {
                if (bPrivate)
                    pDynamicTreeGroup->DetachDynamicGeoms(BaseGeom());
                else
                    pDynamicTreeGroup->AttachDynamicGeoms(BaseGeom());
            }

            if (auto* pTreeGroup = pParentGroup->GetTreeGroup())
            {
                if (bPrivate)
                {
                    pTreeGroup->RemoveChildTreeGroup(this);
                    if (m_pChildTreeGroupsLists)
                    {
                        for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups; ++i)
                        {
                            pTreeGroup->RemoveStaticChildTreeGroup(m_pChildTreeGroupsLists->m_pStaticChildTreeGroups[i]);
                        }
                    }
                }
                else
                {
                    pTreeGroup->AddChildTreeGroup(this);
                    if (m_pChildTreeGroupsLists)
                    {
                        for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups; ++i)
                        {
                            pTreeGroup->AddStaticChildTreeGroup(m_pChildTreeGroupsLists->m_pStaticChildTreeGroups[i]);
                        }
                    }
                }
            }
        }

        m_bPrivate = bPrivate;
    }

    void ZTreeGroup::AddChildTreeGroup(ZTreeGroup* pTreeGroup)
    {
        if (pTreeGroup->IsStaticContainer())
            AddStaticChildTreeGroup(pTreeGroup);

        if (pTreeGroup->IsDynamicContainer())
            AddDynamicChildTreeGroup(pTreeGroup);
    }

    void ZTreeGroup::RemoveChildTreeGroup(ZTreeGroup* pTreeGroup)
    {
        if (pTreeGroup->IsStaticContainer())
            RemoveStaticChildTreeGroup(pTreeGroup);

        if (pTreeGroup->IsDynamicContainer())
            RemoveDynamicChildTreeGroup(pTreeGroup);
    }

    uint16_t ZTreeGroup::GetStaticChildTreeGroupList(ZTreeGroup*** pTreeGroupPtr)
    {
        *pTreeGroupPtr = nullptr;
        if (!m_pChildTreeGroupsLists)
            return 0u;

        *pTreeGroupPtr = m_pChildTreeGroupsLists->m_pStaticChildTreeGroups;
        return m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups;
    }

    uint16_t ZTreeGroup::GetDynamicChildTreeGroupList(ZTreeGroup*** pTreeGroupPtr)
    {
        *pTreeGroupPtr = nullptr;
        if (!m_pChildTreeGroupsLists)
            return 0u;

        *pTreeGroupPtr = m_pChildTreeGroupsLists->m_pDynamicChildTreeGroups;
        return m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups;
    }

    void ZTreeGroup::AddZGEOMBoxLst(eGlobalTreeType eGTT, REFTAB* pGeomRefTab, const ZMat3x3& mMat, const ZVector3& vCen, const ZVector3& vSize, int32_t GeomConMask, int32_t lGeomType, bool bCheckDynamic)
    {
        ZASSERT(this == ZROOT);

        ZBaseGeom* pGeomList[ZCollisionBase::MAX_GEOMS_NR];
        uint32_t lGeomCount = pGeomRefTab->Count();

        ZMat3x3 mLocalMat = mMat;
        ZVector3 vLocalCen = vCen;
        GetLocalMatPos(mLocalMat, vLocalCen);

        auto* pCollision = ZCollisionBase::GetCollisionInterface();
        const uint32_t lCount = pCollision->GetGeomsInBox(
            pGeomList,
            &pGeomList[ZCollisionBase::MAX_GEOMS_NR],
            eGTT,
            mLocalMat,
            vLocalCen,
            vSize,
            GeomConMask,
            lGeomType != 0,
            bCheckDynamic,
            true);

        lGeomCount = lCount;
        for (uint32_t i = 0; i < lGeomCount; ++i)
        {
            pGeomRefTab->Add(pGeomList[i]->GetRef());
        }
    }

    bool ZTreeGroup::ChkLineColi(COLI* pColi, eGlobalTreeType eGTT, int32_t GeomConMask, int32_t lGeomType, bool bCheckStatic, bool bCheckDynamic)
    {
        if (!Active())
            return false;

        ZASSERT(this == ZROOT);

        SExtendedImpactInfo sImpact {};
        sImpact.fPercent = pColi->t;
        sImpact.m_iColiMaterialDescId = 0;

        if (!ZCollisionBase::GetCollisionInterface()->CalcLineColi(
                &sImpact,
                eGTT,
                pColi->lp,
                pColi->ln,
                pColi->m_bBothSides,
                GeomConMask,
                bCheckStatic,
                bCheckDynamic))
        {
            return false;
        }

        pColi->ColiRef = sImpact.pBaseGeom ? sImpact.pBaseGeom->GetRef() : 0;
        pColi->t = sImpact.fPercent;
        pColi->m_HitCache = sImpact.m_HitCache;
        pColi->m_iColiMaterialDescId = sImpact.m_iColiMaterialDescId;
        pColi->m_BoneId = sImpact.m_BoneId;
        pColi->cp = sImpact.vPosition;
        pColi->vP1 = sImpact.vP1;
        pColi->vP2 = sImpact.vP2;
        pColi->vP3 = sImpact.vP3;
        return true;
    }

    bool ZTreeGroup::CheckDynamicLineColi(eGlobalTreeType eGTT, float*, float*, bool ,int , int)
    {
        return false;
    }

    bool ZTreeGroup::CalcDynamicLineColiRecur(SExtendedImpactInfo* Impact, eGlobalTreeType eGTT, const ZVector3& vP, const ZVector3& vD, bool bBothSides, uint32_t GeomConMask, ZGEOMCLASSINFO* pGeomClassInfo)
    {
        ZASSERT(m_bDynamicContainer);

        if (!Active())
            return false;

        bool bResult = false;
        auto* pTree = GetDynamicTreePtr();
        if (pTree)
        {
            ZVector3 vLocalP = vP;
            ZVector3 vLocalD = vD;
            GetLocalPoint(vLocalP);
            BaseGeom()->GetLocalVect(vLocalD);

            ZVector3 vEnd;
            vadd(vEnd, vLocalP, vLocalD);

            ZBaseGeom* pGeomList[ZCollisionBase::MAX_GEOMS_NR];
            STreeGetRightType sGetRightType {};
            sGetRightType.pGeomList = pGeomList;
            sGetRightType.pGeomListEnd = &pGeomList[ZCollisionBase::MAX_GEOMS_NR];
            sGetRightType.lGeomConMask = GeomConMask;

            pTree->CheckLinesegment(&sGetRightType, vLocalP, vEnd);
            for (uint32_t i = 0; i < sGetRightType.lNrGeomsInList; ++i)
            {
                bResult = ZCollisionBase::GetCollisionInterface()->CalcColiLort(
                    Impact,
                    pGeomList[i],
                    eGTT,
                    vP,
                    vD,
                    GeomConMask,
                    bBothSides) || bResult;
            }
        }

        if (m_pChildTreeGroupsLists)
        {
            for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups; ++i)
            {
                bResult = m_pChildTreeGroupsLists->m_pDynamicChildTreeGroups[i]->CalcDynamicLineColiRecur(
                    Impact,
                    eGTT,
                    vP,
                    vD,
                    bBothSides,
                    GeomConMask,
                    pGeomClassInfo) || bResult;
            }
        }

        return bResult;
    }

    bool ZTreeGroup::CalcDynamicLineColi(SExtendedImpactInfo* Impact, eGlobalTreeType eGTT, const ZVector3& vP, const ZVector3& vD, bool bBothSides, uint32_t GeomConMask, ZGEOMCLASSINFO* pGeomClassInfo)
    {
        ZASSERT(IsDynamicContainer());

        if (!Active())
            return false;

        auto* pTree = GetDynamicTreePtr();
        if (!pTree)
            return false;

        ZVector3 vEnd;
        vadd(vEnd, vP, vD);

        ZBaseGeom* pGeomList[ZCollisionBase::MAX_GEOMS_NR];
        STreeGetRightType sGetRightType {};
        sGetRightType.pGeomList = pGeomList;
        sGetRightType.pGeomListEnd = &pGeomList[ZCollisionBase::MAX_GEOMS_NR];
        sGetRightType.lGeomConMask = GeomConMask;

        pTree->CheckLinesegment(&sGetRightType, vP, vEnd);

        bool bResult = false;
        for (uint32_t i = 0; i < sGetRightType.lNrGeomsInList; ++i)
        {
            bResult = ZCollisionBase::GetCollisionInterface()->CalcColiLort(
                Impact,
                pGeomList[i],
                eGTT,
                vP,
                vD,
                GeomConMask,
                bBothSides) || bResult;
        }

        return bResult;
    }

    void ZTreeGroup::AttachDynamicGeoms(ZBaseGeom* pBaseGeom)
    {
        ZASSERT(IsDynamicContainer());

        if (pBaseGeom->IsDerivedFrom<ZGROUP>())
        {
            auto* pGroup = static_cast<ZGROUP*>(pBaseGeom->GetGeom());
            bool bRecurse = true;
            if (pBaseGeom->IsDerivedFrom<ZTreeGroup>())
            {
                auto* pTreeGroup = static_cast<ZTreeGroup*>(pBaseGeom->GetGeom());
                if (pTreeGroup->IsPrivate())
                    return;

                if (pTreeGroup->IsDynamicContainer() && pBaseGeom != BaseGeom())
                    bRecurse = false;
            }

            if (bRecurse)
            {
                for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                {
                    AttachDynamicGeoms(pChild);
                }
            }
        }

        if (pBaseGeom != BaseGeom() && (pBaseGeom->Control() & ZCDYNAMIC) != 0 && (pBaseGeom->Control() & 0x200C00u) == 0)
        {
            pBaseGeom->AttachToDynamicContainer();
        }
    }

    void ZTreeGroup::DetachDynamicGeoms(ZBaseGeom* pBaseGeom)
    {
        ZASSERT(IsDynamicContainer());

        if (pBaseGeom->IsDerivedFrom<ZGROUP>())
        {
            auto* pGroup = static_cast<ZGROUP*>(pBaseGeom->GetGeom());
            bool bRecurse = true;
            if (pBaseGeom->IsDerivedFrom<ZTreeGroup>())
            {
                auto* pTreeGroup = static_cast<ZTreeGroup*>(pBaseGeom->GetGeom());
                if (pTreeGroup->IsPrivate())
                    return;

                if (pTreeGroup->IsDynamicContainer() && pBaseGeom != BaseGeom())
                    bRecurse = false;
            }

            if (bRecurse)
            {
                for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                {
                    DetachDynamicGeoms(pChild);
                }
            }
        }

        if (pBaseGeom != BaseGeom() && (pBaseGeom->Control() & ZCDYNAMIC) != 0)
        {
            pBaseGeom->DetachFromDynamicContainer(nullptr);
        }
    }

    void ZTreeGroup::AttachTreeGroupChilds(ZBaseGeom* pBaseGeom)
    {
        if (!pBaseGeom->IsDerivedFrom<ZGROUP>())
            return;

        auto* pTreeGroup = GetTreeGroup();
        if (!pTreeGroup)
            return;

        if (pBaseGeom->IsDerivedFrom<ZTreeGroup>())
        {
            auto* pChildTreeGroup = static_cast<ZTreeGroup*>(pBaseGeom->GetGeom());
            if (!pChildTreeGroup->IsPrivate())
                pTreeGroup->AddChildTreeGroup(pChildTreeGroup);
            return;
        }

        auto* pGroup = static_cast<ZGROUP*>(pBaseGeom->GetGeom());
        for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
        {
            AttachTreeGroupChilds(pChild);
        }
    }

    void ZTreeGroup::DetachTreeGroupChilds(ZBaseGeom* pBaseGeom)
    {
        if (!pBaseGeom->IsDerivedFrom<ZGROUP>())
            return;

        auto* pTreeGroup = GetTreeGroup();
        if (!pTreeGroup)
            return;

        if (pBaseGeom->IsDerivedFrom<ZTreeGroup>())
        {
            auto* pChildTreeGroup = static_cast<ZTreeGroup*>(pBaseGeom->GetGeom());
            if (!pChildTreeGroup->IsPrivate())
                pTreeGroup->RemoveChildTreeGroup(pChildTreeGroup);
            return;
        }

        auto* pGroup = static_cast<ZGROUP*>(pBaseGeom->GetGeom());
        for (auto* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
        {
            DetachTreeGroupChilds(pChild);
        }
    }

    void ZTreeGroup::MakeStaticContainer(bool bStaticContainer)
    {
        if (m_bStaticContainer == bStaticContainer)
            return;

        if (bStaticContainer)
        {
            m_bStaticContainer = true;
            if (!IsPrivate())
            {
                if (auto* pParentGroup = BaseGeom()->ParentGroup())
                {
                    if (auto* pTreeGroup = pParentGroup->GetTreeGroup())
                    {
                        pTreeGroup->AddStaticChildTreeGroup(this);
                        if (m_pChildTreeGroupsLists)
                        {
                            for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups; ++i)
                            {
                                pTreeGroup->RemoveStaticChildTreeGroup(m_pChildTreeGroupsLists->m_pStaticChildTreeGroups[i]);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            FreeGlobalBounds();
            m_bStaticContainer = false;
            if (!IsPrivate())
            {
                if (auto* pParentGroup = BaseGeom()->ParentGroup())
                {
                    if (auto* pTreeGroup = pParentGroup->GetTreeGroup())
                    {
                        pTreeGroup->RemoveStaticChildTreeGroup(this);
                        if (m_pChildTreeGroupsLists)
                        {
                            for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups; ++i)
                            {
                                pTreeGroup->AddStaticChildTreeGroup(m_pChildTreeGroupsLists->m_pStaticChildTreeGroups[i]);
                            }
                        }
                    }
                }
            }
        }
    }

    void ZTreeGroup::MakeDynamicContainer(bool bDynamicContainer)
    {
        if (this != ZROOT || m_bDynamicContainer == bDynamicContainer)
            return;

        if (bDynamicContainer)
        {
            m_bDynamicContainer = true;
            if (!IsPrivate())
            {
                if (auto* pParentGroup = BaseGeom()->ParentGroup())
                {
                    if (auto* pTreeGroup = pParentGroup->GetTreeGroup())
                    {
                        pTreeGroup->AddDynamicChildTreeGroup(this);
                        if (m_pChildTreeGroupsLists)
                        {
                            for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups; ++i)
                            {
                                pTreeGroup->RemoveDynamicChildTreeGroup(m_pChildTreeGroupsLists->m_pDynamicChildTreeGroups[i]);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            m_bDynamicContainer = false;
            FreeDynamicTrees();
            if (!IsPrivate())
            {
                if (auto* pParentGroup = BaseGeom()->ParentGroup())
                {
                    if (auto* pTreeGroup = pParentGroup->GetTreeGroup())
                    {
                        pTreeGroup->RemoveDynamicChildTreeGroup(this);
                        if (m_pChildTreeGroupsLists)
                        {
                            for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups; ++i)
                            {
                                pTreeGroup->AddDynamicChildTreeGroup(m_pChildTreeGroupsLists->m_pDynamicChildTreeGroups[i]);
                            }
                        }
                    }
                }
            }
        }
    }

    bool ZTreeGroup::IsStaticContainer() const
    {
        return m_bStaticContainer;
    }

    bool ZTreeGroup::IsDynamicContainer() const
    {
        return m_bDynamicContainer;
    }

    bool ZTreeGroup::IsPrivate() const
    {
        return m_bPrivate;
    }

    CQuadtree* ZTreeGroup::GetDynamicTreePtr() const
    {
        return m_pDynamicTree;
    }

    CQuadtreeObj* ZTreeGroup::AddDynamicGeom(ZBaseGeom* pDynBaseGeom)
    {
        ZASSERT(m_bDynamicContainer);
        ZASSERT(pDynBaseGeom->Control() & ZCCOLIMASK);

        auto* pTree = GetDynamicTreePtr();
        if (!pTree)
            return nullptr;

        ZMat3x3 mMat;
        mreset(mMat);

        ZVector3 vPos, vSize, vMin, vMax;
        pDynBaseGeom->GetCen(vPos);
        pDynBaseGeom->GetRootMatPos(mMat, vPos);
        pDynBaseGeom->GetSize(vSize);
        GetLocalMatPos(mMat, vPos);
        TransformBox(mMat, vSize);

        vMin = vPos - vSize;
        vMax = vPos + vSize;

        int vMinOCS[3] {};
        int vMaxOCS[3] {};
        pTree->ConvToOCS(vMinOCS, vMin);
        pTree->ConvToOCS(vMaxOCS, vMax);

        auto* pObject = pTree->CreateObject(pDynBaseGeom->GetRef());
        pObject->m_tInfo.iMinX = static_cast<uint16_t>(vMinOCS[0]);
        pObject->m_tInfo.iMinY = static_cast<uint16_t>(vMinOCS[1]);
        pObject->m_tInfo.iMinZ = static_cast<uint16_t>(vMinOCS[2]);
        pObject->m_tInfo.iMaxX = static_cast<uint16_t>(vMaxOCS[0]);
        pObject->m_tInfo.iMaxY = static_cast<uint16_t>(vMaxOCS[1]);
        pObject->m_tInfo.iMaxZ = static_cast<uint16_t>(vMaxOCS[2]);

        const int iSizeX = vMaxOCS[0] - vMinOCS[0] + 1;
        const int iSizeY = vMaxOCS[1] - vMinOCS[1] + 1;
        const int iSizeZ = vMaxOCS[2] - vMinOCS[2] + 1;

        int iMaxDim = iSizeZ;
        if (iSizeY >= iSizeX)
        {
            if (iSizeY > iMaxDim)
                iMaxDim = iSizeY;
        }
        else if (iSizeX > iMaxDim)
        {
            iMaxDim = iSizeX;
        }

        SRecurseAdd sAddInfo {};
        sAddInfo.pPool = &pTree->m_tPool;
        sAddInfo.pObject = pObject;
        sAddInfo.iCurDepth = 0;
        sAddInfo.iWantedDepth = pTree->GetDepth(iMaxDim);
        sAddInfo.iX = vMinOCS[0] + (iSizeX >> 1);
        sAddInfo.iY = vMinOCS[1] + (iSizeY >> 1);
        sAddInfo.iZ = vMinOCS[2] + (iSizeZ >> 1);

        pTree->m_pRoot->Add(&sAddInfo);
        ++pTree->m_iObjects;

        return pObject;
    }

    void ZTreeGroup::MoveDynamicGeom(ZBaseGeom* pDynBaseGeom)
    {
        ZASSERT(pDynBaseGeom->Control() & ZCDYNAMIC);
        ZASSERT(pDynBaseGeom->Control() & ZCCOLIMASK);

        if (pDynBaseGeom->m_pDynId)
        {
            auto* pTree = GetDynamicTreePtr();
            ZASSERT(pTree);

            ZMat3x3 mMat;
            mreset(mMat);

            ZVector3 vPos, vSize, vMin, vMax;
            pDynBaseGeom->GetCen(vPos);
            pDynBaseGeom->GetRootMatPos(mMat, vPos);
            pDynBaseGeom->GetSize(vSize);
            GetLocalMatPos(mMat, vPos);
            TransformBox(mMat, vSize);

            vMin = vPos - vSize;
            vMax = vPos + vSize;
            pTree->Move(pDynBaseGeom->m_pDynId, vMin, vMax);
        }
    }

    bool ZTreeGroup::RemoveDynamicGeom(ZBaseGeom* pDynBaseGeom)
    {
        ZASSERT(m_bDynamicContainer);
        ZASSERT(pDynBaseGeom->Control() & ZCCOLIMASK);

        if (pDynBaseGeom->m_pDynId)
        {
            auto* pTree = GetDynamicTreePtr();
            ZASSERT(pTree);

            auto* pObj = pDynBaseGeom->m_pDynId;

            // In PC build we have some additional code, but it looks like unrolled/inlined CQuadtree::Delete parts
            // Anyway, in bug case - fix me

            pTree->Delete(pObj);
            ZUniMemory::Delete(pObj);

            return true;
        }

        return false;
    }

    void ZTreeGroup::CreateDynamicTrees()
    {
        ZASSERT(m_bDynamicContainer);
        ZASSERT(!m_pDynamicTree);

        m_pDynamicTree = ZUniMemory::New<CQuadtree>(0x2800);

        ZVector3 vOrigin;
        float fSize = GetOriginAndSize(vOrigin);

        if (this == ZROOT)
        {
            if (fSize < 5000.0f)
            {
                fSize = 5000.0f;
            }
        }
        else if (fSize < 1000.0f)
        {
            fSize = 1000.0f;
        }

        m_pDynamicTree->SetOrigin(vOrigin);
        const float fScale = 32000.0f / fSize;
        m_pDynamicTree->SetScale(fScale);

        AttachDynamicGeoms(BaseGeom());

        if (m_pChildTreeGroupsLists)
        {
            for (int i = 0; i < m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups; ++i)
            {
                m_pChildTreeGroupsLists->m_pDynamicChildTreeGroups[i]->CreateDynamicTrees();
            }
        }
    }

    uint32_t ZTreeGroup::SaveBoundTrees(CHUNK* pChunk, uint32_t lTreeOffset)
    {
        uint32_t lSize = lTreeOffset;

        for (auto& pGT : m_pGlobalTrees)
        {
            if (pGT)
            {
                int iTreeDataSize;
                void* pTreeData = pGT->Compile(&iTreeDataSize);

                ZOctreeCompiled sOct;
                sOct.Init(pTreeData);
                sOct.RemapObjects([](uint32_t lObj) -> uint32_t { return lObj; });

                pChunk->AddData(&iTreeDataSize, sizeof(int));

                const float fScale = pGT->GetScale();
                ZVector3 vOrigin;
                pGT->GetOrigin(vOrigin);

                pChunk->AddData(&fScale, sizeof(float));
                pChunk->AddData(&vOrigin, sizeof(ZVector3));

                const ZVector3 vZero {};
                pChunk->AddData(&vZero, sizeof(ZVector3));

                pChunk->AddData(pTreeData, iTreeDataSize);
                pChunk->AddData(&vZero, (-iTreeDataSize) & 0xF);

                operator delete(pTreeData);

                lSize += 0x20 + iTreeDataSize + ((-iTreeDataSize) & 0xF);
            }
            else
            {
                const int iZero = 0;
                pChunk->AddData(&iZero, sizeof(int));

                const ZVector3 vZero {};
                pChunk->AddData(&vZero, sizeof(ZVector3));

                lSize += 0x10;
            }
        }

        return lSize;
    }

    char* ZTreeGroup::LoadBoundTrees(char* pOctreeBuffer)
    {
        struct SOctreeHeader
        {
            uint32_t lOffset;
            float fScale;
            ZVector3 vOrigin;
        };
        RE_VERIFY_SIZE(SOctreeHeader, 20);
        RE_VERIFY_OFFSET(SOctreeHeader, lOffset, 0x0);
        RE_VERIFY_OFFSET(SOctreeHeader, fScale, 0x4);
        RE_VERIFY_OFFSET(SOctreeHeader, vOrigin, 0x8);

        FreeGlobalBounds();

        int32_t lGTIndex = GLOBAL_TREES_NR;
        ZOctree** pCurrentOctree = m_pGlobalTrees;

        do
        {
            const auto* pHeader = reinterpret_cast<const SOctreeHeader*>(pOctreeBuffer);
            const int32_t lOffset = pHeader->lOffset;
            if (lOffset)
            {
                float fScale = pHeader->fScale;
                ZVector3 vOrigin = pHeader->vOrigin;

                char* pTreeData = pOctreeBuffer + sizeof(SOctreeHeader);

                ZOctreeCompiled* pOctree = ZUniMemory::New<ZOctreeCompiled>();
                pOctree->Init(pTreeData);

                ZASSERT(fScale > 0.0f);
                pOctree->m_fScale = fScale;
                pOctree->SetOrigin(vOrigin);

                *pCurrentOctree = pOctree;
                pOctreeBuffer = pTreeData + ((lOffset + 15) & ~15);
                pOctree->RemapObjects([](uint32_t lID) -> uint32_t { return lID; });
            }
            else
            {
                *pCurrentOctree = nullptr;
                pOctreeBuffer += 0x10;
            }

            ++pCurrentOctree;
            --lGTIndex;
        } while (lGTIndex);
        return pOctreeBuffer;
    }

    void ZTreeGroup::FreeGlobalBounds()
    {
        for (int i = GLOBAL_TREES_NR - 1; i != -1; --i)
        {
            if (m_pGlobalTrees[i])
            {
                ZUniMemory::Delete(m_pGlobalTrees[i]);
                m_pGlobalTrees[i] = nullptr;
            }
        }
    }

    void ZTreeGroup::FreeDynamicTrees()
    {
        if (m_pDynamicTree)
        {
            ZUniMemory::Delete(m_pDynamicTree);
            m_pDynamicTree = nullptr;
        }
    }

    void ZTreeGroup::RemoveGlobalBound(eGlobalTreeType eGTT, ZBaseGeom* pBaseGeom, COctreeObj* pMMLink)
    {
        ZASSERT(pBaseGeom->Control() & ZCDYNAMIC);
        ZASSERT(m_pGlobalTrees[eGTT]);

        m_pGlobalTrees[eGTT]->Delete(pMMLink, false);
    }

    void ZTreeGroup::MoveGlobalBound(eGlobalTreeType eGTT, ZBaseGeom* pBaseGeom, COctreeObj* pMMLink, ZVector3& vMin, ZVector3& vMax)
    {
        ZASSERT(pBaseGeom->Control() & ZCDYNAMIC);
        ZASSERT(m_pGlobalTrees[eGTT]);

        m_pGlobalTrees[eGTT]->Move(pMMLink, vMin, vMax); // it will assert and it's ok lol
    }

    ZOctree* ZTreeGroup::GetGlobalTreePtr(eGlobalTreeType eGTT)
    {
        ZASSERT(eGTT!=GT_None);
        return m_pGlobalTrees[eGTT];
    }

    uint32_t ZTreeGroup::GetStaticGeomsInBound(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, const ZMat3x3& mMat, const ZVector3& vCen, const ZVector3& vSize, uint32_t lGeomConMask, bool bExact)
    {
        auto* pOctree = GetGlobalTreePtr(eGTT);
        if (!pOctree)
            return 0;

        SGeomBoundCubeChk sVisitor {};
        sVisitor.GeomConOn = lGeomConMask;
        sVisitor.pGeomList = pGeomList;
        sVisitor.pGeomListEnd = pGeomListEnd;

        sVisitor.pChkFunc = [](uint32_t lID, SOctreeChk* pDat) -> bool {
            auto* pData = static_cast<SGeomBoundCubeChk*>(pDat);
            auto* pGeomBase = ZGeomBuffer::Instance().GeomRefToBasePtr(lID);
            if (!pGeomBase)
                return false;

            auto lColiBits = pGeomBase->GetColiBits();
            if (((lColiBits | pGeomBase->Control()) & pData->GeomConOn) == 0 || (pGeomBase->Control() & 0x40040C00u) != 0)
            {
                return false;
            }

            auto* pInsertAt = &pData->pGeomList[pData->lNrGeomsInList];
            if (pInsertAt < pData->pGeomListEnd)
            {
                *pInsertAt = pGeomBase;
                ++pData->lNrGeomsInList;
            }

            return true;
        };

        ZMat3x3 mMat2 = mMat;
        ZVector3 vSize2 = vSize, vA, vB;

        TransformBox(mMat2, vSize2);
        vsub(vA, vCen, vSize2);
        vmmul(vB, vCen, vSize2);

        pOctree->CheckCube(&sVisitor, vA, vB);

        if (bExact && sVisitor.lNrGeomsInList)
        {
            ZVector3 vLocalCen = vCen;
            ZMat3x3 mLocalMat = mMat;
            GetLocalMatPos(mLocalMat, vLocalCen);

            uint32_t lCount = sVisitor.lNrGeomsInList;
            for (uint32_t i = 0; i < lCount; ++i)
            {
                auto* pBaseGeom = pGeomList[i];
                ZVector3 vGeomCen;
                ZMat3x3 mGeomMat;
                pBaseGeom->GetCen(vGeomCen);
                mreset(mGeomMat);
                pBaseGeom->GetRootMatPos(mGeomMat, vGeomCen);
                if (!rectBoxColi(mLocalMat, vLocalCen, vSize, mGeomMat, vGeomCen, pBaseGeom->Size()))
                {
                    pGeomList[i--] = pGeomList[--lCount];
                }
            }

            return lCount;
        }

        return sVisitor.lNrGeomsInList;
    }

    void ZTreeGroup::SetDynamicContainer(const bool& dynamic_container)
    {
        MakeDynamicContainer(dynamic_container);
    }

    void ZTreeGroup::SetStaticContainer(const bool& static_container)
    {
        bool bIsStatic = false;

        if (IsDerivedFrom<ZROOM>() || static_container)
            bIsStatic = true;

        MakeStaticContainer(bIsStatic);
    }

    void ZTreeGroup::GetDynamicContainer(bool& bIsDynamic)
    {
        bIsDynamic = m_bDynamicContainer;
    }

    void ZTreeGroup::Proxy_SetDynamicContainer(const bool& bIsDynamic)
    {
        SetDynamicContainer(bIsDynamic);
    }

    void ZTreeGroup::GetIsPrivateContainer(bool& bIsPrivate)
    {
        bIsPrivate = m_bPrivate;
    }

    void ZTreeGroup::SetIsPrivateContainer(const bool& bIsPrivate)
    {
        MakePrivate(bIsPrivate);
    }

    void ZTreeGroup::IsStaticContainer(bool& bIsStatic)
    {
        bIsStatic = m_bStaticContainer;
    }

    void ZTreeGroup::SetIsStaticContainer(const bool& bIsStatic)
    {
        SetStaticContainer(bIsStatic);
    }

    float ZTreeGroup::GetOriginAndSize(ZVector3& vCen)
    {
        GetCen(vCen);

        ZVector3 vSize;
        GetSize(vSize);

        ZVector3 vMin = vCen - vSize;
        ZVector3 vMax = vCen + vSize;

        for (auto* pBaseGeom = m_pGroupFirst; pBaseGeom; RecurGetNext(&pBaseGeom))
        {
            if (pBaseGeom->IsDerivedFrom<ZLIGHT>() || (pBaseGeom->Control() & ZCDYNAMIC) != 0)
            {
                ZVector3 vGeomCen, vGeomSize, vGeomMin, vGeomMax;
                ZMat3x3 mGeomMat;

                pBaseGeom->GetCen(vGeomCen);
                pBaseGeom->GetSize(vGeomSize);
                mreset(mGeomMat);
                pBaseGeom->GetRootMatPos(mGeomMat, vGeomCen);

                if ((pBaseGeom->Control() & ZCDYNAMIC) != 0)
                {
                    if (auto* pGeom = pBaseGeom->GetGeom())
                    {
                        pGeom->ExpandBounds(mGeomMat, vGeomCen, vGeomSize, nullptr);
                    }
                }

                GetLocalMatPos(mGeomMat, vGeomCen);
                TransformBox(mGeomMat, vGeomSize);

                vGeomMin = vGeomCen - vGeomSize;
                vGeomMax = vGeomCen + vGeomSize;
                vmin(vMin, vGeomMin);
                vmax(vMax, vGeomMax);
            }
        }

        const ZVector3 vPad { 0.1f, 0.1f, 0.1f };
        vMin -= vPad;
        vMax += vPad;

        vSize = vMax - vMin;
        vscalar(vSize, 0.5f);
        vCen = vMin + vSize;

        float fSize = vSize.x;
        if (fSize < vSize.y)
            fSize = vSize.y;
        if (fSize < vSize.z)
            fSize = vSize.z;

        return fSize;
    }

    void ZTreeGroup::AddStaticChildTreeGroup(ZTreeGroup* pTreeGroup)
    {
        if (!m_pChildTreeGroupsLists)
        {
            if (!g_pChildTreeGroupsList)
            {
                g_pChildTreeGroupsList = ZUniMemory::New<LINKREFTAB>(1, 416);
            }

            m_pChildTreeGroupsLists = reinterpret_cast<SChildTreeGroups*>(
                reinterpret_cast<char*>(g_pChildTreeGroupsList->Add(0)) - 4);
            memset(m_pChildTreeGroupsLists, 0, sizeof(SChildTreeGroups));
        }

        ZASSERT(m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups < 0x180u);
        m_pChildTreeGroupsLists->m_pStaticChildTreeGroups[
            m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups++] = pTreeGroup;

        if (!IsPrivate() && !IsStaticContainer() && BaseGeom()->ParentGroup())
        {
            auto* pGeom = static_cast<ZGEOM*>(BaseGeom()->ParentGroup());
            if (auto* pTreeGroup2 = pGeom->GetTreeGroup())
            {
                pTreeGroup2->AddStaticChildTreeGroup(pTreeGroup);
            }
        }
    }

    void ZTreeGroup::RemoveStaticChildTreeGroup(ZTreeGroup* pTreeGroup)
    {
        auto* pCurrent = this;
        for (;;)
        {
            ZASSERT(pCurrent->m_pChildTreeGroupsLists);
            ZASSERT(g_pChildTreeGroupsList);
            ZASSERT(g_pChildTreeGroupsList->Exists(
                reinterpret_cast<uint32_t*>(pCurrent->m_pChildTreeGroupsLists)));
            ZASSERT(pCurrent->m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups > 0);

            uint16_t iCount = pCurrent->m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups;
            uint16_t iIndex = 0;
            ZTreeGroup** ppArray = pCurrent->m_pChildTreeGroupsLists->m_pStaticChildTreeGroups;
            while (iIndex < iCount && *ppArray != pTreeGroup)
            {
                ++iIndex;
                ++ppArray;
            }
            ZASSERT(iIndex < iCount);

            --pCurrent->m_pChildTreeGroupsLists->m_iNrStaticChildTreeGroups;
            auto* pLists = pCurrent->m_pChildTreeGroupsLists;
            if (pLists->m_iNrStaticChildTreeGroups > 0)
            {
                pLists->m_pStaticChildTreeGroups[iIndex] =
                    pLists->m_pStaticChildTreeGroups[pLists->m_iNrStaticChildTreeGroups];
            }
            else if (pLists->m_iNrDynamicChildTreeGroups == 0)
            {
                g_pChildTreeGroupsList->DelRefPtr(
                    reinterpret_cast<uint32_t*>(pCurrent->m_pChildTreeGroupsLists));
                pCurrent->m_pChildTreeGroupsLists = nullptr;

                if (!g_pChildTreeGroupsList->Count())
                {
                    ZUniMemory::Delete(g_pChildTreeGroupsList);
                    g_pChildTreeGroupsList = nullptr;
                }
            }

            if (pCurrent->IsPrivate() || pCurrent->IsStaticContainer() ||
                !pCurrent->BaseGeom()->ParentGroup())
                break;

            auto* pGeom = static_cast<ZGEOM*>(pCurrent->BaseGeom()->ParentGroup());
            auto* pTreeGroup2 = pGeom->GetTreeGroup();
            if (!pTreeGroup2)
                break;

            pCurrent = pTreeGroup2;
        }
    }

    void ZTreeGroup::AddDynamicChildTreeGroup(ZTreeGroup* pTreeGroup)
    {
        if (!m_pChildTreeGroupsLists)
        {
            if (!g_pChildTreeGroupsList)
            {
                g_pChildTreeGroupsList = ZUniMemory::New<LINKREFTAB>(1, 416);
            }

            m_pChildTreeGroupsLists = reinterpret_cast<SChildTreeGroups*>(
                reinterpret_cast<char*>(g_pChildTreeGroupsList->Add(0)) - 4);
            memset(m_pChildTreeGroupsLists, 0, sizeof(SChildTreeGroups));
        }

        ZASSERT(m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups < 0x20u);
        m_pChildTreeGroupsLists->m_pDynamicChildTreeGroups[
            m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups++] = pTreeGroup;

        if (!IsPrivate() && !IsDynamicContainer() && BaseGeom()->ParentGroup())
        {
            auto* pGeom = static_cast<ZGEOM*>(BaseGeom()->ParentGroup());
            if (auto* pTreeGroup2 = pGeom->GetTreeGroup())
            {
                pTreeGroup2->AddDynamicChildTreeGroup(pTreeGroup);
            }
        }
    }

    void ZTreeGroup::RemoveDynamicChildTreeGroup(ZTreeGroup* pTreeGroup)
    {
        auto* pCurrent = this;
        for (;;)
        {
            ZASSERT(pCurrent->m_pChildTreeGroupsLists);
            ZASSERT(g_pChildTreeGroupsList);
            ZASSERT(g_pChildTreeGroupsList->Exists(
                reinterpret_cast<uint32_t*>(pCurrent->m_pChildTreeGroupsLists)));
            ZASSERT(pCurrent->m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups > 0);

            uint16_t iCount = pCurrent->m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups;
            uint16_t iIndex = 0;
            ZTreeGroup** ppArray = pCurrent->m_pChildTreeGroupsLists->m_pDynamicChildTreeGroups;
            while (iIndex < iCount && *ppArray != pTreeGroup)
            {
                ++iIndex;
                ++ppArray;
            }
            ZASSERT(iIndex < iCount);

            --pCurrent->m_pChildTreeGroupsLists->m_iNrDynamicChildTreeGroups;
            auto* pLists = pCurrent->m_pChildTreeGroupsLists;
            if (pLists->m_iNrDynamicChildTreeGroups > 0)
            {
                pLists->m_pDynamicChildTreeGroups[iIndex] =
                    pLists->m_pDynamicChildTreeGroups[pLists->m_iNrDynamicChildTreeGroups];
            }
            else if (pLists->m_iNrStaticChildTreeGroups == 0)
            {
                g_pChildTreeGroupsList->DelRefPtr(
                    reinterpret_cast<uint32_t*>(pCurrent->m_pChildTreeGroupsLists));
                pCurrent->m_pChildTreeGroupsLists = nullptr;

                if (!g_pChildTreeGroupsList->Count())
                {
                    ZUniMemory::Delete(g_pChildTreeGroupsList);
                    g_pChildTreeGroupsList = nullptr;
                }
            }

            if (pCurrent->IsPrivate() || pCurrent->IsDynamicContainer() ||
                !pCurrent->BaseGeom()->ParentGroup())
                break;

            auto* pGeom = static_cast<ZGEOM*>(pCurrent->BaseGeom()->ParentGroup());
            auto* pTreeGroup2 = pGeom->GetTreeGroup();
            if (!pTreeGroup2)
                break;

            pCurrent = pTreeGroup2;
        }
    }

    void ZTreeGroup::InitGlobalBounds()
    {
        m_pGlobalTrees[0] = nullptr;
        m_pGlobalTrees[1] = nullptr;
        m_pGlobalTrees[2] = nullptr;
        m_pGlobalTrees[3] = nullptr;
        m_pGlobalTrees[4] = nullptr;
    }


#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<bool> NamespaceItem_2711 {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "StaticContainer",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZTreeGroup::IsStaticContainer,
            .m_Set = &ZTreeGroup::SetIsStaticContainer
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_2710 {
            .m_Node = {
                .m_Next = NamespaceItem_2711,
                .m_Name = "Private",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZTreeGroup::GetIsPrivateContainer,
            .m_Set = &ZTreeGroup::SetIsPrivateContainer
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_2709 {
            .m_Node = {
                .m_Next = NamespaceItem_2710,
                .m_Name = "DynamicContainer",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZTreeGroup::GetDynamicContainer,
            .m_Set = &ZTreeGroup::Proxy_SetDynamicContainer
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZTreeGroup, // ClassName
        ZGROUP,     // BaseClassName
        0x0097B748, // OldClassInfo addr
        "ZTreeGroup", // FactoryName
        0x0076E9A0, // FactoryName Addr
        cProperties::NamespaceItem_2709, // FirstProperty
        0x0080E350, // Properties Addr
        0x0097B6F0, // ID Addr
        0x0097B6F4  // Mask Addr
    );
#   pragma endregion
}
