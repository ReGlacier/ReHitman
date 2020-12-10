#pragma once

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
}