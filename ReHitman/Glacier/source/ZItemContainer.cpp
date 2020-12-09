#include <Glacier/ZItemContainer.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    void ZItemContainer::FreePos(ZItem* item)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_FreePos != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_FreePos != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZItemContainer*, ZItem*))(G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_FreePos))(this, item);
        }
    }

    ZItem* ZItemContainer::OccupyPos(ZItem* item)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_OccupyPos != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_OccupyPos != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((ZItem*(__thiscall*)(ZItemContainer*, ZItem*))(G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_OccupyPos))(this, item);
        }

        return nullptr;
    }

    bool ZItemContainer::IsContainerFull()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_IsContainerFull != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_IsContainerFull != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((bool(__thiscall*)(ZItemContainer*))(G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_IsContainerFull))(this);
        }

        return true;
    }
}