#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/SPrimStrips.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/SRMaterialProperties.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <Glacier/ZSTL/ZOctree.h>
#include <Glacier/ZSTL/CHUNK.h>
#include <Glacier/ZEngineDataBase.h>


namespace Glacier
{
    namespace  // Tree traversal callbacks
    {
        ZOctree* GetOctreePtr(uint32_t lColiId)
        {
            if ((lColiId & 1) != 0)
            {
                return reinterpret_cast<ZOctree*>(&g_pEngineData->m_pPackedTreeData[lColiId & 0xFFFFFFFE]);
            }

            return reinterpret_cast<ZOctree*>(lColiId);
        }

        bool CalcRoomLineCallBack(uint32_t lID, SOctreeChk* pDat)
        {
            auto* pData = reinterpret_cast<SRoomLineChk*>(pDat);
            auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(lID);

            ZASSERT(pBaseGeom->IsDerivedFrom<ZROOM>());

            if ((pBaseGeom->GetColiBits() & 0xC00) == 0)
            {
                return false;
            }

            auto* pRoom = geom_cast<ZROOM>(pBaseGeom->GetGeom());
            ZASSERT(pRoom);

            if ((pRoom->RoomControl() & 1) != 0)
            {
                return false;
            }

            pData->m_bWorkRoomsWrapAround = pData->m_lCurWorkRoom == SRoomLineChk::MAX_WORK_ROOM_NR ? 1 : pData->m_bWorkRoomsWrapAround;
            pData->m_lCurWorkRoom = pData->m_lCurWorkRoom & (SRoomLineChk::MAX_WORK_ROOM_NR - 1);

            if (pData->m_bWorkRoomsWrapAround)
            {
                pData->m_pWorkRoomList[pData->m_lCurWorkRoom]->SetRoomControl(0, 1u);
            }

            pData->m_pWorkRoomList[pData->m_lCurWorkRoom++] = pRoom;
            pRoom->SetRoomControl(1u, 0u);

            auto* pGlobalTreePtr = pRoom->GetGlobalTreePtr(pData->eGTT);
            if (!pGlobalTreePtr)
            {
                return false;
            }

            ZVector3 vPoint, vVect;
            pBaseGeom->GetLocalPointVect(vPoint, vVect);

            vVect += vPoint; // v3add(&vVect, &vPoint)

            if (!pGlobalTreePtr->CheckLinesegment(pData->pGeomLineChk, vPoint, vVect))
            {
                return false;
            }

            pData->fResT = pData->pGeomLineChk->fResT;
            return true;
        }

        bool CalcGeomLineCallBack(uint32_t lID, SOctreeChk* pDat)
        {
            auto* pData = reinterpret_cast<SGeomLineChk*>(pDat);
            auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(lID);
            if (!pBaseGeom || (pBaseGeom->GetColiBits() & 0xC00) == 0 || (pBaseGeom->GetColiBits() & pData->GeomConMask) == 0 || !pBaseGeom->Prim())
            {
                return false;
            }

            const SPrimHeader* pGeomHdr = ZPrimControlBase::GetPrimitive<const SPrimHeader>(pBaseGeom->Prim());
            ZASSERT(pGeomHdr);

            ZOctree* pOctree = nullptr;

            if (pGeomHdr->lType == EPrimType::PTSTRIP || pGeomHdr->lType == EPrimType::PTDOT3STRIP)
            {
                pOctree = GetOctreePtr(reinterpret_cast<const SPrimStrips*>(pGeomHdr)->lColiId);
            }
            else if (pGeomHdr->lType == EPrimType::PTOBJECTHEADER)
            {
                pOctree = GetOctreePtr(reinterpret_cast<const SPrimObjectHeader*>(pGeomHdr)->lColiId);
            }

            if (pOctree)
            {
                ZVector3 vPoint, vVect;
                pBaseGeom->GetLocalPointVect(vPoint, vVect);

                vVect += vPoint; // vadd(&vVect, &vPoint)

                if (pOctree->CheckLinesegment(pData->pStripLineChk, vPoint, vVect))
                {
                    pData->pStripLineChk->pImpact->pBaseGeom = pBaseGeom;
                    pData->fResT = pData->pStripLineChk->fResT;

                    return true;
                }

            }

            return false;
        }

