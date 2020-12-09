#include <Glacier/ZItemTemplate.h>
#include <Glacier/REFTAB.h>

namespace Glacier
{
    class ZItemTemplateContainer : public ZItemTemplate
    {
    public:
        //vftable
        virtual bool CanContainItem(const ZItem* item);

        //data (total size is 0x90 , ZItemTemplate size is 0x74)
        REFTAB m_containedItems;
    };
}