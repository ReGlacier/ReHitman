#include <Glacier/GameBase/ZFightController.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ScriptEngine/ScriptMessages.h>
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/IK/ZLNKWHANDS.h>


namespace Glacier
{
    ZFightController::sAttacker::sAttacker() = default;

    void ZFightController::sAttacker::LoadSave(ISerializerStream& stream)
    {
        auto statusval = static_cast<char>(status);
        auto styleval = static_cast<char>(style);

        stream.Exchange("rActor", rActor);
        stream.Exchange("iChannel", iChannel);
        stream.Exchange("status", statusval);
        stream.Exchange("style", styleval);
        stream.Exchange("rTarget", rTarget);
        stream.Exchange("rTargetDress", rTargetDress);

        status = static_cast<eStatus>(statusval);
        style = static_cast<eAttackStyle>(styleval);
    }

    ZFightController::ZFightController()
        : CBaseEvent<ZBoxPrimitive>()
        , m_PosManager()
    {
        pRadio = nullptr;
        lUnknown30 = 0; // ???
        iNrOfEngagedActors = 0;
        m_fLastRequestAssistanceTime = 0.0f;
    }

    ZFightController::~ZFightController() = default;

    void ZFightController::PostSave(ISerializerStream& stream)
    {
        // TODO: Finish me
    }

    bool ZFightController::PostLoad(ISerializerStream& stream)
    {
        bool bRes = ZEventBase::PostLoad(stream);
        auto iCurrentAttackStyle = static_cast<char>(m_eCurrentAttackStyle);

        if (!stream.TestStreamFilter(ISerializerStream::CONTENT_SimpleRepack))
            return bRes;

        stream.Exchange("bRes", iCurrentAttackStyle);
        m_eCurrentAttackStyle = static_cast<eAttackStyle>(iCurrentAttackStyle);

        if (iNrOfEngagedActors <= 0)
            return bRes;

        for (int i = 0; i < iNrOfEngagedActors; ++i)
        {
            aAttackers[i].LoadSave(stream);
        }

        return bRes;
    }

    const RTP::ZPropertyInfo& ZFightController::GetProperties() const
    {
        return ZFightController::Info;
    }

    void ZFightController::Init()
    {
        // TODO: Finish me after ZGameData reversed
    }

    void ZFightController::Init2()
    {
        // TODO: Finish me after ZActorCommunication finished
    }

    void ZFightController::TimeUpdate()
    {
        if (iNrOfEngagedActors)
        {
            // TODO: Finish me
        }
        else
        {
            m_eCurrentAttackStyle = eAttackStyle_AGGRESIVE;
            DeactivateFrameUpdate();

            // TODO: Finish me after ZGameData reversed
        }
    }

    void ZFightController::RequestNewPosition(ZREF rActor)
    {
        // TODO: Finish me
    }

    bool ZFightController::IsActorInList(ZGEOM* pActor) const
    {
        // TODO: Finish me
        return false;
    }

    int32_t ZFightController::AddActor(ZREF rActor, int8_t channel, eStatus status, ZREF rTarget)
    {
        // TODO: Finish me
        return -1;
    }

    void ZFightController::AttackingTarget(int channel, ZREF rActor, ZREF rTarget)
    {
        if (iNrOfEngagedActors != MAX_MEMBERS_NR)
        {
            auto* pActor = ref_cast<ZLNKWHANDS>(rActor);
            ZASSERT(pActor);

            auto lIndex = AddActor(rActor, channel, ENGAGED, rTarget);
            // TODO: Finish me
        }
    }

    void ZFightController::RemoveActor(ZREF rActor)
    {
        // TODO: Finish me
    }

    void ZFightController::SignOff(ZREF rActor)
    {
        RemoveActor(rActor);
    }

    void ZFightController::UpdateTargetInfo(ZREF rActor, ZREF rNewTarget, ZREF rNewTargetDress)
    {
        // TODO: Finish me
    }

    ZREF ZFightController::QueryTarget(ZREF rActor, int channel)
    {
        if (iNrOfEngagedActors <= 0)
            return 0;

        for (int i = 0; i < iNrOfEngagedActors; ++i)
        {
            if (aAttackers[i].rActor == rActor || (aAttackers[i].iChannel == channel && channel != 999))
            {
                return aAttackers[i].rTarget;
            }
        }

        return 0;
    }

    float ZFightController::GetTotalStrength(ZREF rTarget)
    {
        float fResult = 0.0f;

        for (int i = 0; i < iNrOfEngagedActors; ++i)
        {
            if (aAttackers[i].rTarget == rTarget)
            {
                auto* pAttacker = ref_cast<ZLNKWHANDS>(aAttackers[i].rActor);
                ZASSERT(pAttacker);

                fResult += pAttacker->GetCombatStrength();
            }
        }

        return fResult;
    }

    void ZFightController::NotifyAttackersBackupRequested()
    {
        if (iNrOfEngagedActors <= 0)
            return;

        for (int i = 0; i < iNrOfEngagedActors; ++i)
        {
            if (aAttackers[i].status != eStatus::ENGAGED)
            {
                continue;
            }

            auto* pActor = ref_cast<ZLNKWHANDS>(aAttackers[i].rActor);
            ZASSERT(pActor);

            ScriptSendCommand(pActor, MSG_FightController_BackupRequested, nullptr);
        }
    }

    int ZFightController::GetCountOfAttackers(ZREF rTarget, ZREF rTargetDress)
    {
        int iFound = 0;

        for (int i = 0; i < iNrOfEngagedActors; ++i)
        {
            iFound += static_cast<int>(aAttackers[i].rTarget == rTarget && aAttackers[i].rTargetDress == rTargetDress);
        }

        return iFound;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // TODO: Finish me
    }

    DEFINE_ROUT_CLASS(ZFightController, ZBoxPrimitive, FightController, 8, 0, 0x00814A50, nullptr, ZBoxPrimitive); // TODO: Finish RTTI props
    #   pragma endregion
}
