#pragma once

#include <Glacier/ZGEOM.h>
#include <Glacier/ZGROUP.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>

namespace Glacier
{
    class CInventory : public ZEventBase
    {
    public:
        // const
        static constexpr const char* Name = "Inventory";

        // vftable
        virtual void GiveItem(const char*);
        virtual void GiveItemInGroup(ZGROUP* group, const char* item);
        virtual void GiveAllItemsInGroup(ZGROUP* group);
        virtual void GiveAllItems();
        virtual void CreateTakeActions();
        virtual void GetItemFromItemTemplate(ZItemTemplate* itemTemplate, bool);
        virtual REFTAB32* GetInventoryList();
        virtual void TransferInventoryTo(ZGEOM* target);

        // data
        int field_30;
        REFTAB m_reftab;
        REFTAB32 m_inventoryList;
        REFTAB32 m_reftab32_2;
    };

    static_assert(offsetof(CInventory, field_30) == 0x30, "CInventory| Bad offset of field_30");
}