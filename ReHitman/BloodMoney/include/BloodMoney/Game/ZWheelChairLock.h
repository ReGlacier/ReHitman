#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Hitman::BloodMoney {
    class ZHM3Actor;

    class ZWheelChairLock : public Glacier::CBaseEvent<Glacier::ZGEOM> {
    public:
        //static
        static constexpr Glacier::ZMSGID kDeactivateFrameUpdate = 0x915;

        //vftable (no changes)
        //api
        //data (total size is 0x3C, base size is 0x30)
        Glacier::ZREF m_rActor;
        Glacier::ZLNKWHANDS* m_pActor;
        uint16_t m_iBoneId;
        RE_ADD_PADDING(2);
    }; // size is 0x3C
    RE_VERIFY_SIZE(ZWheelChairLock, 0x3C); // verified
}