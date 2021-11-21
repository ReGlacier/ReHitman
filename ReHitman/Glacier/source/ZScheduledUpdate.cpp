#include <G1ConfigurationService.h>
#include <Glacier/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZScheduledEvent.h>
#include <cassert>

namespace Glacier {
    ZScheduledEvent* ZScheduledUpdate::AddEvent(ZEventBase *pHolder) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_AddEvent != G1ConfigurationService::kNotConfiguredOption);
        assert(pHolder != nullptr);

        if (!pHolder) return nullptr;
        if (pHolder->m_pScheduledScript) return nullptr;

        if (G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_AddEvent != G1ConfigurationService::kNotConfiguredOption) {
            return ((ZScheduledEvent*(__thiscall*)(ZScheduledUpdate*, ZEventBase*))G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_AddEvent)(this, pHolder);
        }

        return nullptr;
    }

    void ZScheduledUpdate::RemoveEvent(ZEventBase* pHolder) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_AddEvent != G1ConfigurationService::kNotConfiguredOption);
        assert(pHolder != nullptr);

        if (!pHolder) return;
        if (!pHolder->m_pScheduledScript) return;

        if (G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_AddEvent != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZScheduledUpdate*, ZEventBase*))G1ConfigurationService::G1API_FunctionAddress_ZScheduledUpdate_RemoveEvent)(this, pHolder);
        }
    }
}