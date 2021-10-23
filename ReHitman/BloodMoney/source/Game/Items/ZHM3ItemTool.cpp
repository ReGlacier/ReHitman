#include <BloodMoney/Game/Items/ZHM3ItemTool.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    std::string_view ZHM3ItemTool::GetHM3ItemName(EHM3ItemType eItemType) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemTool_GetHM3ItemName != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemTool_GetHM3ItemName != BMConfigurationService::kNotConfiguredOption) {
            const char* sValue = ((const char*(__cdecl*)(EHM3ItemType))BMConfigurationService::BMAPI_FunctionAddress_ZHM3ItemTool_GetHM3ItemName)(eItemType);
            if (!sValue) {
                return "";
            }

            return std::string_view { sValue };
        }
        return "";
    }
}