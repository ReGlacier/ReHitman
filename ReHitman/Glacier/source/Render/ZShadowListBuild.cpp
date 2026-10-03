#include <Glacier/Render/ZShadowListBuild.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Prim/ELightType.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZSTL/CListUser.h>
#include <Glacier/Geom/ZENVIRONMENT.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Render/Draw/IDraw.h>


namespace Glacier
{
    void ZShadowListBuild::Init(uint8_t* pxCompiled, int lBufferSize)
    {
        m_pxCompiled = pxCompiled;
        m_pxStore = pxCompiled + sizeof(m_lNumSets);
        m_pxEnd = pxCompiled + lBufferSize;
        m_lNumSets = 0;
        m_lNumCasters = 0;
        m_lNumReceivers = 0;
        m_pSetHeader = nullptr;
        m_pCasterHeader = nullptr;
    }

    int ZShadowListBuild::NewSetBegin(const ZBaseGeom* pLight)
    {
        if (m_pxStore + sizeof(SetHeader) > m_pxEnd)
            return 0;

        m_pSetHeader = reinterpret_cast<SetHeader*>(m_pxStore);
        m_pSetHeader->pLight = pLight;
        m_pSetHeader->pNext = nullptr;
        m_pSetHeader->lNumCasters = 0;
        m_pxStore += sizeof(SetHeader);
        ++m_lNumSets;
        return 1;
    }

    void ZShadowListBuild::NewSetEnd()
    {
        if (m_pSetHeader->lNumCasters)
        {
            m_pSetHeader->pNext = reinterpret_cast<SetHeader*>(m_pxStore);
        }
        else
        {
            m_pxStore -= sizeof(SetHeader);
            --m_lNumSets;
        }
    }

    int ZShadowListBuild::NewCasterBegin(const ZBaseGeom* pCaster)
    {
        if (m_pxStore + sizeof(CasterHeader) > m_pxEnd)
            return 0;

        ++m_pSetHeader->lNumCasters;
        m_pCasterHeader = reinterpret_cast<CasterHeader*>(m_pxStore);
        m_pCasterHeader->pCaster = pCaster;
        m_pCasterHeader->lNumReceivers = 0;
        m_pxStore += sizeof(CasterHeader);
        m_pRecStore = reinterpret_cast<const ZBaseGeom**>(m_pxStore);
        return 1;
    }

    void ZShadowListBuild::RegisterReceiver(const ZBaseGeom* pReceiver)
    {
        if (m_pxStore < m_pxEnd)
        {
            m_pRecStore[m_pCasterHeader->lNumReceivers++] = pReceiver;
            m_pxStore += sizeof(pReceiver);
            ++m_lNumReceivers;
        }
    }

    void ZShadowListBuild::Compile()
    {
        *reinterpret_cast<int*>(m_pxCompiled) = m_lNumSets;
    }

    void ZShadowListBuild::AddReceiver(const ZBaseGeom* pReceiver)
    {
        ZASSERT(m_lNumReceivers < 256);
        m_paReceivers[m_lNumReceivers++] = pReceiver;
        if (m_lNumReceivers > 255)
            m_lNumReceivers = 255;
    }

    void ZShadowListBuild::AddCaster(const ZBaseGeom* pCaster)
    {
        ZASSERT(m_lNumCasters < 128);
        m_paCasters[m_lNumCasters++] = pCaster;
        if (m_lNumCasters > 127)
            m_lNumCasters = 127;
    }

    bool ZShadowListBuild::ValidateCaster(const ZBaseGeom* pCaster) const
    {
        return IDraw::Instance()->ValidateCaster(pCaster);
    }

    bool ZShadowListBuild::ValidateReceiver(const ZBaseGeom* pReceiver) const
    {
        return IDraw::Instance()->ValidateReceiver(pReceiver);
    }

