#include <Glacier/Items/ZItemTemplateContainer.h>

#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemContainer.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZSTL/StringUtils.h>

#include <cstdio>


namespace Glacier
{
    // vtbl slot 82  PC 0x510060 / PS2 0x27A8AC
    ZItemTemplateContainer::ZItemTemplateContainer(const char* psName, ZBaseGeom* pBaseGeom)
        : ZItemTemplate(psName, pBaseGeom)
        , m_containedItems(8, 0)
    {
        m_iMaxNumOfItems = 1;
        m_bHideItem = false;
    }

    ZItemTemplateContainer::~ZItemTemplateContainer() = default;

    // vtbl slot 12  PC 0x5100D0
    const RTP::ZPropertyInfo& ZItemTemplateContainer::GetProperties() const
    {
        return ZItemTemplateContainer::Info;
    }

    // vtbl slot 13  PC 0x510A80
    uint32_t ZItemTemplateContainer::GetObjectId() const
    {
        return ZItemTemplateContainer::m_Id;
    }

    // vtbl slot 14  PC 0x510A70-family
    void ZItemTemplateContainer::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItemTemplateContainer::m_Id;
        mask = ZItemTemplateContainer::m_Mask;
    }

    // vtbl slot 15  PC 0x510100
    ZGEOMCLASSINFO* ZItemTemplateContainer::GetOldClassInfo() const
    {
        return ZItemTemplateContainer::m_OldClassInfo;
    }

    // vtbl slot 82  PC 0x510080 / PS2 0x27AC7C
    void ZItemTemplateContainer::ClassInit()
    {
        ZGEOM::ClassInit();

        // Count the group's owned geoms whose name contains "Pos_Place"; that is the number of
        // items the container can hold unless m_bHideItem suppresses item placement.
        int lMaxItems = 0;
        for (ZBaseGeom* pBaseGeom = m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            const char* pszName = pBaseGeom->Name();
            if (pszName != nullptr && Glacier::striwcmp(pszName, "*Pos_Place*"))
                ++lMaxItems;
        }

        if (!m_bHideItem && lMaxItems != 0)
            m_iMaxNumOfItems = lMaxItems;

        // An empty valid-item list means nothing can ever be placed into the container.
        if (m_containedItems.Count() == 0)
        {
            const char* pszName = Name();
            if (pszName == nullptr)
                pszName = "<NONAME>";

            printf("Container: %s Validitem's property is empty, You won't be able to place anything into it!!\n", pszName);
        }
    }

    // vtbl slot 104  PC 0x5101A0-family / PS2 0x27AB30
    void ZItemTemplateContainer::CopyData(const ZGEOM* Source)
    {
        ZGROUP::CopyData(Source);

        if (!Source->IsDerivedFrom<ZItemTemplateContainer>())
            return;

        const ZItemTemplateContainer* pSource = static_cast<const ZItemTemplateContainer*>(Source);

        m_iMaxNumOfItems = pSource->m_iMaxNumOfItems;
        m_bHideItem = pSource->m_bHideItem;

        RefRun run;
        pSource->m_containedItems.RunInitNxtRef(&run);
        for (const uint32_t* pRef = pSource->m_containedItems.RunNxtRefPtr(&run); pRef != nullptr;
             pRef = pSource->m_containedItems.RunNxtRefPtr(&run))
        {
            m_containedItems.Add(*pRef);
        }
    }

    // vtbl slot 149  PS2 0x7386AC
    uint32_t ZItemTemplateContainer::GetItemClassId() const
    {
        return ZItemContainer::m_TypeId;
    }

    bool ZItemTemplateContainer::CanContainItem(const ZItem* item)
    {
        ZASSERT(item != nullptr);
        if (item == nullptr)
            return false;

        if ((item->GetObjectId() & ZItemContainer::m_Mask) == ZItemContainer::m_Id)
            return false;

        if (m_containedItems.Count() == 0)
            return false;

        const ZItemTemplate* itemTemplate = const_cast<ZItem*>(item)->GetItemTemplate();
        if (itemTemplate == nullptr)
            return false;

        const uint32_t itemTemplateRef = itemTemplate->GetRef();
        RefRun run;
        m_containedItems.RunInitNxtRef(&run);
        for (uint32_t ref = m_containedItems.RunNxtRef(&run); run._RunPtr != nullptr;
             ref = m_containedItems.RunNxtRef(&run))
        {
            if (ref == itemTemplateRef)
                return true;
        }

        return false;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // Chain is declared tail-first; head (m_containedItems) is passed to
        // DECLARE_GEOM_CLASS_IMPL.
        static RTP::ZDataProperty<bool> HideItem{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bHideItem", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateContainer, m_bHideItem)};

        static RTP::ZDataProperty<int> MaxNumOfItems{
            .m_Node = {.m_Next = HideItem, .m_Name = "m_iMaxNumOfItems", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateContainer, m_iMaxNumOfItems)};

        static RTP::ZDataProperty<REFTAB> ValidItems{
            .m_Node = {.m_Next = MaxNumOfItems, .m_Name = "m_ValidItems", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateContainer, m_containedItems)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItemTemplateContainer, // ClassName
        ZItemTemplate, // BaseClass
        0x0099C0C8, // OldClassInfoAddr
        "ZItemTemplateContainer", // FactoryName
        0x00773EC8, // FactoryNameAddr
        cProperties::ValidItems, // FirstProperty
        0x0080C94C, // PropertiesAddr
        0x0099BF48, // IdAddr
        0x0099BF4C // MaskAddr
    );
#   pragma endregion
}
