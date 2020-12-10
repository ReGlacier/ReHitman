#include <Glacier/EventBase/ZLnkActionQueue.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    void ZLnkActionQueue::DispatchNextAction()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_DispatchNextAction != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_DispatchNextAction != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZLnkActionQueue*))(G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_DispatchNextAction))(this);
        }
    }

    void ZLnkActionQueue::RemoveAction(ZLnkAction* action)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_RemoveAction != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_RemoveAction != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZLnkActionQueue*, ZLnkAction*))(G1ConfigurationService::G1API_FunctionAddress_ZLnkActionQueue_RemoveAction))(this, action);
        }
    }
}