#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/Camera/ZCameraSpace.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    // PC 00479CF0: vcross + normalize
    void CalcPlaneNormal(ZVector3* pOut, const ZVector3* pA, const ZVector3* pB)
    {
        vcross(pOut->Get(), pA->Get(), pB->Get());
        vnorm(pOut->Get());
    }

    // PC 0047A360: checks whether an exit portal is facing the viewer and inside
    // the frustum planes. Outputs the portal plane (normal + distance) and the
    // four portal corners expressed in the room-local (viewer) space.
    bool CheckExitVisible(
        Glacier::ZVector3* pPlaneOut,
        Glacier::ZVector3* pCorners,
        const Glacier::ZMat3x3& mRoom,
        const Glacier::ZVector3& vRoomPos,
        const Glacier::ZVector3* pExit,
        const float* pPlanes,
        uint32_t lNrPlanes)
    {
        ZVector3 v12;
        ZVector3 v11;
        vsub(v12.Get(), pExit[1].Get(), pExit[0].Get());
        vsub(v11.Get(), pExit[2].Get(), pExit[0].Get());
        vcross(pPlaneOut[0].Get(), v12.Get(), v11.Get());
        TransformRootVector(pPlaneOut[0], mRoom);

        vmmul(pCorners[0].Get(), pExit[0].Get(), mRoom.Get());
        vadd(pCorners[0].Get(), vRoomPos.Get());

        const float fDistance = -vdot(pPlaneOut[0].Get(), pCorners[0].Get());
        pPlaneOut[1].x = fDistance;

        if (fDistance > 0.0f)
        {
            vmmul(pCorners[1].Get(), pExit[1].Get(), mRoom.Get());
            vadd(pCorners[1].Get(), vRoomPos.Get());
            vmmul(pCorners[2].Get(), pExit[2].Get(), mRoom.Get());
            vadd(pCorners[2].Get(), vRoomPos.Get());
            vmmul(pCorners[3].Get(), pExit[3].Get(), mRoom.Get());
            vadd(pCorners[3].Get(), vRoomPos.Get());

            if (lNrPlanes == 0)
            {
                return true;
            }

            uint32_t lChecked = 0;
            for (uint32_t iPlane = 1;; ++iPlane)
            {
                const float* pPlane = &pPlanes[4 * iPlane];
                const bool bAnyOutside =
                    vdot(pCorners[0].Get(), pPlane) + pPlane[3] < 0.0f ||
                    vdot(pCorners[1].Get(), pPlane) + pPlane[3] < 0.0f ||
                    vdot(pCorners[2].Get(), pPlane) + pPlane[3] < 0.0f ||
                    vdot(pCorners[3].Get(), pPlane) + pPlane[3] < 0.0f;

                if (!bAnyOutside)
                {
                    return false;
                }

                if (++lChecked == lNrPlanes)
                {
                    return true;
                }
            }
        }

        return false;
    }

    // PC 00479D20: clips the incoming portal quad (4 corners) against an existing
    // portal (4 corner "planes") and stores the clipped quad. Returns false when
    // the quad lies entirely on one side of a portal plane (no intersection).
    bool ClipExitQuad(Glacier::ZVector3* pOut, const Glacier::ZVector3* pCorners, const Glacier::ZVector3* pPlanes)
    {
        uint8_t lFlags[4] = { 0, 0, 0, 0 };

        for (uint32_t iPlane = 0; iPlane < 4; ++iPlane)
        {
            const uint8_t lBit = static_cast<uint8_t>(1u << iPlane);
            bool bAllPositive = true;

            for (uint32_t iCorner = 0; iCorner < 4; ++iCorner)
            {
                if (vdot(pCorners[iCorner].Get(), pPlanes[iPlane].Get()) > 0.0f)
                {
                    lFlags[iCorner] |= lBit;
                }
                else
                {
                    bAllPositive = false;
                }
            }

            if (bAllPositive)
            {
                return false;
            }
        }

        const uint8_t lEdge01 = lFlags[0] & lFlags[1];
        const uint8_t lEdge12 = lFlags[1] & lFlags[2];
        const uint8_t lEdge23 = lFlags[2] & lFlags[3];
        const uint8_t lEdge30 = lFlags[3] & lFlags[0];

        if (lEdge01 == 0) CalcPlaneNormal(&pOut[0], &pCorners[0], &pCorners[1]);
        if (lEdge12 == 0) CalcPlaneNormal(&pOut[1], &pCorners[1], &pCorners[2]);
        if (lEdge23 == 0) CalcPlaneNormal(&pOut[2], &pCorners[2], &pCorners[3]);
        if (lEdge30 == 0) CalcPlaneNormal(&pOut[3], &pCorners[3], &pCorners[0]);

        for (uint32_t iPlane = 0; iPlane < 4; ++iPlane)
        {
            const uint8_t lBit = static_cast<uint8_t>(1u << iPlane);

            if (lEdge01 & lBit) pOut[0] = pPlanes[iPlane];
            if (lEdge12 & lBit) pOut[1] = pPlanes[iPlane];
            if (lEdge23 & lBit) pOut[2] = pPlanes[iPlane];
            if (lEdge30 & lBit) pOut[3] = pPlanes[iPlane];
        }

        return true;
    }
}

