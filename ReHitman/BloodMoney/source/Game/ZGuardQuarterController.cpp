#include <BloodMoney/Game/ZGuardQuarterController.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    void ZGuardQuarterController::RegisterActor(Glacier::ZREF rActorREF) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZGuardQuarterController_RegisterActor != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZGuardQuarterController_RegisterActor != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZGuardQuarterController*,Glacier::ZREF))BMConfigurationService::BMAPI_FunctionAddress_ZGuardQuarterController_RegisterActor)(this, rActorREF);
        }
    }

    ZGuardQuarterController* ZGuardQuarterController::g_pCurrentLevelGuardControl = nullptr;
}