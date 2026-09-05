#include <Glacier/GameBase/CInventory.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    ZItem* CInventory::AddItem(ZREF itemREF)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem != G1ConfigurationService::kNotConfiguredOption) {
            return ((Glacier::ZItem*(__thiscall*)(Glacier::CInventory*, Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_CInventory_AddItem))(this, itemREF);
        }

        return 0;
    }

    void CInventory::RemoveItem(ZREF itemREF)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(Glacier::CInventory*, Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_CInventory_RemoveItem))(this, itemREF);
        }
    }
}
