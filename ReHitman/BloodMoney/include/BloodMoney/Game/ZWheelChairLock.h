#pragma once

#include <Glacier/EventBase/ZEventBase.h>

namespace Hitman::BloodMoney {
    class ZHM3Actor;

    class ZWheelChairLock : public Glacier::ZEventBase {
    public:
        //static
        static constexpr Glacier::ZMSGID kDeactivateFrameUpdate = 0x915;

        //vftable (no changes)
        //api
        //data (total size is 0x3C, base size is 0x30)
        int m_rActorREF;
        ZHM3Actor* m_pAttachedActor;
        short m_rAnimation;
        short m_field3A;
    };
}