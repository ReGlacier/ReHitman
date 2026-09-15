#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    class ZFireAlarm : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
        /// vftable - no custom vftable
        /// data (total size : 0x48, ZEventBase size: 0x30)
        Glacier::ZREF m_rAlarmArea;
        Glacier::ZREF m_rPanicArea;
        int m_iTime;
        Glacier::REFTAB* m_pSwitches;
        Glacier::REFTAB* m_pBells;
        float m_fTimeOutCounter;
    };
    RE_VERIFY_SIZE(ZFireAlarm, 0x48); // Verified
}