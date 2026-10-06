#pragma once

#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Glacier
{
    class ZItemTemplateContainer : public ZItemTemplate
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZItemTemplateContainer, 0x1007D9u);

        // methods
        ZItemTemplateContainer(const char* psName, ZBaseGeom* pBaseGeom);

        // vtbl (RTTI / ZGEOM / ZItemTemplate overrides)
        ~ZItemTemplateContainer() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void CopyData(const ZGEOM* Source) override;
        uint32_t GetItemClassId() const override;

        //vftable
        virtual bool CanContainItem(const ZItem* item);

        //data (total size is 0x98 , ZItemTemplate size is 0x74)
        REFTAB m_containedItems;
        int m_iMaxNumOfItems;
        bool m_bHideItem;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZItemTemplateContainer, 0x98); // Verified
}