    void ZShadowListBuild::ProcessLight(CListUser* pListUser, ZBaseGeom* pLightBaseGeom)
    {
        if (!pListUser || !pLightBaseGeom || !pLightBaseGeom->m_lDrawId ||
            (pLightBaseGeom->m_lControl & 0x10000u) == 0)
            return;

        uint32_t lNrMembers = 0;
        uint32_t* pMembers = pListUser->UnfoldList(&lNrMembers, pLightBaseGeom->ListId());
        if (!pMembers)
            return;

        m_lNumCasters = 0;
        m_lNumReceivers = 0;

        const auto* pLightPrim = ZPrimHandle{ pLightBaseGeom->m_lPrim }.Get<SPrimLightSpot>();
        if (!pLightPrim)
            return;

        const uint8_t lLightType = pLightPrim->lLightType;
        const bool bSpotLight = lLightType == LTSPOT || lLightType == LTSPOTSQUARE;
        ZMat3x3 mLight;
        ZVector3 vLight;
        pLightBaseGeom->GetRootTM(mLight, vLight);

        for (uint32_t i = 0; i < lNrMembers; ++i)
        {
            auto* pGeom = reinterpret_cast<ZBaseGeom*>(pMembers[i]);
            if (!pGeom)
                continue;

            if (pGeom->IsDerivedFrom<ZIKLNKOBJ>() && (pGeom->m_lControl & 0x10000u) != 0)
            {
                bool bInLight = true;
                if (bSpotLight)
                {
                    ZMat3x3 mGeom;
                    ZVector3 vGeom;
                    pGeom->GetRootMatPos(mGeom, vGeom);

                    const float fDepth = pLightPrim->fFarRange;
                    const float fTan = std::tan(pLightPrim->fFallOff);
                    float fX = fDepth * fTan;
                    float fY = fX;
                    if (lLightType == LTSPOTSQUARE)
                    {
                        const float fAspectRoot = std::sqrt(
                            static_cast<const SPrimLightSpotSquare*>(pLightPrim)->fAspect);
                        fX *= fAspectRoot;
                        fY /= fAspectRoot;
                    }

                    const ZVector3 aLightCorners[4] =
                    {
                        { -fX, -fY, fDepth }, { -fX, fY, fDepth },
                        { fX, fY, fDepth }, { fX, -fY, fDepth }
                    };
                    ZMat3x3 mRelative;
                    mmtmul(mRelative, mLight, mGeom);
                    ZVector3 aCorners[4];
                    for (int j = 0; j < 4; ++j)
                    {
                        aCorners[j] = aLightCorners[j];
                        vmmul(aCorners[j].Get(), mRelative.data);
                    }

                    ZVector3 vApex = vLight - vGeom;
                    vmtmul(vApex.Get(), mGeom.data);
                    const ZVector3 vBounds(
                        pGeom->m_vSize.x * 0.5f, pGeom->m_vSize.y, pGeom->m_vSize.z * 0.5f);
                    ZVector3 aNormals[5];
                    vcross(aNormals[0].Get(), aCorners[0].Get(), aCorners[1].Get());
                    vcross(aNormals[1].Get(), aCorners[2].Get(), aCorners[3].Get());
                    vcross(aNormals[2].Get(), aCorners[1].Get(), aCorners[2].Get());
                    vcross(aNormals[3].Get(), aCorners[3].Get(), aCorners[0].Get());
                    aNormals[4] = ZVector3(-mLight.data[0], -mLight.data[1], -mLight.data[2]);
                    vmtmul(aNormals[4].Get(), mGeom.data);
                    for (int j = 0; j < 5 && bInLight; ++j)
                    {
                        vnorm(aNormals[j].Get());
                        const ZVector3& n = aNormals[j];
                        bInLight = std::fabs(vBounds.x * n.x) + std::fabs(vBounds.y * n.y) +
                            std::fabs(vBounds.z * n.z) + vdot(vApex.Get(), n.Get()) >= 0.0f;
                    }
                }
                if (bInLight)
                    AddCaster(pGeom);
            }
            if ((pGeom->m_lControl & 0x21000u) == 0x21000u && ValidateReceiver(pGeom))
                AddReceiver(pGeom);
        }

        if (!NewSetBegin(pLightBaseGeom))
            return;
        ZVector3 vLightPoint;
        pLightBaseGeom->GetRootPoint(vLightPoint);
        for (int i = 0; i < m_lNumCasters; ++i)
        {
            const ZBaseGeom* pCaster = m_paCasters[i];
            if (!ValidateCaster(pCaster))
                continue;
            ZVector3 vBottom = pCaster->m_vCen;
            ZVector3 vTop = pCaster->m_vCen;
            vBottom.y -= pCaster->m_vSize.y;
            vTop.y += pCaster->m_vSize.y;
            pCaster->GetRootPoint(vBottom);
            pCaster->GetRootPoint(vTop);

            if (!NewCasterBegin(pCaster))
                continue;
            for (int j = 0; j < m_lNumReceivers; ++j)
            {
                const ZBaseGeom* pReceiver = m_paReceivers[j];
                ZVector3 vProjectedBottom = vBottom - vLightPoint;
                ZVector3 vProjectedTop = vTop - vLightPoint;
                vnorm(vProjectedBottom.Get());
                vnorm(vProjectedTop.Get());
                vProjectedBottom = vBottom + vProjectedBottom * 1000.0f;
                vProjectedTop = vTop + vProjectedTop * 1000.0f;

                ZMat3x3 mReceiver;
                ZVector3 vReceiver;
                pReceiver->GetRootTM(mReceiver, vReceiver);
                ZVector3 p0 = vBottom - vReceiver;
                ZVector3 p1 = vTop - vReceiver;
                ZVector3 p2 = vProjectedTop - vReceiver;
                ZVector3 p3 = vProjectedBottom - vReceiver;
                vmtmul(p0.Get(), mReceiver.data);
                vmtmul(p1.Get(), mReceiver.data);
                vmtmul(p2.Get(), mReceiver.data);
                vmtmul(p3.Get(), mReceiver.data);
                p0 -= pReceiver->m_vCen;
                p1 -= pReceiver->m_vCen;
                p2 -= pReceiver->m_vCen;
                p3 -= pReceiver->m_vCen;
                const ZVector3 vSize = pReceiver->m_vSize * 1.2f;
                if (TriangleAABBOverlap(vSize.Get(), p0.Get(), p1.Get(), p2.Get()) ||
                    TriangleAABBOverlap(vSize.Get(), p0.Get(), p2.Get(), p3.Get()))
                    RegisterReceiver(pReceiver);
            }
        }
        NewSetEnd();
    }

