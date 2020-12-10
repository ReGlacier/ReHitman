#pragma once

#include <Glacier/REFTAB32.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemBomb : public ZHM3ItemWeapon
    {
    public:
        // api
        void Explode();

        // data (total size is 0x2A0, base size is 0x15C)
        bool m_field15C;
        bool field_15D;
        bool field_15E;
        bool field_15F;
        int m_field160;
        int m_field164;
        int m_field168;
        int m_field16C;
        int m_field170;
        int m_field174;
        int m_field178;
        int m_field17C;
        int m_field180;
        int m_field184;
        int m_field188;
        int m_field18C;
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
        int m_field1A0;
        int m_field1A4;
        int m_field1A8;
        int m_field1AC;
        int m_field1B0;
        int m_field1B4;
        int m_field1B8;
        int m_field1BC;
        int m_field1C0;
        int m_field1C4;
        int m_field1C8;
        int m_field1CC;
        int m_field1D0;
        int m_field1D4;
        int m_field1D8;
        int m_field1DC;
        int m_field1E0;
        int m_field1E4;
        Glacier::REFTAB32 m_reftab32;
        int m_field294;
        int m_field298;
        int m_field29C;
    };
}