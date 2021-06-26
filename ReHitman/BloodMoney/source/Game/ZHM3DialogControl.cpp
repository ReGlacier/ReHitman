#include <BloodMoney/Game/ZHM3DialogControl.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney {
    void ZHM3DialogControl::StartDialog(Glacier::ZLNKWHANDS* pFirstPerson,
                                        Glacier::ZLNKWHANDS* pSecondPerson,
                                        const char* pDialogBaseName,
                                        const char* pFirstPersonRoleName,
                                        const char* pSecondPersonRoleName,
                                        int startRole,
                                        bool a7,
                                        bool a8,
                                        bool a9,
                                        bool a10) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_StartDialog != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_StartDialog != BMConfigurationService::kNotConfiguredOption) {
            using Func_t = void(__thiscall*)(ZHM3DialogControl*, Glacier::ZLNKWHANDS*, Glacier::ZLNKWHANDS*, const char*, const char*, const char*, int, bool, bool, bool, bool);
            ((Func_t)BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_StartDialog)(
                    this,
                    pFirstPerson, pSecondPerson,
                    pDialogBaseName, pFirstPersonRoleName, pSecondPersonRoleName, startRole,
                    a7, a8, a9, a10);
        }
    }

    void ZHM3DialogControl::SetDialogSkipable(bool value) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_SetDialogSkipable != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_SetDialogSkipable != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZHM3DialogControl*, bool))BMConfigurationService::BMAPI_FunctionAddress_ZHM3DialogControl_SetDialogSkipable)(this, value);
        }
    }

    bool ZHM3DialogControl::IsPlaying() const {
        return this->m_flags & 1;
    }
}