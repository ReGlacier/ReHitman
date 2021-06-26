#include <BloodMoney/Game/ZTie.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    void ZTie::HideTie(bool value) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTie != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTie != BMConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZTie*, bool))(BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTie))(this, value);
        }
    }

    void ZTie::HideTieInMirror(bool value) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTieInMirror != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTieInMirror != BMConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZTie*, bool))(BMConfigurationService::BMAPI_FunctionAddress_ZTie_HideTieInMirror))(this, value);
        }
    }
}