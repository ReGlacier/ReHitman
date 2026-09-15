#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemBomb : public ZHM3ItemWeapon
    {
    public:
        // api
        void Explode();

        // data (total size is 0x2A0, base size is 0x15C)
        bool m_bExploded;
        bool m_bExploding;
        bool m_bTimerActivated;
        RE_ADD_PADDING(1);
        uint32_t m_iNumberOfTargets;
        Glacier::ZBaseGeom* m_pTargets[30];
        uint32_t m_iTargetCheck;
        Glacier::TIMETYPE m_ttTimeToExplode;
        uint32_t m_iNumberOfLines;
        Glacier::REFTAB32 m_Lines;
        Glacier::ZGEOM* m_pPlaceBombActionGeom;
        uint32_t m_nTopEvCamId;
        Glacier::TIMETYPE m_ttTopViewCamStartTime;

    };
    RE_VERIFY_SIZE(ZHM3ItemBomb, 0x2A0); // Verified
}