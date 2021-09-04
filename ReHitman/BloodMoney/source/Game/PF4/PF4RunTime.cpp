#include <BloodMoney/Game/PF4/PF4RunTime.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney::PF4RunTime {
    ZData* ZData::CreatePathFinder(void *pData) {
        if (!pData) return nullptr;
        assert(BMConfigurationService::BMAPI_FunctionAddress_PF4_CreatePathFinder != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_PF4_CreatePathFinder != BMConfigurationService::kNotConfiguredOption) {
            return ((ZData*(__cdecl*)(void*))BMConfigurationService::BMAPI_FunctionAddress_PF4_CreatePathFinder)(pData);
        }

        return nullptr;
    }
}