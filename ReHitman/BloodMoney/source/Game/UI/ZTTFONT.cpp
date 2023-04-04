#include <BloodMoney/Game/UI/ZTTFONT.h>
#include <BloodMoney/BMConfigurationService.h>


namespace Hitman::BloodMoney {
    ZTTFONT* ZTTFONT::Create() {
        if (BMConfigurationService::BMAPI_FunctionAddress_ZTTFONT_FactoryConstructor != BMConfigurationService::kNotConfiguredOption) {
            return ((ZTTFONT*(__cdecl*)())BMConfigurationService::BMAPI_FunctionAddress_ZTTFONT_FactoryConstructor)();
        }
        return nullptr;
    }
}