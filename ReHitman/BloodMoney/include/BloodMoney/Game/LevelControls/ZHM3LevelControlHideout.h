#pragma once

#include <Glacier/ReGlacier.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    class ZHM3LevelControlHideout : public ZHM3LevelControl
    {
    public:
        // vtbl (no changes)
        // data
        EHM3ItemType m_eAvailableAmmoType[15]; // 0x5DC
        int m_iNumberOfAmmoTypes; // +0x614
    };
    RE_VERIFY_SIZE(ZHM3LevelControlHideout, 0x618); // Verified
}