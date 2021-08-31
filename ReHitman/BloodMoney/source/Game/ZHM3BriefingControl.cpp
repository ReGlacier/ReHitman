#include <BloodMoney/Game/ZHM3BriefingControl.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    void ZHM3BriefingControl::CompleteObjective(unsigned int iObjectiveId, bool bShowOSDNotification) {
        assert(iObjectiveId > 0);
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_CompleteObjective != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_CompleteObjective != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3BriefingControl*,unsigned int, bool))BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_CompleteObjective)(this, iObjectiveId, bShowOSDNotification);
        }
    }

    void ZHM3BriefingControl::PlaySpeech(bool bRegisterSpeechAsSpoken) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_PlaySpeech != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_PlaySpeech != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3BriefingControl*,bool))BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_PlaySpeech)(this, bRegisterSpeechAsSpoken);
        }
    }

    void ZHM3BriefingControl::StopSpeech() {
        if (!m_bIsSpeechPlaying) return;
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_StopSpeech != BMConfigurationService::kNotConfiguredOption);

        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_StopSpeech != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3BriefingControl*))BMConfigurationService::BMAPI_FunctionAddress_ZHM3BriefingControl_StopSpeech)(this);
        }
    }
}