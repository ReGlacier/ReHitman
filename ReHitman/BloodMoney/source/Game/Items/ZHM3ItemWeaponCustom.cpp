#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    void ZHM3ItemWeaponCustom::ApplyUpgrades(char a1) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ApplyUpgrades != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ApplyUpgrades != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3ItemWeaponCustom*,char))BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ApplyUpgrades)(this, a1);
        }
    }

    void ZHM3ItemWeaponCustom::UpdateWeaponPartDrawStatus() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3ItemWeaponCustom*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_UpdateWeaponPartDrawStatus)(this);
        }
    }

    void ZHM3ItemWeaponCustom::ClearUpgrades() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ClearUpgrades != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ClearUpgrades != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3ItemWeaponCustom*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemWeaponCustom_ClearUpgrades)(this);
        }
    }
}