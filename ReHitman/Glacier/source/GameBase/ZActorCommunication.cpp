#include <Glacier/GameBase/ZActorCommunication.h>
#include <Glacier/GameBase/ZActor.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEntityTracker.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>


namespace Glacier
{
    ZActorCommunication::ZActorCommunication()
        : CBaseEvent<ZBoxPrimitive>()
    {
        m_iNrOfRadioUsers = 0;
        m_pActorList = nullptr;
        memset(m_aRadioUsers, 0, sizeof(m_aRadioUsers));
    }

    ZActorCommunication::~ZActorCommunication() = default;

    void ZActorCommunication::sRadioOwner::LoadSave(ISerializerStream& stream)
    {
        stream.Exchange("rUser", rUser);

        auto iChannelValue = static_cast<char>(iChannel);
        stream.Exchange("iChannel", iChannelValue);
        iChannel = static_cast<uint8_t>(iChannelValue);
    }

    bool ZActorCommunication::PostLoad(ISerializerStream& stream)
    {
        const bool bResult = ZEventBase::PostLoad(stream);

        if (!stream.TestStreamFilter(ISerializerStream::CONTENT_SimpleRepack))
        {
            return bResult;
        }

        for (int32_t i = 0; i < m_iNrOfRadioUsers; ++i)
        {
            m_aRadioUsers[i].LoadSave(stream);
        }

        return bResult;
    }

    void ZActorCommunication::PostSave(ISerializerStream& stream)
    {
        if (!stream.TestStreamFilter(ISerializerStream::CONTENT_SimpleRepack))
        {
            return;
        }

        for (int32_t i = 0; i < m_iNrOfRadioUsers; ++i)
        {
            m_aRadioUsers[i].LoadSave(stream);
        }
    }

    RTP::ZPropertyInfo& ZActorCommunication::GetProperties() const
    {
        return ZActorCommunication::Info;
    }

