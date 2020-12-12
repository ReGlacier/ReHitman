#pragma once

#include <BloodMoney/Game/OnLevel/CMetalDetector.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    void CMetalDetector::DoDetectWeapon()
    {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoDetectWeapon != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoDetectWeapon != BMConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CMetalDetector*))(BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoDetectWeapon))(this);
        }
    }

    void CMetalDetector::DoAlarm()
    {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoAlarm != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoAlarm != BMConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(CMetalDetector*))(BMConfigurationService::BMAPI_FunctionAddress_CMetalDetector_DoAlarm))(this);
        }
    }
}