        bool CalcStripLineCallBack(uint32_t lID, SOctreeChk* pDat)
        {
            auto* pData = reinterpret_cast<SStripLineChk*>(pDat);
            const STempStripsUniqueId* pStripInfo = reinterpret_cast<const STempStripsUniqueId*>(&pData->pUniqueSubStripInfo[lID]);

             if (!ZCollisionBase::GetCollisionInterface()->CalcLineCollision(
                pData->pImpact, pStripInfo,
                pData->vStart,
                pData->vVect,
                pData->bBothSides,
                pData->ColiMask))
            {
                return false;
            }

            pData->fResT = pData->pImpact->fPercent;
            return true;
        }

        bool CallBackStripInsideBox(uint32_t lID, SOctreeChk* pDat)
        {
            auto* pData = reinterpret_cast<SStripInsideBoxCubeChk*>(pDat);
            const STempStripsUniqueId* pStripInfo = reinterpret_cast<const STempStripsUniqueId*>(&pData->pUniqueSubStripInfo[lID]);
            ZRawStrip* pMemBuffer = reinterpret_cast<ZRawStrip*>(pData->pMemBuffer);

            if ((pData->pMemBufferEnd - pData->pMemBuffer) < 0x498) // What is 0x498???
            {
                return 0;
            }

            pMemBuffer->m_pBaseGeom = pData->pBaseGeom;

            char* pStripVerticesMemBuffer = pData->pCollisionBase->GetStripVerticesFromId(
                pMemBuffer,
                pStripInfo,
                pData->lColiMask,
                pData->mBaseGeomToBox,
                pData->vBaseGeomToBox,
                pData->vBoxDimensions);

            ZASSERT(pStripVerticesMemBuffer < pData->pMemBufferEnd);
            if (pStripVerticesMemBuffer == pData->pMemBuffer)
            {
                return false;
            }

            pData->pMemBuffer = pStripVerticesMemBuffer;
            ++pData->lNrStrips;

            return true;
        }

        bool TreeGetRightTypeCallBack(uint32_t lID, SOctreeChk* pDat)
        {
            STreeGetRightType* pData = reinterpret_cast<STreeGetRightType*>(pDat);
            auto* pGeomBase = ZGeomBuffer::Instance().GeomRefToBasePtr(lID);
            if (!pGeomBase)
            {
                return false;
            }

            if ((pGeomBase->Control() & pData->lGeomConMask) == 0 || (pGeomBase->Control() & 0xC00) != 0)
            {
                return false;
            }

            ZASSERT(&pData->pGeomList[pData->lNrGeomsInList] < pData->pGeomListEnd);

            auto** ppGeom = &pData->pGeomList[pData->lNrGeomsInList];
            if (ppGeom < pData->pGeomListEnd)
            {
                *ppGeom = pGeomBase;
                ++pData->lNrGeomsInList;
            }

            return true;
        }
    }

    STreeGetRightType::STreeGetRightType()
        : SRecurseInfoCompiled()
    {
        lGeomConMask = 0;
        lNrGeomsInList = 0;
        pChkFunc = TreeGetRightTypeCallBack;
    }

    namespace
    {
        /**
         * Serialized in-place layout consumed by ZCollisionBase::LoadInternColiTree().
         * The buffer is cast to this struct; the tree object is placement-constructed
         * over its serialized bytes (its vftable pointer reuses the first 4 bytes
         * right after the block-offset field).
         *
         *   +0x00  int32                  offset to the next serialized block
         *   +0x04  ZOctreeCompiled        in-place constructed tree object (0x18 bytes)
         *   +0x1C  uint32                 padding/alignment gap
         *   +0x20  SBinaryOctreeHeader    serialized tree data (m_pxTree target)
         */
        struct SInternColiTree
        {
            int32_t m_lNextBlockOffset;
            ZOctreeCompiled m_Octree;
            uint32_t m_dwPadding;

