#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/CBaseEvent.h>

namespace Hitman::BloodMoney 
{
    class CTelePortList : public Glacier::CBaseEvent<Glacier::ZLIST> 
    {
    public:
        // size is 0x3C
        int32_t m_iCurPos;
        uint32_t m_iKey;
        Glacier::ZREF m_rObject; // REF to entity who will be teleported to position

        // public API
        void TeleportToPointAtIndex(int index);
    };
    RE_VERIFY_SIZE(CTelePortList, 0x3C); // Verified
    RE_VERIFY_OFFSET(CTelePortList, m_iCurPos, 0x30);
    RE_VERIFY_OFFSET(CTelePortList, m_iKey, 0x34);
}