#include <BloodMoney/Game/UI/ZHM3MenuElements.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    ZGUIBase* ZHM3MenuElements::GetGUIElement(const char *psElementName) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3MenuElements_GetGUIElement != BMConfigurationService::kNotConfiguredOption);
        assert(psElementName != nullptr);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3MenuElements_GetGUIElement != BMConfigurationService::kNotConfiguredOption && psElementName) {
            return ((ZGUIBase*(__thiscall*)(ZHM3MenuElements*,const char*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3MenuElements_GetGUIElement)(this, psElementName);
        }

        return nullptr;
    }
}