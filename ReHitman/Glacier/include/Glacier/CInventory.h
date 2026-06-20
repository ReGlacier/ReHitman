#pragma once

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>

namespace Glacier
{
    class ZItem;

    class CInventory : public CBaseEvent<ZGEOM>
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
        virtual int GetItemFromItemTemplate(ZItemTemplate* itemTemplate, bool);
        virtual REFTAB32* GetInventoryList();
        virtual void TransferInventoryTo(ZGEOM* target);

        // api
        ZItem* AddItem(Glacier::ZREF itemREF);
        void RemoveItem(Glacier::ZREF itemREF);

        // data
        uint32_t m_rCUI;
        REFTAB   m_StartInventory;
        REFTAB32 m_Inventory;
        REFTAB32 m_TakeActions;
    };

    RE_VERIFY_SIZE(CInventory, 0x1A8);
    RE_VERIFY_OFFSET(CInventory, m_rCUI, 0x30);
    RE_VERIFY_OFFSET(CInventory, m_StartInventory, 0x34);
}