    void ZActorCommunication::Init()
    {
        // The original stores this event's ref in ZHM3GameData::m_rActorCommunication
        // (BloodMoney/Game/ZHM3GameData.h, +0x6990). That type is not reachable from the
        // Glacier module, so the documented raw slot is used instead.
        *reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(g_pGameData) + 0x6990) = GetRef();
    }

    void ZActorCommunication::Init2()
    {
        m_pActorList = ZLIST::m_TrackLinkObjectsInstance;
    }

    void ZActorCommunication::CopyData(const ZEventBase* Source)
    {
        // Empty in every build: the compiler folds it onto the shared no-op vtable slot.
        (void)Source;
    }

    void ZActorCommunication::RegisterRadioUser(ZREF rActor, uint32_t iChannel)
    {
        if (m_iNrOfRadioUsers >= 100)
        {
            ZASSERT(false); // Couldn't register any more radioUsers, in ZActorCommunication
            return;
        }

        m_aRadioUsers[m_iNrOfRadioUsers].rUser = rActor;
        m_aRadioUsers[m_iNrOfRadioUsers].iChannel = static_cast<uint8_t>(iChannel);
        ++m_iNrOfRadioUsers;

        // The original asserts the receiver is a ZHM3Actor; that BloodMoney type is not
        // reachable from Glacier, so the closest available base type is used.
        ZGEOM* pGeom = ZGEOM::RefToPtr(rActor);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZActor>()); // non actor registered as radio user
    }

    bool ZActorCommunication::IsReceiverDead(ZGEOM* pGeom) const
    {
        if (!pGeom)
        {
            return true;
        }

        if (!pGeom->IsDerivedFrom<ZActor>())
        {
            return false;
        }

        return geom_cast<ZActor>(pGeom)->IsDead();
    }

    void ZActorCommunication::CalculateDistances(ZREF rActor, int iChannel, float* pOrigin, float* pDistances)
    {
        for (int32_t i = 0; i < m_iNrOfRadioUsers; ++i)
        {
            pDistances[i] = -1.0f;

            if (iChannel != static_cast<int8_t>(m_aRadioUsers[i].iChannel) || rActor == m_aRadioUsers[i].rUser)
            {
                continue;
            }

            ZGEOM* pReceiver = ZGEOM::RefToPtr(m_aRadioUsers[i].rUser);
            if (!pReceiver || IsReceiverDead(pReceiver))
            {
                continue;
            }

            ZVector3 vReceiverPos;
            vreset(vReceiverPos);
            pReceiver->GetRootPoint(vReceiverPos);

            pDistances[i] = vdist(pOrigin, vReceiverPos);
        }
    }

    void ZActorCommunication::SendRadioMessage(ZREF rActor, int iChannel, uint16_t message, void* pData)
    {
        for (int32_t i = 0; i < m_iNrOfRadioUsers; ++i)
        {
            if (iChannel != static_cast<int8_t>(m_aRadioUsers[i].iChannel) || rActor == m_aRadioUsers[i].rUser)
            {
                continue;
            }

            ZGEOM* pReceiver = ZGEOM::RefToPtr(m_aRadioUsers[i].rUser);
            if (pReceiver && !IsReceiverDead(pReceiver))
            {
                pReceiver->SendCommand(message, pData, nullptr);
            }
        }
    }

    void ZActorCommunication::SendRadioMessageToClosestN(ZREF rActor, int iChannel, int iNrOfReceivers, uint16_t message, void* pData)
    {
        ZGEOM* pSender = ZGEOM::RefToPtr(rActor);
        if (!pSender)
        {
            return;
        }

        ZVector3 vSenderPos;
        vreset(vSenderPos);
        pSender->GetRootPoint(vSenderPos);

        float aDistances[100];
        CalculateDistances(rActor, iChannel, vSenderPos, aDistances);

        if (iNrOfReceivers <= 0)
        {
            return;
        }

        int32_t iSent = 0;
        do
        {
            bool bFound = false;
            int32_t iClosest = 0;
            float fClosestDistance = 1.0e20f;

            for (int32_t i = 0; i < m_iNrOfRadioUsers; ++i)
            {
                if (aDistances[i] != -1.0f && fClosestDistance > aDistances[i])
                {
                    bFound = true;
                    fClosestDistance = aDistances[i];
                    iClosest = i;
                }
            }

            if (!bFound)
            {
                break;
            }

            const ZREF rReceiver = m_aRadioUsers[iClosest].rUser;
            aDistances[iClosest] = -1.0f;

            ZGEOM* pReceiver = ZGEOM::RefToPtr(rReceiver);
            if (pReceiver)
            {
                pReceiver->SendCommand(message, pData, nullptr);
            }

            ++iSent;
        } while (iSent < iNrOfReceivers);
    }

    float ZActorCommunication::SendRadioMessageToNeededForce(ZREF rActor, int iChannel, float fForce, uint16_t message, void* pData)
    {
        ZGEOM* pSender = ZGEOM::RefToPtr(rActor);
        if (!pSender)
        {
            return 0.0f;
        }

        ZVector3 vSenderPos;
        vreset(vSenderPos);
        pSender->GetRootPoint(vSenderPos);

        float aDistances[100];
        CalculateDistances(rActor, iChannel, vSenderPos, aDistances);

        float fTotalForce = 0.0f;
        if (fForce <= 0.0f)
        {
            return fTotalForce;
        }

        bool bFound = true;
        while (bFound)
        {
            bFound = false;
            int32_t iClosest = 0;
            float fClosestDistance = 1.0e20f;

            for (int32_t i = 0; i < m_iNrOfRadioUsers; ++i)
            {
                if (aDistances[i] != -1.0f && fClosestDistance > aDistances[i])
                {
                    bFound = true;
                    fClosestDistance = aDistances[i];
                    iClosest = i;
                }
            }

            if (bFound)
            {
                const ZREF rReceiver = m_aRadioUsers[iClosest].rUser;
                aDistances[iClosest] = -1.0f;

                ZGEOM* pReceiver = ZGEOM::RefToPtr(rReceiver);
                if (pReceiver && pReceiver->IsDerivedFrom<ZActor>())
                {
                    // The original reads a ZHM3Actor-level virtual here. That type is not
                    // reachable from Glacier, so the shared combat-strength accessor is used.
                    const float fStrength = geom_cast<ZLNKWHANDS>(pReceiver)->GetCombatStrength();
                    if (fStrength != 0.0f)
                    {
                        pReceiver->SendCommand(message, pData, nullptr);
                        fTotalForce += fStrength;
                    }
                }
            }

            if (fTotalForce >= fForce)
            {
                break;
            }
        }

        return fTotalForce;
    }

    void ZActorCommunication::SendRangedMessage(ZREF rActor, float fRange, uint16_t message, void* pData)
    {
        ZGEOM* pSender = ZGEOM::RefToPtr(rActor);
        if (!pSender)
        {
            return;
        }

        ZVector3 vSenderPos;
        vreset(vSenderPos);
        pSender->GetRootPoint(vSenderPos);

        if (!m_pActorList || !m_pActorList->m_pZList)
        {
            return;
        }

        REFTAB* pActors = m_pActorList->m_pZList;
        RefRun run;
        pActors->RunInitNxtRef(&run);
        for (uint32_t rActorRef = pActors->RunNxtRef(&run); run; rActorRef = pActors->RunNxtRef(&run))
        {
            ZGEOM* pReceiver = ZGEOM::RefToPtr(rActorRef);
            if (!pReceiver || rActorRef == rActor || IsReceiverDead(pReceiver))
            {
                continue;
            }

            ZVector3 vReceiverPos;
            vreset(vReceiverPos);
            pReceiver->GetRootPoint(vReceiverPos);

            if (vdist(vSenderPos, vReceiverPos) < fRange)
            {
                pReceiver->SendCommand(message, pData, nullptr);
            }
        }
    }

    void ZActorCommunication::SendGlobalEvent(ZREF rActor, uint16_t message, void* pData)
    {
        for (int32_t i = 0; i < m_iNrOfRadioUsers; ++i)
        {
            if (rActor == m_aRadioUsers[i].rUser)
            {
                continue;
            }

            ZGEOM* pReceiver = ZGEOM::RefToPtr(m_aRadioUsers[i].rUser);
            if (pReceiver && !IsReceiverDead(pReceiver))
            {
                pReceiver->SendCommand(message, pData, nullptr);
            }
        }
    }

    void ZActorCommunication::SendEventToActorsInBox(ZREF rActor, uint16_t message, void* pData)
    {
        ZGEOM* pBox = ZGEOM::RefToPtr(rActor);
        if (!pBox || !pBox->IsDerivedFrom<ZBoxPrimitive>())
        {
            return;
        }

        pBox->CalcCenSize();

        const ZVector3 vSize = pBox->Size();
        const float fRadius = vlen(vSize);

        ZVector3 vWorldPos;
        vreset(vWorldPos);
        pBox->GetWorldPosition(vWorldPos);

        PF4::ZLocation kLocation;
        g_pEngineData->m_pPathfinder4Data->MapLocation(vWorldPos, kLocation);

        ZEntityTracker* pTracker = g_pEngineData->m_pEntityTracker;
        const int32_t iActorType = pTracker->m_ActorTypeId;
        const int32_t iHeroType = pTracker->m_HeroTypeId;

        PF4::ZInterface::ZResult aResults[1000];
        const int32_t iCount = pTracker->GetClosest(kLocation, aResults, 1000, fRadius, -1);

        for (int32_t i = 0; i < iCount; ++i)
        {
            PF4::ZMetaNode* pNode = aResults[i].pNode;
            if (!pNode || (pNode->m_Type != iActorType && pNode->m_Type != iHeroType))
            {
                continue;
            }

            ZGEOM* pTarget = static_cast<ZGEOM*>(pNode->m_Data);
            if (!pTarget)
            {
                continue;
            }

            ZVector3 vTargetPos;
            vreset(vTargetPos);
            pTarget->GetRootPoint(vTargetPos);
            pBox->GetLocalPoint(vTargetPos);

            if (pBox->CheckPointInside(vTargetPos, 0.0f))
            {
                pTarget->SendCommand(message, pData, nullptr);
            }
        }
    }

    void ZActorCommunication::SendEventToActorsInBox2(ZREF rActor, uint16_t message, void* pData)
    {
        ZGEOM* pBox = ZGEOM::RefToPtr(rActor);
        if (!pBox || !pBox->IsDerivedFrom<ZBoxPrimitive>())
        {
            return;
        }

        ZLIST* pList = g_pGameData->m_pTrackLinkObjectList;
        if (!pList || !pList->m_pZList)
        {
            return;
        }

        REFTAB* pObjects = pList->m_pZList;
        RefRun run;
        pObjects->RunInitNxtRef(&run);
        for (uint32_t rObject = pObjects->RunNxtRef(&run); run; rObject = pObjects->RunNxtRef(&run))
        {
            ZGEOM* pTarget = ZGEOM::RefToPtr(rObject);
            if (!pTarget)
            {
                continue;
            }

            ZVector3 vTargetPos;
            vreset(vTargetPos);
            pTarget->GetRootPoint(vTargetPos);
            pBox->GetLocalPoint(vTargetPos);

            if (pBox->CheckPointInside(vTargetPos, 0.0f))
            {
                pTarget->SendCommand(message, pData, nullptr);
            }
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<int> NamespaceItem_1337
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_iNrOfRadioUsers",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__14,
            .m_Offset = CLASS_PROPERTY(ZActorCommunication, m_iNrOfRadioUsers)
        };
    }

    DEFINE_ROUT_CLASS(ZActorCommunication, ZBoxPrimitive, ActorCommunication, 0, 0, 0x00814A70, cProperties::NamespaceItem_1337, ZBoxPrimitive);
#   pragma endregion
}
