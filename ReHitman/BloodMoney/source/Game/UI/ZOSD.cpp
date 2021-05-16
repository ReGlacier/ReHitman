#include <BloodMoney/Game/UI/ZOSD.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    void ZOSD::AddInfo(const char* message, bool shouldPlayNotificationSound) {
        if (!message) return;

        assert(BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddInfo != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddInfo != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZOSD*, const char*, bool))BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddInfo)(this, message, shouldPlayNotificationSound);
        }
    }

    void ZOSD::AddWarning(const char* message, bool shouldPlayNotificationSound)
    {
        if (!message) return;

        assert(BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddWarning != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddWarning != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZOSD*, const char*, bool))BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddWarning)(this, message, shouldPlayNotificationSound);
        }
    }

    void ZOSD::AddHint(const char* message,
                       bool a2,
                       bool shouldPlayNotificationSound,
                       int osdElement,
                       bool a6,
                       const char* tutorialId)
    {
        if (!message) return;

        assert(BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddHint != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddHint != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZOSD*, const char*, bool, bool, int, bool, const char*))BMConfigurationService::BMAPI_FunctionAddress_ZOSD_AddHint)(this, message, a2, shouldPlayNotificationSound, osdElement, a6, tutorialId);
        }
    }
}