namespace Glacier
{
    ZViewSpace::ZViewSpace()
    {
        for (int i = 0; i < MAX_ENTRIES_NR; ++i)
        {
            m_Rooms.m_Array[i].m_pRoom = nullptr;
            m_Rooms.m_Array[i].m_lNumExits = 0;
            m_Rooms.m_Array[i].m_pFirstExit = nullptr;
        }
        m_Rooms.m_lNrEntries = 0;
        m_Exits.m_lNrEntries = 0;
        m_pTopNode = nullptr;
        m_bExitsEnabled = true;
        m_bTraverseThroughExitsEnabled = true;
        m_RoomGeomVolumes.m_Rooms.m_lNrEntries = 0;
        m_RoomGeomVolumes.m_DefaultBlock.m_pNext = nullptr;
        m_RoomGeomVolumes.m_DefaultBlock.m_pCurrent = m_RoomGeomVolumes.m_DefaultBlock.m_Memory;
        m_RoomGeomVolumes.m_pFirstBlock = &m_RoomGeomVolumes.m_DefaultBlock;
        for (int i = 0; i < 12; ++i)
        {
            m_bClipPlanesEnabled[i] = false;
        }
    }

    void ZViewSpace::SetClipPlanesFromCameraSpace(ZCameraSpace* pCamSpace)
    {
        uint32_t lNrClipPlanes = pCamSpace->CalcRootClipPlanes(m_ClipPlanes, 12);

        const float* pStartPoint = pCamSpace->GetCameraStartPoint();
        m_vPosition = ZVector3(pStartPoint);
        m_vPositionInnerRoom = ZVector3(pStartPoint);

        for (uint32_t i = 0; i < lNrClipPlanes; ++i)
        {
            m_bClipPlanesEnabled[i] = true;
        }

        if (pCamSpace->GetGatesEnabled())
        {
            m_bExitsEnabled = true;
            m_pTopNode = nullptr;
        }
        else
        {
            m_bExitsEnabled = false;
            m_pTopNode = static_cast<ZGROUP*>(ZGEOM::RefToPtr(pCamSpace->GetTopNode()));
            if (!m_pTopNode)
            {
                m_pTopNode = g_pEngineData->m_pRoot;
            }
        }

        if (!pCamSpace->GetGeomBoundsEnabled())
        {
            m_bExitsEnabled = false;
            for (int i = 0; i < 12; ++i)
            {
                m_bClipPlanesEnabled[i] = false;
            }
        }

        m_pOwnerRoom = nullptr;
    }

    void ZViewSpace::SetClipPlane(uint32_t lIndex, const float* pPlane)
    {
        ZASSERT(lIndex < 12);

        float* pDst = &m_ClipPlanes[4 * lIndex];
        pDst[0] = pPlane[0];
        pDst[1] = pPlane[1];
        pDst[2] = pPlane[2];
        pDst[3] = pPlane[3];
    }

    bool ZViewSpace::SetClipPlaneEnabled(uint32_t lIndex, bool bEnabled)
    {
        ZASSERT(lIndex < 12);

        bool bPrevious = m_bClipPlanesEnabled[lIndex];
        m_bClipPlanesEnabled[lIndex] = bEnabled;
        return bPrevious;
    }

