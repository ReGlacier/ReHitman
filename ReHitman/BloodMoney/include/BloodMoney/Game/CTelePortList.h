#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZBaseConRout.h>

namespace Hitman::BloodMoney {
    class CTelePortList : public Glacier::ZBaseConRout {
    public:
        // size is 0x3C
        int m_field30;
        int m_field34;
        Glacier::ZREF m_targetREF; // REF to entity who will be teleported to position

        // public API
        void TeleportToPointAtIndex(int index);
    };
    RE_VERIFY_OFFSET(CTelePortList, m_field30, 0x30);
}