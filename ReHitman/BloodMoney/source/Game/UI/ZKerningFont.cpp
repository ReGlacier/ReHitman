#include <BloodMoney/Game/UI/ZKerningFont.h>
#include <BloodMoney/BMConfigurationService.h>

namespace Hitman::BloodMoney {
    ZKerningFont* ZKerningFont::Create() {
        if (BMConfigurationService::BMAPI_FunctionAddress_ZKerningFont_FactoryConstructor != BMConfigurationService::kNotConfiguredOption) {
            return ((ZKerningFont*(__cdecl*)())BMConfigurationService::BMAPI_FunctionAddress_ZKerningFont_FactoryConstructor)();
        }
        return nullptr;
    }
}