    void ZViewSpace::GetClipPlane(uint32_t lIndex, float* pPlane) const
    {
        ZASSERT(lIndex < 12);

        const float* pSrc = &m_ClipPlanes[4 * lIndex];
        pPlane[0] = pSrc[0];
        pPlane[1] = pSrc[1];
        pPlane[2] = pSrc[2];
        pPlane[3] = pSrc[3];
    }

    void ZViewSpace::GetPosition(float* pPosition) const
    {
        pPosition[0] = m_vPosition.x;
        pPosition[1] = m_vPosition.y;
        pPosition[2] = m_vPosition.z;
    }

    void ZViewSpace::SetPosition(const float* pPosition)
    {
        m_vPosition.x = pPosition[0];
        m_vPosition.y = pPosition[1];
        m_vPosition.z = pPosition[2];
    }

    void ZViewSpace::SetPositionInnerRoom(const float* pPosition)
    {
        m_vPositionInnerRoom.x = pPosition[0];
        m_vPositionInnerRoom.y = pPosition[1];
        m_vPositionInnerRoom.z = pPosition[2];
    }

    void ZViewSpace::GetPositionInnerRoom(float* pPosition) const
    {
        pPosition[0] = m_vPositionInnerRoom.x;
        pPosition[1] = m_vPositionInnerRoom.y;
        pPosition[2] = m_vPositionInnerRoom.z;
    }

    uint32_t ZViewSpace::GetClipPlaneDistances(float* pPlanes) const
    {
        uint32_t lNrPlanes = 0;

        for (uint32_t i = 0; i < 12; ++i)
        {
            if (m_bClipPlanesEnabled[i])
            {
                const float* pPlane = &m_ClipPlanes[4 * i];
                pPlanes[0] = pPlane[0];
                pPlanes[1] = pPlane[1];
                pPlanes[2] = pPlane[2];
                pPlanes[3] = m_vPosition.x * pPlane[0] + m_vPosition.y * pPlane[1] + m_vPosition.z * pPlane[2] + pPlane[3];
                pPlanes += 4;
                ++lNrPlanes;
            }
        }

        return lNrPlanes;
    }

    void ZViewSpace::Reset()
    {
        for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
        {
            ZASSERT(m_Rooms.m_Array[i].m_pRoom->m_pTempRoom == nullptr);
        }

        for (int i = 0; i < 12; ++i)
        {
            m_bClipPlanesEnabled[i] = false;
        }

        m_Rooms.m_lNrEntries = 0;
        m_Exits.m_lNrEntries = 0;
    }

    void ZViewSpace::LocateRoomsAndExits()
    {
        Reset();

        float vPlanes[48];
        uint32_t lNrPlanes = GetClipPlaneDistances(vPlanes);

        if (m_bExitsEnabled)
        {
            ZVector3 vScale(1.0f, 1.0f, 1.0f);
            ZMat3x3 mMat;
            mMat.Reset();

            ZROOM* pRooms[256];
            uint32_t lNrRooms;

            if (m_pOwnerRoom)
            {
                pRooms[0] = m_pOwnerRoom;
                lNrRooms = 1;
            }
            else
            {
                lNrRooms = ZCollisionBase::s_pCollisionBase->GetInnerRoomsLst(
                    pRooms,
                    pRooms + 256,
                    mMat,
                    m_vPositionInnerRoom,
                    vScale,
                    false);
            }

            for (uint32_t i = 0; i < lNrRooms; ++i)
            {
                ZASSERT(m_Rooms.m_lNrEntries < MAX_ENTRIES_NR);

                ZVisibleRoom* pVisibleRoom = &m_Rooms.m_Array[m_Rooms.m_lNrEntries];
                pVisibleRoom->m_pRoom = pRooms[i];
                pVisibleRoom->m_lNumExits = 0;
                pVisibleRoom->m_pFirstExit = nullptr;
                ++m_Rooms.m_lNrEntries;

                pRooms[i]->m_pTempRoom = reinterpret_cast<ZTempRoom*>(pVisibleRoom);
                pRooms[i]->m_bIsOnDrawStack = true;
            }

            if (m_bTraverseThroughExitsEnabled)
            {
                for (uint32_t i = 0; i < lNrRooms; ++i)
                {
                    CheckExitsInRoom(
                        reinterpret_cast<ZVisibleRoom*>(pRooms[i]->m_pTempRoom),
                        nullptr,
                        true,
                        vPlanes,
                        lNrPlanes);
                }
            }

            for (uint32_t i = 0; i < lNrRooms; ++i)
            {
                pRooms[i]->m_bIsOnDrawStack = false;

                ZVisibleRoom* pVisibleRoom = reinterpret_cast<ZVisibleRoom*>(pRooms[i]->m_pTempRoom);
                pVisibleRoom->m_pFirstExit = nullptr;
                pVisibleRoom->m_lNumExits = 0;
            }

            for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
            {
                ZROOM* pRoom = m_Rooms.m_Array[i].m_pRoom;
                if (pRoom)
                {
                    pRoom->m_iLastVisibleFrameCount = g_pSysInterface->m_lFrameCount;
                }
            }
        }
        else
        {
            for (uint32_t i = 0; i < lNrPlanes; ++i)
            {
                vPlanes[4 * i + 3] -= m_vPosition.x * vPlanes[4 * i] + m_vPosition.y * vPlanes[4 * i + 1] + m_vPosition.z * vPlanes[4 * i + 2];
            }

            ZMat3x3 mMat;
            mMat.Reset();
            ZVector3 vPos(0.0f, 0.0f, 0.0f);

            if (m_pTopNode && m_pTopNode->IsDerivedFrom<ZROOM>())
            {
                GetVisibleRoomsRecursive(m_pTopNode, vPlanes, lNrPlanes, mMat.Get(), vPos.Get());
            }
        }
    }

