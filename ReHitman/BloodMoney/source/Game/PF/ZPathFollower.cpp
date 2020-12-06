#include <BloodMoney/Game/PF/ZPathFollower.h>
#include <BloodMoney/BMConfigurationService.h>

#include <cassert>

namespace Hitman::BloodMoney
{
    int ZPathFollower::GetClosestWaypoint() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetClosestWaypoint != BMConfigurationService::kNotConfiguredOption);
        return ((int(__thiscall*)(ZPathFollower*))(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetClosestWaypoint))(this);
    }

    int ZPathFollower::GetRndUsePoint() {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetRndUsePoint != BMConfigurationService::kNotConfiguredOption);
        return ((int(__thiscall*)(ZPathFollower*))(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_GetRndUsePoint))(this);
    }

    void ZPathFollower::SetExternalWaypointList(unsigned int REF) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetExternalWaypointList != BMConfigurationService::kNotConfiguredOption);
        ((void(__thiscall*)(ZPathFollower*, unsigned int))(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetExternalWaypointList))(this, REF);
    }

    void ZPathFollower::SetWaypointIndex(int index) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetWaypointIndex != BMConfigurationService::kNotConfiguredOption);
        ((void(__thiscall*)(ZPathFollower*, int))(BMConfigurationService::BMAPI_FunctionAddress_ZPathFollower_SetWaypointIndex))(this, index);
    }
}