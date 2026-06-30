#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZBitfield.h>
#include <BloodMoney/Game/LevelControls/ESecurityZone.h>


namespace Hitman::BloodMoney
{
    class ZZoneControl : public Glacier::CBaseEvent<Glacier::ZGEOM> 
    {
    public:
        Glacier::ZBitfield<ESecurityZone> m_eZone;
        Glacier::REFTAB                   m_Rooms;
    };
    RE_VERIFY_SIZE(ZZoneControl, 0x50); // Verified

}