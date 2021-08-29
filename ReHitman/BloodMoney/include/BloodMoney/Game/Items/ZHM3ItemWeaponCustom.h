#pragma once

#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemWeaponCustom : public ZHM3ItemWeapon
    {
    public:
        // vftable
        virtual void GetAnims(); //Load anims inside
        virtual void* GetMuzzleVelocity(float);

        // api
        void ApplyUpgrades(char a1);
        void UpdateWeaponPartDrawStatus();
        void ClearUpgrades();

        // data (new size is 0x1C0)
        int m_field15C;
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
        int m_anim_Reload_Bolt;
        int m_anim_Reload_DoubleClip;
        int m_anim_Reload_100shot;
        int m_field1BC;
    };
}