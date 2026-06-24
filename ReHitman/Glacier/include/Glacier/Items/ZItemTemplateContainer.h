#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Glacier
{
    class ZItemTemplateContainer : public ZItemTemplate
    {
    public:
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