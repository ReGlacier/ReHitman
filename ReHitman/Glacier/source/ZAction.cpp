#include <Glacier/ZAction.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    ZAction** ZAction::GetActionArray()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((ZAction**(__thiscall*)(ZAction*))(G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray))(this);
        }
        return nullptr;
    }
}