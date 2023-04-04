#include <BloodMoney/Game/UI/ZLINEOBJ.h>
#include <BloodMoney/BMConfigurationService.h>

namespace Hitman::BloodMoney {
    ZLINEOBJ* ZLINEOBJ::Create() {
        using ZLINEOBJ_FactoryBuilder_t = ZLINEOBJ*(__cdecl*)();

        if (BMConfigurationService::BMAPI_FunctionAddress_ZLINEOBJ_FactoryConstructor != BMConfigurationService::kNotConfiguredOption) {
            return ((ZLINEOBJ_FactoryBuilder_t)BMConfigurationService::BMAPI_FunctionAddress_ZLINEOBJ_FactoryConstructor)();
        }

        return nullptr;
    }
}