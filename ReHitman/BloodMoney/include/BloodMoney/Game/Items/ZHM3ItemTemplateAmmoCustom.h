#pragma once

#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmo.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateAmmoCustom : public ZHM3ItemTemplateAmmo
    {
    public:
        // data (total size is 0xC0, ZHM3ItemTemplateAmmo size is 0xA8)
        int m_fieldA8;
        int m_fieldAC;
        int m_fieldB0;
        int m_fieldB4;
        int m_fieldB8;
        int m_fieldBC;
    };
}