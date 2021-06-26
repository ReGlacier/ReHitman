#include <Glacier/CInventory.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    void CInventory::AddItem(Glacier::ZREF itemREF)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(Glacier::CInventory*, Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem))(this, itemREF);
        }
    }

    void CInventory::RemoveItem(Glacier::ZREF itemREF)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(Glacier::CInventory*, Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem))(this, itemREF);
        }
    }
}