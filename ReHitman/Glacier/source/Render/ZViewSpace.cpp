#include <Glacier/Render/ZViewSpace.h>
#include <Glacier/Camera/ZCameraSpace.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZENVIRONMENT.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <xmmintrin.h>
#include <emmintrin.h>


namespace Glacier
{
    float ZViewSpace::m_Planes[48 + 16 * MAX_ENTRIES_NR];

    // PC 00479CF0: vcross + normalize
    void ZViewSpace::CreatePlane(ZVector3* pOut, const ZVector3* pA, const ZVector3* pB) const
    {
        vcross(pOut->Get(), pA->Get(), pB->Get());
        vnorm(pOut->Get());
    }

    // PC 0047A360: checks whether an exit portal is facing the viewer and inside
    // the frustum planes. Outputs the portal plane (normal + distance) and the
    // four portal corners expressed in the room-local (viewer) space.
    bool ZViewSpace::InitializeExit(
        ZVector3* pPlaneOut,
        ZVector3* pCorners,
        const ZMat3x3& mRoom,
        const ZVector3& vRoomPos,
        const ZVector3* pExit,
        const float* pPlanes,
        uint32_t lNrPlanes) const
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
    bool ZViewSpace::ExitVZExitPlanesChck(ZVector3* pOut, const ZVector3* pCorners, const ZVector3* pPlanes) const
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

        if (lEdge01 == 0) CreatePlane(&pOut[0], &pCorners[0], &pCorners[1]);
        if (lEdge12 == 0) CreatePlane(&pOut[1], &pCorners[1], &pCorners[2]);
        if (lEdge23 == 0) CreatePlane(&pOut[2], &pCorners[2], &pCorners[3]);
        if (lEdge30 == 0) CreatePlane(&pOut[3], &pCorners[3], &pCorners[0]);

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

    uint32_t ZViewSpace::GetEnabledClipPlanes(float* pPlanes) const
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
        uint32_t lNrPlanes = GetEnabledClipPlanes(vPlanes);

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

                if (InitializeExit(vPlane, vCorners, mRoom, vRoomPos, &pExit->p1, pPlanes, lNrPlanes))
                {
                    ZVisibleExit* pExitList = pRoom->m_pFirstExit;

                    if (pExitList != nullptr)
                    {
                        if (pExitList != pFirstExit)
                        {
                            while (true)
                            {
                                ZVector3 vClipped[4];
                                if (ExitVZExitPlanesChck(vClipped, vCorners, reinterpret_cast<const ZVector3*>(&pExitList->m_Exit)))
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
                        CreatePlane(&vCross01, &vCorners[0], &vCorners[1]);
                        CreatePlane(&vCross12, &vCorners[1], &vCorners[2]);
                        CreatePlane(&vCross23, &vCorners[2], &vCorners[3]);
                        CreatePlane(&vCross30, &vCorners[3], &vCorners[0]);

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

    void ZViewSpace::InitVisibCheck(const float* pCamPlanes, int iNPlanes, const ZVisibleExit* pVisibleExit, const float* m0, const float* p0)
    {
        m_iNumExists = 0;
        memset(m_Planes, 0, 0xC0);

        // Transform the camera clip planes into the SoA scratch buffer, four at a time.
        for (int iPlane = 0; iPlane < iNPlanes; ++iPlane)
        {
            const float* n = &pCamPlanes[4 * iPlane];
            const int iBase = (iPlane >> 2) * 16 + (iPlane & 3);

            m_Planes[iBase]      = m0[6] * n[0] + m0[7] * n[1] + m0[8] * n[2];
            m_Planes[iBase + 4]  = m0[3] * n[0] + m0[4] * n[1] + m0[5] * n[2];
            m_Planes[iBase + 8]  = m0[0] * n[0] + m0[1] * n[1] + m0[2] * n[2];
            m_Planes[iBase + 12] = p0[0] * n[0] + p0[1] * n[1] + p0[2] * n[2] + n[3];
        }

        m_iNumUserPlanes = (iNPlanes + 3) & ~3;

        // Transform each exit portal's four corners into four side planes (SoA).
        float* pPlane = &m_Planes[4 * m_iNumUserPlanes];
        static constexpr int lOrder[4] = { 0, 2, 1, 3 };  // pl1, pl3, pl2, pl4

        for (const ZVisibleExit* pExit = pVisibleExit; pExit != nullptr; pExit = pExit->m_pNext)
        {
            ++m_iNumExists;

            const ZVector3* pCorners = reinterpret_cast<const ZVector3*>(&pExit->m_Exit);
            for (int iCorner = 0; iCorner < 4; ++iCorner)
            {
                const ZVector3& corner = pCorners[lOrder[iCorner]];

                pPlane[iCorner]      = m0[6] * corner.x + m0[7] * corner.y + m0[8] * corner.z;
                pPlane[iCorner + 4]  = m0[3] * corner.x + m0[4] * corner.y + m0[5] * corner.z;
                pPlane[iCorner + 8]  = m0[0] * corner.x + m0[1] * corner.y + m0[2] * corner.z;
                pPlane[iCorner + 12] = p0[0] * corner.x + p0[1] * corner.y + p0[2] * corner.z;
            }

            pPlane += 16;
        }
    }

    bool ZViewSpace::IsVisible(ZBaseGeomVolume* pVolume)
    {
        const __m128 absMask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));

        const __m128 vCenterX = _mm_set1_ps(pVolume->m_vSize.x);
        const __m128 vCenterY = _mm_set1_ps(pVolume->m_vSize.y);
        const __m128 vCenterZ = _mm_set1_ps(pVolume->m_vSize.z);
        const __m128 vRadius = _mm_set1_ps(pVolume->m_fRadius);

        const float* pMat = pVolume->m_RootPosition.m0.data;
        const __m128 vCol0X = _mm_set1_ps(pMat[0]);
        const __m128 vCol0Y = _mm_set1_ps(pMat[1]);
        const __m128 vCol0Z = _mm_set1_ps(pMat[2]);
        const __m128 vCol1X = _mm_set1_ps(pMat[3]);
        const __m128 vCol1Y = _mm_set1_ps(pMat[4]);
        const __m128 vCol1Z = _mm_set1_ps(pMat[5]);
        const __m128 vCol2X = _mm_set1_ps(pMat[6]);
        const __m128 vCol2Y = _mm_set1_ps(pMat[7]);
        const __m128 vCol2Z = _mm_set1_ps(pMat[8]);

        const __m128 vExtent0 = _mm_set1_ps(pVolume->m_vCenter.x);
        const __m128 vExtent1 = _mm_set1_ps(pVolume->m_vCenter.y);
        const __m128 vExtent2 = _mm_set1_ps(pVolume->m_vCenter.z);

        const __m128* pPlane = reinterpret_cast<const __m128*>(m_Planes);
        const __m128* pPlaneEnd = reinterpret_cast<const __m128*>(m_Planes + 4 * m_iNumUserPlanes);

        // Sphere test against the camera planes.
        while (pPlane != pPlaneEnd)
        {
            const __m128 vDot = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCenterX, pPlane[0]), _mm_mul_ps(vCenterY, pPlane[1])), _mm_mul_ps(vCenterZ, pPlane[2]));
            const __m128 vTest = _mm_sub_ps(_mm_sub_ps(vRadius, pPlane[3]), vDot);
            if (_mm_movemask_ps(vTest))
            {
                return false;
            }
            pPlane += 4;
        }

