#pragma once

#include <cstdint>

namespace Hitman::BloodMoney
{
    struct BMConfigurationService
    {
        // Consts
        static constexpr std::intptr_t kNotConfiguredOption = 0x0;

        // Config
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_GetClosestWaypoint;
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_GetRndUsePoint;
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_SetExternalWaypointList;
        static std::intptr_t BMAPI_FunctionAddress_ZPathFollower_SetWaypointIndex;
        static std::intptr_t BMAPI_FunctionAddress_ZHM3ItemBomb_Explode;
        static std::intptr_t BMAPI_FunctionAddress_CMetalDetector_DoDetectWeapon;
        static std::intptr_t BMAPI_FunctionAddress_CMetalDetector_DoAlarm;
    };
}