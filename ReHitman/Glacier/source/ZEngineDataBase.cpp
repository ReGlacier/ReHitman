#include <Glacier/ZEngineDataBase.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    std::intptr_t ZEngineDataBase::GetSceneCom()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((std::intptr_t(__thiscall*)(ZEngineDataBase*))(G1ConfigurationService::G1API_FunctionAddress_ZEngineDataBase_GetSceneCom))(this);
        }

        return 0;
    }
}