            void* GetTreeData() { return reinterpret_cast<char*>(this) + sizeof(SInternColiTree); }
        };
        RE_VERIFY_OFFSET(SInternColiTree, m_Octree, 0x4);
        RE_VERIFY_SIZE(SInternColiTree, 0x20);
    }

    ZCollisionBase::ZCollisionBase(bool bPackEnabled)
        : m_CollisionBox()
    {
        m_pCollisionPackBase = nullptr;
        m_pBuildOctreeList = nullptr;
        m_lUniqueSubStripInfoCount = 0;
        m_pUniqueSubStripInfo = nullptr;
        m_bUniqueInfoLoaded = 0;
        m_pRoomColiTree = nullptr;
        m_pRoomInsideTree = nullptr;
    }

    ZCollisionBase::~ZCollisionBase() = default;

    ZCollisionPackBase* ZCollisionBase::GetCollisionPack()
    {
        return m_pCollisionPackBase;
    }

    void ZCollisionBase::InstallCollisionBuffer(char* pData, int lDataSize)
    {
        if (lDataSize != -1)
        {
            m_pRoomColiTree = ZUniMemory::New<ZOctreeCompiled>();
            m_pRoomColiTree->Init(pData);
        }
    }

    void ZCollisionBase::InstallInsideBuffer(char* pData, int lDataSize)
    {
        if (lDataSize != -1)
        {
            m_pRoomInsideTree = ZUniMemory::New<ZOctreeCompiled>();
            m_pRoomInsideTree->Init(pData);
        }
    }

    void ZCollisionBase::FreeSceneMemory()
    {
        if (m_pRoomColiTree)
        {
            ZUniMemory::Delete(m_pRoomColiTree);
            m_pRoomColiTree = nullptr;
        }

        if (m_pRoomInsideTree)
        {
            ZUniMemory::Delete(m_pRoomInsideTree);
            m_pRoomInsideTree = nullptr;
        }
    }

