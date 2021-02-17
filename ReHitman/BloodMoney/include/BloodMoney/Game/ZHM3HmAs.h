#pragma once

#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <BloodMoney/Game/LevelControls/ESecurityZone.h>

namespace Hitman::BloodMoney
{
    class ZHM3HmAs : public Glacier::ZGROUP
    {
    public:
        /// vftable
        virtual void CopyDataFromHmAs(ZHM3HmAs* from);
        virtual bool IsZoneAllowed(ESecurityZone zoneKind);

        /// data (total size is 0xA0, parent size is 0x4C)
        int m_field4C; //copied at CopyDataFromHmAs
        int m_field50;
        Glacier::REFTAB m_reftab54;
        int m_field70;
        uint32_t m_allowedRegionsMask;
        Glacier::REFTAB m_reftab78;
        int m_field94;
        int m_field98;
        int m_field9C;
    };
}