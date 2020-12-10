#pragma once

#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemWeaponCustomTemplate : public ZHM3ItemTemplateWeapon
    {
    public:
        /// vftable
        virtual void* GetCustomData();
        virtual void SetMuzzleVelocity(float);

        /// data (total size is 0x1FC, base size is 0x1A0)
        int m_field1A0;
        int m_field1A4;
        int m_field1A8;
        int m_customData;
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
        int m_field1E8;
        int m_field1EC;
        int m_field1F0;
        int m_field1F4;
        int m_field1F8;
    };
}