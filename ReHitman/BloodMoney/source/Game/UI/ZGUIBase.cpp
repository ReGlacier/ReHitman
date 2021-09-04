#include <BloodMoney/Game/UI/ZGUIBase.h>
#include <BloodMoney/Game/UI/ZWINGROUP.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    void ZGUIBase::GetSize(ZWINGROUP* pGroup, Glacier::Vector2* pSize) {
        assert(pGroup != nullptr);
        assert(pSize != nullptr);
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZGUIBase_GetSize != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZGUIBase_GetSize != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZGUIBase*,ZWINGROUP*,Glacier::Vector2*))BMConfigurationService::BMAPI_FunctionAddress_ZGUIBase_GetSize)(this, pGroup, pSize);
        }
    }
}