#include <BloodMoney/Game/ZHM3WeaponUpgradeControl.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    void ZHM3WeaponUpgradeControl::InitWeaponReferences() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_InitWeaponReferences != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_InitWeaponReferences != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3WeaponUpgradeControl*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_InitWeaponReferences)(this);
        }
    }

    void ZHM3WeaponUpgradeControl::ApplyDefaultUpgrades(EWeaponType weaponType, ZHM3ItemWeaponCustom *pCustomGun) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_ApplyDefaultUpgrades != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_ApplyDefaultUpgrades != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3WeaponUpgradeControl*,EWeaponType,ZHM3ItemWeaponCustom*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_ApplyDefaultUpgrades)(this, weaponType, pCustomGun);
        }
    }

    EWeaponType ZHM3WeaponUpgradeControl::GetWeaponType(const char *psWeaponName) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_GetWeaponType != BMConfigurationService::kNotConfiguredOption);
        assert(psWeaponName != nullptr);

        if (psWeaponName && BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_GetWeaponType != BMConfigurationService::kNotConfiguredOption) {
            return ((EWeaponType(__stdcall*)(const char*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3WeaponUpgradeControl_GetWeaponType)(psWeaponName);
        }

        return EWeaponType::EW_UNKNOWN;
    }
}