    void ZShadowListBuild::ProcessEnvironment(ZStackArray<512, ZRenderEntryGeom*>* pLnkList)
    {
        if (!pLnkList || !ZCollisionBase::s_pCollisionBase)
            return;
        for (uint32_t i = 0; i < pLnkList->Count(); ++i)
        {
            auto* pEntry = *pLnkList->Get(i);
            if (!pEntry)
                continue;
            auto* pGeom = pEntry->GetBaseGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZIKLNKOBJ>() ||
                (pGeom->m_lControl & 0x1000u) == 0 || !ValidateCaster(pGeom))
                continue;
            auto* pParent = pGeom->GetDynamicParent();
            auto* pRooms = pParent ? pParent->GetRoomListPtr() : nullptr;
            if (!pRooms || !pRooms->m_cNrRooms)
                continue;
            auto* pRoom = pRooms->GetRoomNr(0);
            auto* pEnvironment = pRoom ? pRoom->m_pEnvironment : nullptr;
            if (!pEnvironment || (pEnvironment->m_lControl & 0x10000u) == 0)
                continue;
            if (!NewSetBegin(pEnvironment) || !NewCasterBegin(pGeom))
                continue;
            ZVector3 vCenter;
            pGeom->GetCen(vCenter);
            pGeom->GetRootPoint(vCenter);
            ZVector3 vSize;
            pGeom->GetSize(vSize);
            ZMat3x3 mRoom;
            ZVector3 vRoomCenter;
            for (uint32_t r = 0; r < pRooms->m_cNrRooms; ++r)
            {
                auto* pRoomGeom = pRooms->GetRoomNr(r);
                if (!pRoomGeom)
                    continue;

                // 0x46BB10 builds a room-local box from the environment root
                // transform, not from the IK object's transform.
                mRoom = pEnvironment->m_mMat;
                const ZVector3 vEnvironmentAxis(mRoom.data[0], mRoom.data[1], mRoom.data[2]);
                const float fDepth = vSize.z / std::fabs(vEnvironmentAxis.y) * 1.6f - 50.0f;
                vRoomCenter = vCenter + vEnvironmentAxis * fDepth;
                pRoomGeom->GetLocalMatPos(mRoom, vRoomCenter);
                ZBaseGeom* aGeoms[2048] {};
                ZBaseGeom** pEnd = aGeoms + 2048;
                const uint32_t lNr = ZCollisionBase::s_pCollisionBase->GetGeomsInBoxLocal(
                    aGeoms, pEnd, pRoomGeom, GT_StdObjs, mRoom.data,
                    vRoomCenter, vSize, -1, true, false, true);
                for (uint32_t g = 0; g < lNr; ++g)
                    if ((aGeoms[g]->m_lControl & 0x21000u) == 0x21000u && ValidateReceiver(aGeoms[g]))
                        RegisterReceiver(aGeoms[g]);
            }
            NewSetEnd();
        }
    }
}
