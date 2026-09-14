#include <Glacier/Render/Entry/ZRenderEntryRoom.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZENVIRONMENT.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderEntryRoom::ZRenderEntryRoom(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryGeom(sInfo)
    {
        m_pStaticVolumesHeader = nullptr;
        m_pMovableGeomVolumes = nullptr;
        m_lNumMovableGeomVolumes = 0;
        m_lLastFrameUpdate = 0;

        auto* pGeom = m_pBaseGeom ? m_pBaseGeom->GetGeom() : nullptr;
        auto* pRoom = pGeom ? static_cast<ZROOM*>(pGeom) : nullptr;
        if (!pRoom)
        {
            return;
        }

        ZBaseGeom* aGeoms[4096];
        ZBaseGeom* aEnvironments[4096];
        uint32_t lNumLights = 0;
        const uint32_t lNumGeoms = GetStaticGeomsInRoom(
            &lNumLights, aGeoms, aEnvironments, pRoom);

        if (!lNumGeoms)
        {
            return;
        }

        auto* pVolumes = static_cast<ZBaseGeomVolume*>(
            ZUniMemory::Allocate(sizeof(ZRoomGeomVolumes::ZHeader) + sizeof(ZBaseGeomVolume) * lNumGeoms));
        if (!pVolumes)
        {
            m_lControl |= RE_CONSTRUCTION_FAILED;
            return;
        }

        m_pStaticVolumesHeader = reinterpret_cast<ZRoomGeomVolumes::ZHeader*>(pVolumes);
        auto* pVolume = reinterpret_cast<ZBaseGeomVolume*>(
            reinterpret_cast<char*>(pVolumes) + sizeof(ZRoomGeomVolumes::ZHeader));
        ZMatrix mRoomMat;
        ZVector3 vRoomPos;
        pGeom->GetRootTM(mRoomMat.m0, vRoomPos);

        uint32_t aMovable[4096];
        uint32_t lNumMovable = 0;
        for (uint32_t i = 0; i < lNumGeoms; ++i, ++pVolume)
        {
            ZBaseGeom* pBaseGeom = aGeoms[i];
            if (pBaseGeom->m_pParent == m_pBaseGeom)
            {
                mmmul(pVolume->m_RootPosition.m0, pBaseGeom->m_mMat, mRoomMat.m0);
                vmmul(pVolume->m_RootPosition.p0.Get(), pBaseGeom->m_vPos, mRoomMat.m0);
                vadd(pVolume->m_RootPosition.p0.Get(), vRoomPos.Get());
            }
            else
            {
                pBaseGeom->GetRootTM(pVolume->m_RootPosition.m0, pVolume->m_RootPosition.p0);
            }

            vmmul(pVolume->m_vCenter, pBaseGeom->m_vCen, pVolume->m_RootPosition.m0);
            vadd(pVolume->m_vCenter, pVolume->m_RootPosition.p0.Get());
            pVolume->m_vSize = pBaseGeom->m_vSize;
            pVolume->m_fRadius = pBaseGeom->m_fRadius;
            pVolume->m_pBaseGeom = pBaseGeom;
            pVolume->m_pBaseGeomEnvironment = aEnvironments[i];

            if (pBaseGeom->IsMovingObject())
            {
                aMovable[lNumMovable++] = i;
            }
        }

        m_pStaticVolumesHeader->m_lNumGeoms = lNumGeoms;
        m_pStaticVolumesHeader->m_lNumLights = lNumLights;
        m_pStaticVolumesHeader->m_lNumEnvironments = 0;
        m_lNumMovableGeomVolumes = lNumMovable;
        if (lNumMovable)
        {
            m_pMovableGeomVolumes = static_cast<ZRoomGeomVolumes**>(
                ZUniMemory::Allocate(sizeof(ZRoomGeomVolumes*) * lNumMovable));
            if (m_pMovableGeomVolumes)
            {
                for (uint32_t i = 0; i < lNumMovable; ++i)
                {
                    m_pMovableGeomVolumes[i] = reinterpret_cast<ZRoomGeomVolumes*>(
                        reinterpret_cast<char*>(m_pStaticVolumesHeader) + sizeof(ZRoomGeomVolumes::ZHeader)
                        + sizeof(ZBaseGeomVolume) * aMovable[i]);
                }
            }
            else
            {
                m_lControl |= RE_CONSTRUCTION_FAILED;
            }
        }
    }

    ZRenderEntryRoom::~ZRenderEntryRoom()
    {
        ZUniMemory::Free(m_pMovableGeomVolumes);
        ZUniMemory::Free(m_pStaticVolumesHeader);
    }

    ZRenderEntryRoom* ZRenderEntryRoom::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        return ZUniMemory::New<ZRenderEntryRoom>(sInfo);
    }

    ZRenderEntryRoom::RENDERENTRY_BASETYPE ZRenderEntryRoom::GetType() const
{
        return ZRenderEntryRoom::RENDERENTRY_BASETYPE::RT_ROOM; // 6
    }

    uint32_t ZRenderEntryRoom::GetStaticGeomsInRoom(uint32_t* pNumLights, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, const ZROOM* pRoom)
    {
        ZBaseGeom* aRanges[4096];
        auto* pMutableRoom = const_cast<ZROOM*>(pRoom);
        ZBaseGeom** pRangesEnd = pMutableRoom->GetStaticPrimDrawGeomsListsRecur(aRanges, aRanges + 4096);
        pRangesEnd = pMutableRoom->GetStaticCustomDrawGeomsListsRecur(pRangesEnd, aRanges + 4096);
        pRangesEnd = pMutableRoom->GetStaticLightsRecur(pRangesEnd, aRanges + 4096);

        uint32_t lNumGeoms = 0;
        for (ZBaseGeom** pRange = aRanges; pRange < pRangesEnd; pRange += 2)
        {
            for (auto* pGeom = *pRange; pGeom <= pRange[1];
                pGeom = reinterpret_cast<ZBaseGeom*>(reinterpret_cast<char*>(pGeom) + sizeof(ZBaseGeom)))
            {
                if ((pGeom->m_lControl & 0x202C00u) == 0
                    && !(pGeom->GetGeom()
                        ? ((pGeom->GetGeom()->GetObjectId() & ZENVIRONMENT::m_Mask) == ZENVIRONMENT::m_Id)
                        : pGeom->IsDerivedFromStdObj(ZENVIRONMENT::m_Id)))
                {
                    pBaseGeoms[lNumGeoms] = pGeom;
                    pBaseGeomEnvironments[lNumGeoms++] = pRoom->m_pEnvironment;
                }
            }
        }

        const uint32_t lTotalGeoms = lNumGeoms;
        uint32_t lNumLights = 0;
        for (uint32_t i = 0; i < lNumGeoms;)
        {
            auto* pGeom = pBaseGeoms[i];
            const bool bLight = (pGeom->GetGeom()
                ? ((pGeom->GetGeom()->GetObjectId() & ZLIGHT::m_Mask) == ZLIGHT::m_Id)
                : pGeom->IsDerivedFromStdObj(ZLIGHT::m_Id));
            if (bLight)
            {
                --lNumGeoms;
                auto* pLastGeom = pBaseGeoms[lNumGeoms];
                pBaseGeoms[lNumGeoms] = pBaseGeoms[i];
                pBaseGeoms[i] = pLastGeom;
                auto* pLastEnvironment = pBaseGeomEnvironments[lNumGeoms];
                pBaseGeomEnvironments[lNumGeoms] = pBaseGeomEnvironments[i];
                pBaseGeomEnvironments[i] = pLastEnvironment;
                ++lNumLights;
                continue;
            }
            ++i;
        }

        *pNumLights = lNumLights;
        return lTotalGeoms;
    }

    void ZRenderEntryRoom::UpdateMovedGeoms()
    {
        const uint32_t lFrame = g_pSysInterface->m_lFrameCount;
        if (m_lLastFrameUpdate == lFrame)
        {
            return;
        }

        m_lLastFrameUpdate = lFrame;
        auto* pGeom = m_pBaseGeom->GetGeom();
        ZMatrix mRoomMat;
        ZVector3 vRoomPos;
        pGeom->GetRootTM(mRoomMat.m0, vRoomPos);
        auto* pRoomBaseGeom = pGeom->BaseGeom();
        for (uint32_t i = 0; i < m_lNumMovableGeomVolumes; ++i)
        {
            auto* pVolume = reinterpret_cast<ZBaseGeomVolume*>(m_pMovableGeomVolumes[i]);
            auto* pBaseGeom = pVolume->m_pBaseGeom;
            if ((pBaseGeom->m_lControl & ZCHASMOVED) != 0 || pBaseGeom->IsMovingObject())
            {
                if (pBaseGeom->m_pParent == pRoomBaseGeom)
                {
                    mmmul(pVolume->m_RootPosition.m0, pBaseGeom->m_mMat, mRoomMat.m0);
                    vmmul(pVolume->m_RootPosition.p0.Get(), pBaseGeom->m_vPos, mRoomMat.m0);
                    vadd(pVolume->m_RootPosition.p0.Get(), vRoomPos.Get());
                }
                else
                {
                    pBaseGeom->GetRootTM(pVolume->m_RootPosition.m0, pVolume->m_RootPosition.p0);
                }
                vmmul(pVolume->m_vCenter, pBaseGeom->m_vCen, pVolume->m_RootPosition.m0);
                vadd(pVolume->m_vCenter, pVolume->m_RootPosition.p0.Get());
            }
        }
    }
}
