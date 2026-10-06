#include <Glacier/ZActorHeroCheckInside.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZCubeGrid.h>
#include <Glacier/ZEntityTracker.h>
#include <Glacier/PF4/ZMetaNode.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    ZActorHeroCheckInside::ZActorHeroCheckInside()
        : CBaseEvent<ZGEOM>()
        , m_rReceiver(0)
        , m_MapLocation()
    {
    }

    bool ZActorHeroCheckInside::PostLoad(ISerializerStream& stream)
    {
        const bool bResult = ZEventBase::PostLoad(stream);
        Init();
        return bResult;
    }

    const RTP::ZPropertyInfo& ZActorHeroCheckInside::GetProperties() const
    {
        return Info;
    }

    void ZActorHeroCheckInside::Init()
    {
        ZGEOM* pGeom = GetGeom();

        // PC 0x5D4BF0: sqrt(x^2 + y^2 + z^2) of the geom size vector, plus a 200 unit margin.
        m_fCheckDist = vlen(pGeom->Size()) + 200.0f;

        ZVector3 vWorldPosition;
        pGeom->GetWorldPosition(vWorldPosition);
        g_pEngineData->m_pPathfinder4Data->MapLocation(vWorldPosition, m_MapLocation);

        m_iActorType = static_cast<uint32_t>(g_pEngineData->m_pEntityTracker->m_ActorTypeId);
        m_iHitmanType = static_cast<uint32_t>(g_pEngineData->m_pEntityTracker->m_HeroTypeId);
        m_iNumberOfResults = 0;
        m_iCurrentCheckIndex = 0;
    }

    void ZActorHeroCheckInside::FrameUpdate()
    {
        if (m_iCurrentCheckIndex < m_iNumberOfResults)
        {
            CheckNearbyActors();
        }
        else
        {
            GetNearbyActors();
        }
    }

    int ZActorHeroCheckInside::Command(ZMSGID command, ZDATA data)
    {
        (void)data;

        if (command == kActivateFrameUpdateMsg)
            ActivateFrameUpdate(false);

        return 0;
    }

    void ZActorHeroCheckInside::GetNearbyActors()
    {
        m_iNumberOfResults = static_cast<uint32_t>(g_pEngineData->m_pEntityTracker->GetClosest(
            m_MapLocation, m_aResults, 32, m_fCheckDist, -1));
        m_iCurrentCheckIndex = 0;
    }

    void ZActorHeroCheckInside::CheckNearbyActors()
    {
        for (uint32_t i = m_iCurrentCheckIndex; i < m_iNumberOfResults; ++i)
        {
            ++m_iCurrentCheckIndex;

            PF4::ZMetaNode* pNode = m_aResults[i].pNode;
            if (pNode->m_Type != static_cast<int>(m_iActorType)
                && pNode->m_Type != static_cast<int>(m_iHitmanType))
            {
                continue;
            }

            auto* pGeom = static_cast<ZGEOM*>(pNode->m_Data);
            if (IsInside(pGeom))
            {
                DeactivateFrameUpdate();

                ZGEOM* pReceiver = ZGEOM::RefToPtr(m_rReceiver);
                ZREF rGeom = pGeom->GetRef();
                pReceiver->SendCommand(kActorHeroCheckInsideEnterMsg, &rGeom, nullptr);
            }
            return;
        }
    }

    bool ZActorHeroCheckInside::IsInside(ZGEOM* pGeom)
    {
        ZVector3 vPoint;
        pGeom->GetCen(vPoint);
        pGeom->GetRootPoint(vPoint);

        ZGEOM* pReceiver = GetGeom();
        pReceiver->GetLocalPoint(vPoint);

        // PC 0x5430A0: a grid receiver takes the non-virtual local-point test rather
        // than the generic CheckPointInside path
        // ((GetObjectId() & ZCubeGrid::m_Mask) == ZCubeGrid::m_Id).
        if (pReceiver->IsDerivedFrom<ZCubeGrid>())
            return reinterpret_cast<ZCubeGrid*>(pReceiver)->IsLocalPointInside(&vPoint.x);

        return pReceiver->CheckPointInside(vPoint, -10.0f);
    }

#   pragma region " --- RTTI --- "
    STATIC_CLASS_VAR_IMPL(ZActorHeroCheckInside, RTP::ZPropertyInfo, Info, 0x00809594, (RTP::ZPropertyInfo {
        // PC 0x00809580 ZActorHeroCheckInside::Property_2C is a ZDataProperty over m_pRoutClassInfo
        // (ZBaseConRout::m_pRoutClassInfo, offset 0x2C); its virtual table has no counterpart in
        // this repository, so the chain is left empty until it can be expressed.
        .First = nullptr,
        .Super = &ZEventBase::Info,
        .Name = nullptr
    }));
#   pragma endregion
}
