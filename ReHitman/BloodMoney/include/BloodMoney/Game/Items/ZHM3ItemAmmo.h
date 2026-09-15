#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Items/ZItemAmmo.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemAmmo : public Glacier::ZItemAmmo
    {
    public:
        // vftable
        virtual EHM3ItemType GetHM3ItemType();
    };
    RE_VERIFY_SIZE(ZHM3ItemAmmo, 0x88); // Verified
}