    void ZViewSpace::GetVisibleRoomsRecursive(ZGROUP* pGroup, const float* pPlanes, uint32_t lNrPlanes, const float* pMat, const float* pPos)
    {
        if (pGroup->IsDerivedFrom<ZROOM>())
        {
            ZASSERT(m_Rooms.m_lNrEntries < MAX_ENTRIES_NR);

            ZVisibleRoom* pVisibleRoom = &m_Rooms.m_Array[m_Rooms.m_lNrEntries];
            pVisibleRoom->m_pRoom = static_cast<ZROOM*>(pGroup);
            pVisibleRoom->m_lNumExits = 0;
            pVisibleRoom->m_pFirstExit = nullptr;
            ++m_Rooms.m_lNrEntries;
        }

        // Transform the frustum planes into this group's local space.
        float vLocalPlanes[48];
        for (uint32_t i = 0; i < lNrPlanes; ++i)
        {
            const float* pSrc = &pPlanes[4 * i];
            float* pDst = &vLocalPlanes[4 * i];

            ZVector3 vNormal(pSrc[0], pSrc[1], pSrc[2]);
            vmmul(vNormal.Get(), pMat);

            pDst[0] = vNormal.x;
            pDst[1] = vNormal.y;
            pDst[2] = vNormal.z;
            pDst[3] = pSrc[3] - vdot(vNormal.Get(), pPos);
        }

        // View position in this group's local space, and the transposed matrix.
        ZVector3 vLocalPos = pPos;
        vneg(vLocalPos);
        vmtmul(vLocalPos, pMat);

        ZMat3x3 mLocalMat;
        tmat(mLocalMat.Get(), pMat);

        ZBaseGeom* pChild = pGroup->m_pGroupFirst;
        while (ForGroupsCheck(pChild))
        {
            if ((pChild->m_lControl & 0x202C00) == 0)
            {
                ZGROUP* pChildGroup = static_cast<ZGROUP*>(pChild->m_pExtraGeom);

                if ((pChildGroup->m_lGroupCon & 0x400) == 0)
                {
                    ZVector3 vCenter;
                    vmmul(vCenter.Get(), pChild->m_vCen.Get(), pChild->m_mMat.Get());
                    vadd(vCenter.Get(), pChild->m_vPos.Get());

                    // TODO: Verify the group bound-tree flag test (PC 0047AEA0,
                    // a virtual call result whose bit 15 gates the sphere test).
                    // Decompiled control flow skips the sphere test when the flag
                    // is set; treat every child as cullable for now.
                    const float fRadius = pChild->m_fRadius;

                    bool bVisible = (lNrPlanes == 0);
                    for (uint32_t i = 0; i < lNrPlanes && !bVisible; ++i)
                    {
                        const float* pPlane = &vLocalPlanes[4 * i];
                        if (vdot(vCenter.Get(), pPlane) + pPlane[3] > fRadius)
                        {
                            break;
                        }

                        if (i + 1 == lNrPlanes)
                        {
                            bVisible = true;
                        }
                    }

                    if (bVisible)
                    {
                        ZVector3 vChildPos;
                        vsub(vChildPos.Get(), pPos, pChild->m_vPos.Get());
                        vmtmul(vChildPos.Get(), pChild->m_mMat.Get());

                        ZMat3x3 mNewMat;
                        mmtmul(mNewMat.Get(), pMat, pChild->m_mMat.Get());

                        GetVisibleRoomsRecursive(pChildGroup, pPlanes, lNrPlanes, mNewMat.Get(), vChildPos.Get());
                    }
                }
            }

            pChild = pChild->Next();
        }
    }

