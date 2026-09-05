#pragma once

#include <Glacier/Geom/ZBoxPrimitive.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ReGlacier.h>


namespace Hitman::BloodMoney
{
    class CMetalDetector : public Glacier::CBaseEvent<Glacier::ZBoxPrimitive>
    {
    public:
        // vtbl
        // api
        void DoDetectWeapon();
        void DoAlarm();

        // data (total size is 0xA8, ZEventBase size is 0x30)
        Glacier::REFTAB m_rtTargets;
        Glacier::ZAUDIOREF m_rSnd;
        Glacier::REFTAB m_rIgnoredHMAs;
        Glacier::REFTAB m_rIgnoredWeapons;
        Glacier::REFTAB m_rtMetalItems;
        bool m_bInside;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(CMetalDetector, 0xA8); // Verified
}