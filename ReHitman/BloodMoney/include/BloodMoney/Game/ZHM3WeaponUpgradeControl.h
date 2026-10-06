#pragma once

#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <BloodMoney/Game/Items/EUpgradeType.h>

namespace Hitman {
    class ZHM3ItemWeaponCustom;
    class ZHM3WeaponUpgrade;

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
        // PC 0x653AF0. Walks the weapon's upgrade REFTAB and applies every present upgrade geom.
        void ApplyUpgrades(EWeaponType weaponType, ZHM3ItemWeaponCustom* pCustomGun, Glacier::REFTAB* pUpgrades, bool bApply);
        // PC 0x653A50. Appends every ZHM3WeaponUpgrade child of the upgrade weapon to pOut.
        void GetUpgradeList(EWeaponType weaponType, Glacier::REFTAB* pOut);
        // PC 0x653920 / 0x653990. Adds / removes the bones listed by the upgrade on the weapon.
        void AddVisibleBonesFromUpgrade(ZHM3ItemWeaponCustom* pCustomGun, ZHM3WeaponUpgrade* pUpgrade);
        void HideVisibleBonesFromUpgrade(ZHM3ItemWeaponCustom* pCustomGun, ZHM3WeaponUpgrade* pUpgrade);
        // PC 0x653660. Fixes the SMG's silencer / laser bones depending on which variants are present.
        void FixCustomSMGBones(EWeaponType weaponType, ZHM3ItemWeaponCustom* pCustomGun, EUpgradeType eUpgradeType);

        // static methods
        static EWeaponType GetWeaponType(const char* psWeaponName);

        // data (total size is 0x60, base size is 0x4C)
        // The five upgrade weapons are stored contiguously and indexed by EWeaponType
        // (PC 0x653A00 indexes (&m_pCustomRifleSniperUpgradeWeapon)[weaponType]).
        Glacier::ZGEOM* m_pCustomRifleSniperUpgradeWeapon; // +0x4C, EWeaponType::EW_UNKNOWN
        Glacier::ZGEOM* m_pCustomGunHardballerUpgradeWeapon;
        Glacier::ZGEOM* m_pCustomRifleAssaultUpgradeWeapon;
        Glacier::ZGEOM* m_pCusomSmgUpgradeWeapon;
        Glacier::ZGEOM* m_pCustomRifleShotgunUpgradeWeapon;
    };
}