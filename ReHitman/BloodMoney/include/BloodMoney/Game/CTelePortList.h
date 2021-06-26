#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/EventBase/ZEventBase.h>

namespace Hitman::BloodMoney {
    class CTelePortList : public Glacier::ZEventBase {
    public:
        // size is 0x3C
        int m_field30;
        int m_field34;
        Glacier::ZREF m_targetREF; // REF to entity who will be teleported to position

        // public API
        void TeleportToPointAtIndex(int index);
    };
}