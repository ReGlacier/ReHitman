#include <BloodMoney/Game/UI/ZWINOBJ.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    int ZWINOBJ::GetTexture(unsigned int iTextureType) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZWINOBJ_GetTexture != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZWINOBJ_GetTexture != BMConfigurationService::kNotConfiguredOption) {
            return ((int(__thiscall*)(ZWINOBJ*,unsigned int))BMConfigurationService::BMAPI_FunctionAddress_ZWINOBJ_GetTexture)(this, iTextureType);
        }
        return 0;
    }
}