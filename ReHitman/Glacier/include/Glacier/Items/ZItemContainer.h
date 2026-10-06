#pragma once

#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Items/ZItem.h>

namespace Glacier
{
    class ZItemContainer : public ZItem
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZItemContainer, 0x1007DAu);

        // methods
        ZItemContainer(const char* psName, ZBaseGeom* pBaseGeom);

        // vtbl (RTTI / ZItem overrides)
        ~ZItemContainer() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const ZGEOM* Source) override;
        void CreateFromTemplate() override;

        // vftable
        virtual void InitItemContainerAction();
        virtual void InsertItem(Glacier::ZREF itemRef, bool a2);
        virtual void RemoveItem(Glacier::ZREF itemRef);
        virtual REFTAB* GetContainedItems();
        virtual bool CanContainItem(ZItem* item);

        // api
        void EnablePlaceRetrieve(bool enable);
        void FreePos(ZItem* item);
        ZItem* OccupyPos(ZItem* item);
        bool IsContainerFull();

        //data (total size is 0xC8, ZItem size is 0x84)
        ZAction* m_pActionPlaceItem;
        ZMSGID m_msgRemoveItemFromInventory;
        RE_ADD_PADDING(2);
        REFTAB* m_prtContainedItems;
        REFTAB m_rDelayedInsertItem;
        REFTAB m_rPlacePoses;
    };
    RE_VERIFY_SIZE(ZItemContainer, 0xC8); // Verified
}
