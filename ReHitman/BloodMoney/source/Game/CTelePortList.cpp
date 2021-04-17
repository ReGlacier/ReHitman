#include <BloodMoney/Game/CTelePortList.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    void CTelePortList::TeleportToPointAtIndex(int index) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CTelePortList_TeleportToPointAtIndex != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CTelePortList_TeleportToPointAtIndex != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(Hitman::BloodMoney::CTelePortList*, int))
            (BMConfigurationService::BMAPI_FunctionAddress_CTelePortList_TeleportToPointAtIndex))(this, index);
        }
    }
}