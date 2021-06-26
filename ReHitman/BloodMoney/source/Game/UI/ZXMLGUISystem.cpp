#include <BloodMoney/Game/UI/ZXMLGUISystem.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    ZWINDOW* ZXMLGUISystem::GetTopWindow() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZXMLGUISystem_GetTopWindow != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZXMLGUISystem_GetTopWindow != BMConfigurationService::kNotConfiguredOption) {
            return ((ZWINDOW*(__thiscall*)(ZXMLGUISystem*))BMConfigurationService::BMAPI_FunctionAddress_ZXMLGUISystem_GetTopWindow)(this);
        }
        return nullptr;
    }
}