    uint32_t ZCollisionBase::GetRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, ZOctreeCompiled* pRoomTree, const float* Mat1, const float* Cen1, const float* Size1)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZCollisionBase::GetRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, ZOctreeCompiled* pRoomTree, const float* vPos)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZCollisionBase::GetInsideRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, const float* Mat1, const float* Cen1, const float* Size1)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZCollisionBase::GetColiRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, const float* Mat1, const float* Cen1, const float* Size1)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZCollisionBase::GetInnerRoomsLst(ZROOM** pRoomList, ZROOM** pRoomListEnd, const float* Mat1, const float* Cen1, const float* Size1, bool bIncludeBackdrop)
    {
        // TODO: Finish me
        return 0;
    }

    bool ZCollisionBase::CalcLineColi(SExtendedImpactInfo* pImpact, ZROOM* pRoom, eGlobalTreeType eGTT, const float* vPos, float* const vVect, bool bBothSides, uint32_t GeomConMask)
    {
        auto* pOctree = pRoom->GetGlobalTreePtr(eGTT);
        if (!pOctree)
        {
            return false;
        }

        SStripLineChk sStripLineCheck {};
        SGeomLineChk sGeomLineCheck {};

        sStripLineCheck.pUniqueSubStripInfo = reinterpret_cast<char*>(m_pUniqueSubStripInfo);
        sStripLineCheck.bBothSides = bBothSides;
        sStripLineCheck.ColiMask = GeomConMask;
        sStripLineCheck.pChkFunc = CalcStripLineCallBack;
        sStripLineCheck.pImpact = pImpact;

        sGeomLineCheck.vStart = vPos;
        sGeomLineCheck.vVect = vVect;
        sGeomLineCheck.pStripLineChk = &sStripLineCheck;
        sGeomLineCheck.GeomConMask = GeomConMask;
        sGeomLineCheck.pChkFunc = CalcGeomLineCallBack;

        ZVector3 vB { 0.f };
        ZVector3 vA { vPos };
        vmmul(vB, vA, vVect);

        if (pOctree->CheckLinesegment(&sGeomLineCheck, vA, vB))
        {
            pImpact->pBaseGeom->GetRootPoint(pImpact->vPosition);
            pImpact->m_iColiMaterialDescId = FindMaterialDescriptor(pImpact->m_HitCache.___u0.__s0.lIdHi);
            return true;
        }

        return false;
    }

    bool ZCollisionBase::CalcLineColi(SExtendedImpactInfo* pImpact, eGlobalTreeType eGTT, const float* vPos, const float* vVect, bool bBothSides, uint32_t GeomConMask, bool bCheckStatic, bool bCheckDynamic)
    {
        return CalcLineColi_(pImpact, eGTT, vPos, vVect, bBothSides, GeomConMask, bCheckStatic, bCheckDynamic);
    }

    bool ZCollisionBase::CheckLineColi(SExtendedImpactInfo* pImpact, eGlobalTreeType eGTT, const float* vPos, const float* vVect, bool bBothSides, uint32_t GeomConMask, bool bCheckStatic, bool bCheckDynamic)
    {
        return CalcLineColi(pImpact, eGTT, vPos, vVect, bBothSides, GeomConMask, bCheckStatic, bCheckDynamic);
    }

    uint32_t ZCollisionBase::GetGeomsInBox(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, const float* mMat, const float* vCen, const float* vSize, int GeomConMask, bool bCheckStatic, bool bCheckDynamic, bool bExact)
    {
        static constexpr uint32_t MAX_ROOMS_NR = 256u;
        uint32_t lCount = 0;

        if (bCheckStatic)
        {
            ZROOM* aRooms[MAX_ROOMS_NR] { nullptr };

            const uint32_t lRooms = GetRoomsLst(aRooms, &aRooms[256], m_pRoomColiTree, mMat, vCen, vSize);
            const uint32_t lRoomsCount = std::min(MAX_ROOMS_NR, lRooms);
            for (uint32_t i = 0; i < lRoomsCount; ++i)
            {
                ZVector3 vCenter { vCen };
                ZMat3x3 mMatrix { mMat };
                ZROOM* pCurrentRoom = aRooms[i];

                pCurrentRoom->GetLocalMatPos(mMatrix, vCenter);
                lCount += pCurrentRoom->GetStaticGeomsInBound(&pGeomList[lCount], pGeomListEnd, eGTT, mMatrix, vCen, vSize,GeomConMask, bExact);
            }
        }

        if (bCheckDynamic)
        {
            lCount += GetDynamicGeomsInBox(&pGeomList[lCount], pGeomListEnd, eGTT, mMat, vCen, vSize, GeomConMask, bExact);
        }

        return lCount;
    }

    uint32_t ZCollisionBase::GetGeomsInBoxLocal(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, ZTreeGroup* pTreeGroup, eGlobalTreeType eGTT, const float* mMat, const float* vCen, const float* vSize, int GeomConMask, bool bCheckStatic, bool bCheckDynamic, bool bExact)
    {
        uint32_t lCount = 0u;

        if (bCheckStatic)
        {
            lCount = pTreeGroup->GetStaticGeomsInBound(pGeomList, pGeomListEnd, eGTT, mMat, vCen, vSize, GeomConMask, bExact);
        }

        if (bCheckDynamic)
        {
            ZVector3 vCenter { vCen };
            ZMat3x3 mMatrix { mMat };
            pTreeGroup->GetRootMatPos(mMatrix, vCenter);

            lCount += GetDynamicGeomsInBox(&pGeomList[lCount], pGeomListEnd, eGTT, mMatrix, vCenter, vSize, GeomConMask, bExact);
        }

        return lCount;
    }

    void ZCollisionBase::SaveUniqueSubStripInfo(CHUNK* pChunk)
    {
        pChunk->AddData(m_pUniqueSubStripInfo, 8 * m_lUniqueSubStripInfoCount);
    }

    void ZCollisionBase::LoadUniqueSubStripInfo(SUniqueSubStripInfo* pData, uint32_t lSize)
    {
        m_lUniqueSubStripInfoCount = lSize >> 3;
        ZASSERT(m_lUniqueSubStripInfoCount <= UNIQUESUBSTRIPINFOCOUNT);

        m_pUniqueSubStripInfo = pData;
        m_bUniqueInfoLoaded = true;
    }

    bool ZCollisionBase::IsCollisionBoxLocked() const
    {
        return m_CollisionBox.IsLocked();
    }

    ZCollisionBox* ZCollisionBase::LockCollisionBox(char* pMemBuffer, uint32_t lMemBufferSize)
    {
        ZASSERT(!m_CollisionBox.IsLocked());

        m_CollisionBox.Lock();
        m_CollisionBox.SetMemBuffer(pMemBuffer, lMemBufferSize);
        m_CollisionBox.m_lNrStrips = 0;

        return &m_CollisionBox;
    }
    void ZCollisionBase::UnlockCollisionBox(ZCollisionBox* pCollisionBox)
    {
        ZASSERT(m_CollisionBox.IsLocked());

        m_CollisionBox.Unlock();
        ZASSERT(pCollisionBox == &m_CollisionBox);
    }

    char* ZCollisionBase::LoadInternColiTree(char* pOctreeBuffer)
    {
        auto* pTree = reinterpret_cast<SInternColiTree*>(pOctreeBuffer);

        znew_placement<ZOctreeCompiled>(&pTree->m_Octree);

        pTree->m_Octree.Init(pTree->GetTreeData());

        ZASSERT(pTree->m_Octree.GetScale() > 0.0f);

        return pOctreeBuffer + pTree->m_lNextBlockOffset;
    }

    bool ZCollisionBase::CalcColiLort(SExtendedImpactInfo* Impact, ZBaseGeom* pDynBaseGeom, eGlobalTreeType eGTT, const float* vLineStart, const float* vLineDirection, uint32_t GeomConMask, bool bBothSides)
    {
        // TODO: Finish me
        return false;
    }

    bool ZCollisionBase::CalcDynamicLineCollision(SExtendedImpactInfo* Impact, const ZTreeGroup* pTreeGroup, eGlobalTreeType eGTT, const float* vP, const float* vD, bool bBothSides, int GeomConMask)
    {
        ZASSERT(pTreeGroup->IsDynamicContainer());

        if (!pTreeGroup->Active())
        {
            return false;
        }

        auto* pQuadtree = pTreeGroup->GetDynamicTreePtr();
        if (!pQuadtree)
        {
            return false;
        }

        STreeGetRightType sGetRT {};

        sGetRT.lGeomConMask = GeomConMask;
        sGetRT.pGeomList = m_GeomList;
        sGetRT.pGeomListEnd = &m_GeomList[MAX_GEOMS_NR];

        ZVector3 vA {};
        vadd(vA, vP, vD);

        pQuadtree->CheckLinesegment(&sGetRT, vP, vA);
        bool bResult = false;
        if (sGetRT.lNrGeomsInList)
        {
            bool bFoundAtLeastOne = false;

            for (int i = 0; i < sGetRT.lNrGeomsInList; ++i)
            {
                bFoundAtLeastOne = CalcColiLort(Impact, m_GeomList[i], eGTT, vP, vD, GeomConMask, bBothSides) || bFoundAtLeastOne;
                bResult = bFoundAtLeastOne;
            }

            if (bFoundAtLeastOne && Impact->m_BoneId)
            {
                const ZBaseGeom* pBaseGeom = Impact->pBaseGeom;
                const ZGEOM* pGeom = pBaseGeom->GetGeom();

                if (pBaseGeom->IsDerivedFrom<ZLNKOBJ>())
                {
                    const ZLNKOBJ* pLnkObj = geom_cast<ZLNKOBJ>(pGeom);

                    if (auto* pBoneModify = pLnkObj->GetBoneModifier())
                    {
                        ZMat3x3 mBone;
                        ZVector3 vBone;

                        pBoneModify->GetBoneMatPos(mBone, vBone, Impact->m_BoneId, pLnkObj);

                        vsub(vBone, Impact->vPosition);

                        createmat(mBone.data, vBone, nullptr);

                        Impact->vP1 = Impact->vPosition;
                        vadd(Impact->vP2, Impact->vPosition, &mBone.data[6]);
                        vadd(Impact->vP3, Impact->vPosition, &mBone.data[6]);
                        vadd(Impact->vP3, &mBone.data[3]);
                    }
                    else
                    {
                        Impact->vP1 = {};
                        Impact->vP2 = { 1.f, 0.f, 0.f };
                        Impact->vP3 = { 1.f, 1.f, 0.f };
                    }
                }
            }
        }

        return bResult;
    }

    bool ZCollisionBase::CalcLineCollision(SExtendedImpactInfo* pImpact, ZBaseGeom* pBaseGeom, float* vLineStart, float* vLineDirection, bool bTwoSided, uint32_t lColiMask)
    {
        // TODO: Finish me
        return false;
    }

    uint32_t ZCollisionBase::FindMaterialDescriptor(uint32_t lPrim)
    {
        if (!lPrim)
        {
            return 1;
        }

        const SPrimHeader* pPrimHdr = ZPrimControlBase::GetPrimitive<const SPrimHeader>(lPrim);
        if (pPrimHdr->lType == EPrimType::PTMESH)
        {
            const SPrimMesh* pMesh = reinterpret_cast<const SPrimMesh*>(pPrimHdr);
            return ZRenderMaterialBuffer::g_pMaterialBufferInstance->GetMaterialInstance(pMesh->lMaterialId)->m_pMaterialProperties->lMaterialDescriptor;
        }

        if (pPrimHdr->lType == EPrimType::PTOBJECTHEADER)
        {
            const SPrimObjectHeader* pObjectHdr = reinterpret_cast<const SPrimObjectHeader*>(pPrimHdr);
            const uint32_t* pObjectTable = ZPrimControlBase::GetPrimitive<const uint32_t>(pObjectHdr->lObjectTable);
            const SPrimObject* pFoundObject = nullptr;

            for (int i = 0; i < pObjectHdr->lNumObjects; ++i)
            {
                const SPrimObject* pObject = ZPrimControlBase::GetPrimitive<const SPrimObject>(pObjectTable[i]);
                if (pObject->lType == EPrimType::PTMESH)
                {
                    pFoundObject = pObject;
                    break;
                }
            }

            if (!pFoundObject)
            {
                return 1;
            }

            return ZRenderMaterialBuffer::g_pMaterialBufferInstance->GetMaterialInstance(pFoundObject->lMaterialId)->m_pMaterialProperties->lMaterialDescriptor;
        }

        return 1;
    }

    uint32_t ZCollisionBase::GetDynamicGeomsInBox(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, const float* mMat, const float* vCen, const float* vSize, uint32_t lGeomConMask, bool bExact)
    {
        // TODO: Finish me
        return 0;
    }

    uint32_t ZCollisionBase::GetDynamicGeomsInBoxInRooms(ZBaseGeom** pGeomList, ZBaseGeom** pGeomListEnd, eGlobalTreeType eGTT, ZROOM** pRoomList, uint32_t lNrRooms, const float* mMat, const float* vCen, const float* vSize, uint32_t lGeomConMask, bool bExact)
    {
        // TODO: Finish me
        return 0;
    }

    bool ZCollisionBase::GetStripsInsideBox(uint32_t* pNrStrips, char* pMemBuffer, uint32_t lMemBufferSize, const float* mBoxMatrix, const float* vBoxPosition, const float* vBoxDimensions, uint32_t lColiMask, bool bCheckStatic, bool bCheckDynamic, bool bIgnoreActors, eGlobalTreeType eGTT)
    {
        // TODO: Finish me
        return false;
    }

    bool ZCollisionBase::CalcLineColi_(SExtendedImpactInfo *pImpact, eGlobalTreeType eGTT, const float* vPos, const float* vVect, bool bBothSides, uint32_t GeomConMask, bool bCheckStatic, bool bCheckDynamic)
    {
        if (bCheckStatic)
        {
            ZASSERT(m_pRoomColiTree);

            // TODO: Finish me
        }

        // TODO: Finish me
        return false;
    }

    ZCollisionBase* ZCollisionBase::GetCollisionInterface()
    {
        return ZCollisionBase::s_pCollisionBase;
    }

    STATIC_CLASS_VAR_IMPL(ZCollisionBase, ZCollisionBase*, s_pCollisionBase, 0x0090DE98, nullptr);
}
