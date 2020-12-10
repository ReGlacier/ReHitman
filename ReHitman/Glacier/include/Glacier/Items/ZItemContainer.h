#pragma once

#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Items/ZItem.h>

namespace Glacier
{
    class ZItemContainer : public ZItem
    {
    public:
        // vftable
        virtual void InitItemContainerAction();
        virtual void InsertItem(Glacier::ZREF itemRef, bool a2);
        virtual void RemoveItem(Glacier::ZREF itemRef);
        virtual REFTAB* GetContainedItems();
        virtual bool CanContainItem(ZItem* item);

        // api
        void FreePos(ZItem* item);
        ZItem* OccupyPos(ZItem* item);
        bool IsContainerFull();

        //data (total size is 0xC8, ZItem size is 0x84)
        ZAction* m_actionsHolder;
        ZMSGID m_MSG_REMOVEITEMFROMINVENTORY;
        ZMSGID field_8A; // not message, just alignment
        REFTAB* m_containedItems;
        REFTAB m_items;
        REFTAB m_reftabAC;
    };
}