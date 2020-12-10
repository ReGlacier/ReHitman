#include <BloodMoney/Game/Items/ZHM3ItemBomb.h>
#include <BloodMoney/BMConfigurationService.h>

#include <cassert>

namespace Hitman::BloodMoney
{
    void ZHM3ItemBomb::Explode()
    {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemBomb_Explode != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemBomb_Explode != BMConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZHM3ItemBomb*))(BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemBomb_Explode))(this);
        }
    }

}
