#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/BMConfigurationService.h>

#include <cassert>

namespace Hitman::BloodMoney
{
    void ZHM3Actor::PreparePath()
    {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath != BMConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*))(BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath))(this);
        }
    }
}