    void ZViewSpace::CheckExitsInRoom(ZVisibleRoom* pRoom, ZVisibleExit* pFirstExit, bool bEnabled, const float* pPlanes, uint32_t lNrPlanes)
    {
        ZROOM* pRoomGeom = pRoom->m_pRoom;

        ZMat3x3 mRoom;
        ZVector3 vRoomPos;
        pRoomGeom->GetRootTM(mRoom, vRoomPos);
        vsub(vRoomPos.Get(), m_vPosition.Get());

        ZROOM* pLastNeighbor = nullptr;
        ZVisibleRoom* pTempRoom = nullptr;

        // Deferred recursion work list: { ZVisibleRoom*, ZVisibleExit* } pairs.
        ZVisibleRoom* pDeferredRooms[256];
        ZVisibleExit* pDeferredExits[256];
        uint32_t lDeferred = 0;

        const int32_t lNrExits = static_cast<int32_t>(pRoomGeom->m_lNrExits);
        for (int32_t iExit = 0; iExit < lNrExits; ++iExit)
        {
            ZROOM::ZExit* pExit = &pRoomGeom->m_pExits[iExit];
            const uint8_t lControl = pExit->m_lControl;

            if ((lControl & 1) == 0 && (lControl & 2) != 0 && !pExit->m_pNeighbor->m_bIsOnDrawStack)
            {
                ZVector3 vPlane[2];
                ZVector3 vCorners[4];

                if (CheckExitVisible(vPlane, vCorners, mRoom, vRoomPos, &pExit->p1, pPlanes, lNrPlanes))
                {
                    ZVisibleExit* pExitList = pRoom->m_pFirstExit;

                    if (pExitList != nullptr)
                    {
                        if (pExitList != pFirstExit)
                        {
                            while (true)
                            {
                                ZVector3 vClipped[4];
                                if (ClipExitQuad(vClipped, vCorners, reinterpret_cast<const ZVector3*>(&pExitList->m_Exit)))
                                {
                                    ZROOM* pNeighbor = pExit->m_pNeighbor;
                                    if (pLastNeighbor != pNeighbor)
                                    {
                                        pLastNeighbor = pNeighbor;

                                        if (pNeighbor->m_pTempRoom != nullptr)
                                        {
                                            pTempRoom = reinterpret_cast<ZVisibleRoom*>(pNeighbor->m_pTempRoom);
                                        }
                                        else
                                        {
                                            ZASSERT(m_Rooms.m_lNrEntries < MAX_ENTRIES_NR);
                                            pTempRoom = &m_Rooms.m_Array[m_Rooms.m_lNrEntries];
                                            pTempRoom->m_pRoom = pNeighbor;
                                            pTempRoom->m_lNumExits = 0;
                                            pTempRoom->m_pFirstExit = nullptr;
                                            ++m_Rooms.m_lNrEntries;

                                            pNeighbor->m_pTempRoom = reinterpret_cast<ZTempRoom*>(pTempRoom);
                                        }

                                        pDeferredRooms[lDeferred] = pTempRoom;
                                        pDeferredExits[lDeferred] = pTempRoom->m_pFirstExit;
                                        ++lDeferred;
                                    }

                                    ZASSERT(m_Exits.m_lNrEntries < MAX_ENTRIES_NR);
                                    ZVisibleExit* pNewExit = &m_Exits.m_Array[m_Exits.m_lNrEntries];
                                    pNewExit->m_Exit.pl1 = vClipped[0];
                                    pNewExit->m_Exit.pl2 = vClipped[1];
                                    pNewExit->m_Exit.pl3 = vClipped[2];
                                    pNewExit->m_Exit.pl4 = vClipped[3];
                                    ++m_Exits.m_lNrEntries;

                                    pNewExit->m_pNext = pTempRoom->m_pFirstExit;
                                    pTempRoom->m_pFirstExit = pNewExit;
                                    ++pTempRoom->m_lNumExits;
                                }

                                pExitList = pExitList->m_pNext;
                                if (pExitList == pFirstExit)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        ZROOM* pNeighbor = pExit->m_pNeighbor;
                        if (pLastNeighbor != pNeighbor)
                        {
                            pLastNeighbor = pNeighbor;

                            if (pNeighbor->m_pTempRoom != nullptr)
                            {
                                pTempRoom = reinterpret_cast<ZVisibleRoom*>(pNeighbor->m_pTempRoom);
                            }
                            else
                            {
                                ZASSERT(m_Rooms.m_lNrEntries < MAX_ENTRIES_NR);
                                pTempRoom = &m_Rooms.m_Array[m_Rooms.m_lNrEntries];
                                pTempRoom->m_pRoom = pNeighbor;
                                pTempRoom->m_lNumExits = 0;
                                pTempRoom->m_pFirstExit = nullptr;
                                ++m_Rooms.m_lNrEntries;

                                pNeighbor->m_pTempRoom = reinterpret_cast<ZTempRoom*>(pTempRoom);
                            }

                            pDeferredRooms[lDeferred] = pTempRoom;
                            pDeferredExits[lDeferred] = pTempRoom->m_pFirstExit;
                            ++lDeferred;
                        }

                        ZVector3 vCross01;
                        ZVector3 vCross12;
                        ZVector3 vCross23;
                        ZVector3 vCross30;
                        CalcPlaneNormal(&vCross01, &vCorners[0], &vCorners[1]);
                        CalcPlaneNormal(&vCross12, &vCorners[1], &vCorners[2]);
                        CalcPlaneNormal(&vCross23, &vCorners[2], &vCorners[3]);
                        CalcPlaneNormal(&vCross30, &vCorners[3], &vCorners[0]);

                        ZASSERT(m_Exits.m_lNrEntries < MAX_ENTRIES_NR);
                        ZVisibleExit* pNewExit = &m_Exits.m_Array[m_Exits.m_lNrEntries];
                        pNewExit->m_Exit.pl1 = vCross01;
                        pNewExit->m_Exit.pl2 = vCross12;
                        pNewExit->m_Exit.pl3 = vCross23;
                        pNewExit->m_Exit.pl4 = vCross30;
                        ++m_Exits.m_lNrEntries;

                        pNewExit->m_pNext = pTempRoom->m_pFirstExit;
                        pTempRoom->m_pFirstExit = pNewExit;
                        ++pTempRoom->m_lNumExits;
                    }
                }
            }
        }

        pRoomGeom->m_bIsOnDrawStack = true;

        if (lDeferred == 0)
        {
            pRoomGeom->m_bIsOnDrawStack = bEnabled;
        }
        else
        {
            for (uint32_t i = 0; i < lDeferred; ++i)
            {
                CheckExitsInRoom(pDeferredRooms[i], pDeferredExits[i], false, pPlanes, lNrPlanes);
            }

            pRoomGeom->m_bIsOnDrawStack = bEnabled;
        }
    }

    uint32_t ZViewSpace::GetVisibleVolumes(ZVolumeList* pVolumeList, bool bIncludeBackdrop, ZRenderEntry* pRenderEntry)
    {
        // TODO: Finish me after ZVolumeList reversed (PC 0047C640)
        //
        // LocateRoomsAndExits();
        // float vPlanes[48];
        // uint32_t lNrPlanes = GetClipPlaneDistances(vPlanes);
        // for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
        //     FillRoomVolumes(pVolumeList, vPlanes, lNrPlanes, &m_Rooms.m_Array[i], bIncludeBackdrop);
        // for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
        //     m_Rooms.m_Array[i].m_pRoom->m_pTempRoom = nullptr;
        // return m_Rooms.m_lNrEntries;

        return 0;
    }

    bool ZViewSpace::GetVisibleVolumesIncludingBackdrop(ZVolumeList* pVolumeList, ZRenderEntry* pRenderEntry, bool bIncludeBackdrop)
    {
        // TODO: Finish me after ZVolumeList reversed (PC 0047C6F0)
        return false;
    }
}
