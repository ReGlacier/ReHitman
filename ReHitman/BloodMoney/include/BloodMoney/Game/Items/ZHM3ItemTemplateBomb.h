#pragma once

#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateBomb : public ZHM3ItemTemplateWeapon
    {
    public:
        // data (total size is 0x1B4, base size is 0x1A0)
        int m_field1A0;
        int m_field1A4;
        int m_field1A8;
        int m_field1AC;
        int m_field1B0;
    };
}