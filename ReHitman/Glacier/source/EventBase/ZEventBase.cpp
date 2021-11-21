#include <Glacier/EventBase/ZEventBase.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier {
    void ZEventBase::ActivateFrameUpdate(bool a1) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ActivateFrameUpdate != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ActivateFrameUpdate != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZEventBase*, bool))G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ActivateFrameUpdate)(this, a1);
        }
    }

    void ZEventBase::DeactivateFrameUpdate() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEventBase_DeactivateFrameUpdate != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEventBase_DeactivateFrameUpdate != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZEventBase*))G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ActivateFrameUpdate)(this);
        }
    }

    void ZEventBase::ChangeEventActivity() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ChangeEventActivity != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ChangeEventActivity != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZEventBase*))G1ConfigurationService::G1API_FunctionAddress_ZEventBase_ChangeEventActivity)(this);
        }
    }

    void ZEventBase::ActivateTimeUpdate(float) {

    }

    int* ZEventBase::GetDefaultStatus() {
        assert(G1ConfigurationService::G1API_InstanceAddress_ZEventBase_m_DefaultStatus != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_InstanceAddress_ZEventBase_m_DefaultStatus != G1ConfigurationService::kNotConfiguredOption) {
            return reinterpret_cast<int*>(G1ConfigurationService::G1API_InstanceAddress_ZEventBase_m_DefaultStatus);
        }
        return nullptr;
    }
}