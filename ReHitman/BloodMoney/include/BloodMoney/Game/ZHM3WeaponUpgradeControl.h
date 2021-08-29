#pragma once

#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Hitman::BloodMoney {
    class ZHM3ItemWeaponCustom;

    enum class EWeaponType {
        EW_PISTOL          = 1,
        EW_ASSAULT_RIFLE   = 2,
        EW_SUB_MACHINE_GUN = 3,
        EW_SHOTGUN         = 4,
        EW_UNKNOWN         = 0
    };

    class ZHM3WeaponUpgradeControl : public Glacier::ZGROUP
    {
    public:
        // vtbl (no ext methods)

        // api
        void InitWeaponReferences();
        void ApplyDefaultUpgrades(EWeaponType weaponType, ZHM3ItemWeaponCustom* pCustomGun);

        // static methods
        static EWeaponType GetWeaponType(const char* psWeaponName);

        // data (total size is 0x60, base size is 0x4C)
        int m_field4C; //Pointer to array of possible weapon upgrade groups (as I understand)
        Glacier::ZGEOM* m_pCustomGunHardballerUpgradeWeapon;
        Glacier::ZGEOM* m_pCustomRifleAssaultUpgradeWeapon;
        Glacier::ZGEOM* m_pCusomSmgUpgradeWeapon;
        Glacier::ZGEOM* m_pCustomRifleShotgunUpgradeWeapon;
    };
}