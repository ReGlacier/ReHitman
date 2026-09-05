#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZBoxPrimitive.h>
#include <Glacier/ScriptEngine/Common.h> // eAttackStyle
#include <Glacier/GameBase/AttackerPosManager.h>
#include <Glacier/Runtime/Macro.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZActorCommunication;

    class ZFightController : public CBaseEvent<ZBoxPrimitive>
    {
    public:
        // RTTI
        DECLARE_ROUT_CLASS(ZFightController, ZBoxPrimitive, FightController, 8, 0);

        // constants
        static constexpr int MAX_MEMBERS_NR = 50;

        // types
        enum eStatus
        {
            ENGAGED = 0,
            HUNTING = 1,
            STANDBY = 2,
        };

        struct sAttacker
        {
            // methods
            sAttacker();

            void LoadSave(ISerializerStream& stream);

            // members
            ZREF rActor = 0;       // +0x0
            uint32_t iChannel = 0; // +0x4
            eStatus status = eStatus::ENGAGED;    // +0x8
            eAttackStyle style = eAttackStyle::eAttackStyle_RETREAT;  // +0xC
            ZREF rTarget;      // +0x10
            ZREF rTargetDress; // +0x14
        };
        RE_VERIFY_SIZE(sAttacker, 0x18); // Verified PC ZFightController::PostLoad

        // vtbl
        ~ZFightController() override;

        // ZSerializable
        void PostSave(ISerializerStream& stream) override;
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZEventBase
        void Init() override;
        void Init2() override;
        void TimeUpdate() override;

        // methods
        ZFightController();

        void RequestNewPosition(ZREF rActor);
        bool IsActorInList(ZGEOM* pActor) const;
        int32_t AddActor(ZREF rActor, int8_t channel, eStatus status, ZREF rTarget);
        void AttackingTarget(int channel, ZREF rActor, ZREF rTarget);
        void RemoveActor(ZREF rActor);
        void SignOff(ZREF rActor);
        void UpdateTargetInfo(ZREF rActor, ZREF rNewTarget, ZREF rNewTargetDress);
        ZREF QueryTarget(ZREF rActor, int channel);
        float GetTotalStrength(ZREF rTarget);
        void NotifyAttackersBackupRequested();
        int GetCountOfAttackers(ZREF rTarget, ZREF rTargetDress);

        // members
        uint32_t lUnknown30;
        ZActorCommunication* pRadio;
        sAttacker aAttackers[MAX_MEMBERS_NR];
        int32_t iNrOfEngagedActors;
        eAttackStyle m_eCurrentAttackStyle;
        AttackerPosManager m_PosManager;
        float m_fLastRequestAssistanceTime;
    };
    RE_VERIFY_SIZE(ZFightController, 0x534u); // Verified PC alloc
}