        // Oriented bounding box test against the camera planes.
        for (const __m128* i = reinterpret_cast<const __m128*>(m_Planes); i != pPlaneEnd; i += 4)
        {
            const __m128 dotCol0 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCol0X, i[0]), _mm_mul_ps(vCol0Y, i[1])), _mm_mul_ps(vCol0Z, i[2]));
            const __m128 dotCol1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCol1X, i[0]), _mm_mul_ps(vCol1Y, i[1])), _mm_mul_ps(vCol1Z, i[2]));
            const __m128 dotCol2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCol2X, i[0]), _mm_mul_ps(vCol2Y, i[1])), _mm_mul_ps(vCol2Z, i[2]));

            const __m128 term0 = _mm_and_ps(_mm_mul_ps(vExtent0, dotCol2), absMask);
            const __m128 term1 = _mm_and_ps(_mm_mul_ps(vExtent1, dotCol1), absMask);
            const __m128 term2 = _mm_and_ps(_mm_mul_ps(vExtent2, dotCol0), absMask);

            const __m128 center = _mm_add_ps(_mm_add_ps(_mm_add_ps(i[3], _mm_mul_ps(vCenterX, i[0])), _mm_mul_ps(vCenterY, i[1])), _mm_mul_ps(vCenterZ, i[2]));

            const __m128 result = _mm_add_ps(_mm_add_ps(term0, term1), _mm_add_ps(term2, center));
            if (_mm_movemask_ps(result))
            {
                return false;
            }
        }

        if (m_iNumExists == 0)
        {
            return true;
        }

        // Exit portal test: visible if fully inside any exit's four side planes.
        const __m128* pExitEnd = pPlaneEnd + (static_cast<size_t>(m_iNumExists) * 4);
        for (const __m128* i = pPlaneEnd; i != pExitEnd; i += 4)
        {
            const __m128 vDot = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCenterX, i[0]), _mm_mul_ps(vCenterY, i[1])), _mm_mul_ps(vCenterZ, i[2]));
            const __m128 vSphere = _mm_sub_ps(_mm_sub_ps(vRadius, i[3]), vDot);
            if (_mm_movemask_ps(vSphere))
            {
                continue;
            }

            const __m128 dotCol0 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCol0X, i[0]), _mm_mul_ps(vCol0Y, i[1])), _mm_mul_ps(vCol0Z, i[2]));
            const __m128 dotCol1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCol1X, i[0]), _mm_mul_ps(vCol1Y, i[1])), _mm_mul_ps(vCol1Z, i[2]));
            const __m128 dotCol2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(vCol2X, i[0]), _mm_mul_ps(vCol2Y, i[1])), _mm_mul_ps(vCol2Z, i[2]));

            const __m128 term0 = _mm_and_ps(_mm_mul_ps(vExtent0, dotCol2), absMask);
            const __m128 term1 = _mm_and_ps(_mm_mul_ps(vExtent1, dotCol1), absMask);
            const __m128 term2 = _mm_and_ps(_mm_mul_ps(vExtent2, dotCol0), absMask);

            const __m128 center = _mm_add_ps(_mm_add_ps(_mm_add_ps(i[3], _mm_mul_ps(vCenterX, i[0])), _mm_mul_ps(vCenterY, i[1])), _mm_mul_ps(vCenterZ, i[2]));

            const __m128 result = _mm_add_ps(_mm_add_ps(term0, term1), _mm_add_ps(term2, center));
            if (!_mm_movemask_ps(result))
            {
                return true;
            }
        }

        return false;
    }

    void ZViewSpace::CheckAndAddGeomsInRoom(ZVolumeList* pVolumeList, const float* pPlanes, uint32_t lNrPlanes, ZVisibleRoom* pRoom, bool bLightsEnabled, bool bBackdrop)
    {
        ZROOM* pRoomGeom = pRoom->m_pRoom;

        if (pRoom->m_lNumExits > 0x40)
        {
            return;
        }

        ZRoomGeomVolumes::ZHeader* pHeader = m_RoomGeomVolumes.GetVolumesHeader(pRoomGeom, m_bExitsEnabled);
        if (!pHeader)
        {
            return;
        }

        ZBaseGeomVolume* pVolumes = reinterpret_cast<ZBaseGeomVolume*>(pHeader + 1);
        const uint32_t lNumGeoms = pHeader->m_lNumGeoms;

        ZMat3x3 mMat;
        mMat.Reset();
        ZVector3 vNegPos = m_vPosition;
        vneg(vNegPos.Get());
        InitVisibCheck(pPlanes, static_cast<int>(lNrPlanes), pRoom->m_pFirstExit, mMat.Get(), vNegPos.Get());

        ZBaseGeomVolume** pList = pVolumeList->BeginRoom(pRoomGeom);
        uint32_t lCount = 0;

        for (uint32_t i = 0; i < lNumGeoms; ++i)
        {
            if (IsVisible(&pVolumes[i]))
            {
                pList[lCount++] = &pVolumes[i];
            }
        }

        // TODO: Finish me (PC 0047C2E0): reorder/emit lights, environments and the
        // IDraw AllocateDeviceBuffers backdrop path separately.

        pVolumeList->EndGroup(lCount);
    }

    void ZViewSpace::GetVisibleVolumes(ZVolumeList* pVolumeList, bool bLightsEnabled, ZVolumeList* pVolumeListCheck)
    {
        LocateRoomsAndExits();

        float vPlanes[48];
        const uint32_t lNrPlanes = GetEnabledClipPlanes(vPlanes);

        for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
        {
            CheckAndAddGeomsInRoom(pVolumeList, vPlanes, lNrPlanes, &m_Rooms.m_Array[i], bLightsEnabled, false);
        }

        for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
        {
            m_Rooms.m_Array[i].m_pRoom->m_pTempRoom = nullptr;
        }
    }

    bool ZViewSpace::GetVisibleVolumesIncludingBackdrop(ZVolumeList* pVolumeList, ZRenderEntry* pRenderEntry, bool bLightsEnabled)
    {
        float vPlanes[48];
        const uint32_t lNrPlanes = GetEnabledClipPlanes(vPlanes);

        if (!m_pTopNode || m_pTopNode->IsDerivedFrom<ZROOM>())
        {
            LocateRoomsAndExits();

            // TODO: Finish me (PC 0047C6F0): collect the room's attached draw base
            // geoms (m_rAttachedDrawBaseGeoms) and their render entries (IDraw),
            // then re-emit them as backdrop volumes after disabling clip plane 1.

            for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
            {
                CheckAndAddGeomsInRoom(pVolumeList, vPlanes, lNrPlanes, &m_Rooms.m_Array[i], bLightsEnabled, false);
            }

            for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
            {
                m_Rooms.m_Array[i].m_pRoom->m_pTempRoom = nullptr;
            }

            return false;
        }

        if (!m_bExitsEnabled)
        {
            ZRoomGeomVolumes::ZHeader* pHeader = m_RoomGeomVolumes.CreateGroupVolumes(m_pTopNode);
            if (pHeader)
            {
                ZBaseGeomVolume** pList = pVolumeList->BeginGroup(m_pTopNode);
                ZBaseGeomVolume* pVolumes = reinterpret_cast<ZBaseGeomVolume*>(pHeader + 1);
                for (uint32_t i = 0; i < pHeader->m_lNumGeoms; ++i)
                {
                    pList[i] = &pVolumes[i];
                }
                pVolumeList->EndGroup(pHeader->m_lNumGeoms);
            }
        }

        return m_bExitsEnabled;
    }
}
