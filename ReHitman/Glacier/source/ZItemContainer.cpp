#include <Glacier/Items/ZItemContainer.h>

#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ITEMSTATE.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Items/ZItemTemplateContainer.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZAction.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>


namespace Glacier
{
    // vtbl slot 173  PC 0x510160 / PS2 0x27B058
    ZItemContainer::ZItemContainer(const char* psName, ZBaseGeom* pBaseGeom)
        : ZItem(psName, pBaseGeom)
        , m_rDelayedInsertItem(8, 0)
        , m_rPlacePoses(1, 1)
    {
        m_prtContainedItems = ZUniMemory::New<REFTAB>(8, 0);
        m_msgRemoveItemFromInventory = g_pEngineData->RegisterZMsg(
            "MSG_REMOVEITEMFROMINVENTORY", 0, __FILE__, __LINE__);
        m_pActionPlaceItem = nullptr;
    }

    // vtbl slot 174  PC 0x510270
    ZItemContainer::~ZItemContainer()
    {
        if (m_prtContainedItems != nullptr)
        {
            ZUniMemory::Delete(m_prtContainedItems);
            m_prtContainedItems = nullptr;
        }
    }

    // vtbl slot 12  PC 0x510250
    const RTP::ZPropertyInfo& ZItemContainer::GetProperties() const
    {
        return ZItemContainer::Info;
    }

    // vtbl slot 13  PC 0x510B80
    uint32_t ZItemContainer::GetObjectId() const
    {
        return ZItemContainer::m_Id;
    }

    // vtbl slot 14  PC 0x510B90
    void ZItemContainer::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItemContainer::m_Id;
        mask = ZItemContainer::m_Mask;
    }

    // vtbl slot 15  PC 0x5101F0
    ZGEOMCLASSINFO* ZItemContainer::GetOldClassInfo() const
    {
        return ZItemContainer::m_OldClassInfo;
    }

    // vtbl slot 104  PS2 0x27B4F4
    void ZItemContainer::CopyData(const ZGEOM* Source)
    {
        ZItem::CopyData(Source);

        if (!Source->IsDerivedFrom<ZItemContainer>())
            return;

        const ZItemContainer* pSource = static_cast<const ZItemContainer*>(Source);

        m_msgRemoveItemFromInventory = pSource->m_msgRemoveItemFromInventory;

        if (m_pActionPlaceItem != nullptr && pSource->m_pActionPlaceItem != nullptr)
            m_pActionPlaceItem = pSource->m_pActionPlaceItem;

        ZASSERT(pSource->m_prtContainedItems != nullptr);
        if (m_prtContainedItems == nullptr || pSource->m_prtContainedItems == nullptr)
            return;

        RefRun run;
        pSource->m_prtContainedItems->RunInitNxtRef(&run);
        for (const uint32_t* pRef = pSource->m_prtContainedItems->RunNxtRefPtr(&run); pRef != nullptr;
             pRef = pSource->m_prtContainedItems->RunNxtRefPtr(&run))
        {
            m_prtContainedItems->Add(*pRef);
        }
    }

    // vtbl slot 147  PS2 0x27B8A4
    void ZItemContainer::CreateFromTemplate()
    {
        ZItem::CreateFromTemplate();

        // Move the template's owned geoms into a fresh contained-items table: ZItem/ZItemTemplate
        // children are instantiated through the container, everything else is a warning.
        REFTAB* pOldContainedItems = m_prtContainedItems;
        m_prtContainedItems = ZUniMemory::New<REFTAB>(32, 0);

        if (pOldContainedItems == nullptr)
            return;

        RefRun run;
        pOldContainedItems->RunInitNxtRef(&run);
        for (uint32_t* pRef = pOldContainedItems->RunNxtRefPtr(&run); pRef != nullptr;
             pRef = pOldContainedItems->RunNxtRefPtr(&run))
        {
            ZGEOM* pGeom = ZGEOM::RefToPtr(*pRef);
            if (pGeom == nullptr)
                continue;

            if (pGeom->IsDerivedFrom<ZItemTemplate>())
            {
                ZItemTemplate* pItemTemplate = static_cast<ZItemTemplate*>(pGeom);
                pGeom = pItemTemplate->CreateItem(
                    g_pEngineData->m_pRoot, ZItemTemplate::kItemGroupId, false, false);

                if (pGeom == nullptr)
                    continue;
            }
            else if (!pGeom->IsDerivedFrom<ZItem>())
            {
                const char* pszName = pGeom->Name();
                const char* pszOwnerName = Name();

                printf("WARNING: Object %s in ZItemContainer %s is not an ZItem or ZItemTemplate\n",
                    pszName != nullptr ? pszName : "<NONAME>",
                    pszOwnerName != nullptr ? pszOwnerName : "<NONAME>");
                continue;
            }

            InsertItem(pGeom->GetRef(), false);
        }

        ZUniMemory::Delete(pOldContainedItems);
    }

    // vtbl slot 177  PC 0x510450 / PS2 0x27C298
    void ZItemContainer::InitItemContainerAction()
    {
        if (m_pActionPlaceItem != nullptr)
            return;

        const char* psPlaceText =
            g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Actions", "PlaceItem");
        m_pActionPlaceItem = ZAction::AddAction(
            this, psPlaceText, psPlaceText, AT_PLACEITEM, 0, 0, 150, 0);

        const char* psRetrieveText =
            g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Actions", "RetrieveItem");
        ZAction::AddAction(this, psRetrieveText, psRetrieveText, AT_RETRIEVEITEM, 0, 0, 150, 0);
    }

    // PC 0x5104E0 / PS2 0x27C450
    void ZItemContainer::EnablePlaceRetrieve(bool enable)
    {
        // The guard is the container's template ref (ZItem::m_rItemTemplate at +0x50).
        if (m_rItemTemplate == 0)
            return;

        if (ZAction* pPlace = GetAction(AT_PLACEITEM))
        {
            if (enable)
                pPlace->Show();
            else
                pPlace->Hide();
        }

        if (ZAction* pRetrieve = GetAction(AT_RETRIEVEITEM))
        {
            if (enable)
                pRetrieve->Show();
            else
                pRetrieve->Hide();
        }
    }

    // vtbl slot 175  PC 0x512DA0 / PS2 0x27C71C
    void ZItemContainer::InsertItem(Glacier::ZREF itemRef, bool a2)
    {
        ZGEOM* pGeom = ZGEOM::RefToPtr(itemRef);

        ZASSERT(pGeom != nullptr && pGeom->IsDerivedFrom<ZItem>());
        ZASSERT(m_prtContainedItems != nullptr && !m_prtContainedItems->Exists(itemRef));

        if (pGeom == nullptr || !pGeom->IsDerivedFrom<ZItem>())
            return;
        if (m_prtContainedItems == nullptr || m_prtContainedItems->Exists(itemRef))
            return;
        if (itemRef == GetRef())
            return;

        ZItem* pItem = static_cast<ZItem*>(pGeom);

        // a2 forces the insert past the container's valid-item check.
        if (!a2 && !CanContainItem(pItem))
            return;

        if (IsContainerFull())
        {
            // Remember the item so it can be inserted once a slot is freed.
            m_rDelayedInsertItem.Add(itemRef);
            return;
        }

        pItem->Hide(true);

        // Container hand-off. If the container is currently held in an actor's hand, detach it
        // from that hand and re-home it to the scene root; the item being inserted is likewise
        // detached from its own owner's hand. PC 0x512E5C..0x512F1E calls ZLNKWHANDS::
        // GetRHandItem/GetLHandItem and, on a ref match, AttachRHandItem(0)/AttachLHandItem(0).
        if (ZGEOM* pOwner = GetItemOwner(); pOwner != nullptr && geom_cast<ZLNKWHANDS>(pOwner) != nullptr)
        {
            auto* pHands = static_cast<ZLNKWHANDS*>(pOwner);

            if (ZItem* pRHandItem = pHands->GetRHandItem(); pRHandItem != nullptr && pRHandItem->GetRef() == GetRef())
            {
                pHands->AttachRHandItem(0);
                SetItemOwner(0, g_pEngineData->m_pRoot, true, true);
            }
            else if (ZItem* pLHandItem = pHands->GetLHandItem(); pLHandItem != nullptr && pLHandItem->GetRef() == GetRef())
            {
                pHands->AttachLHandItem(0);
                SetItemOwner(0, g_pEngineData->m_pRoot, true, true);
            }

            MakeDynamic(true);
            SetAutoRoomAssign(true);
        }

        if (ZGEOM* pItemOwner = pItem->GetItemOwner(); pItemOwner != nullptr && geom_cast<ZLNKWHANDS>(pItemOwner) != nullptr)
        {
            auto* pItemHands = static_cast<ZLNKWHANDS*>(pItemOwner);

            if (ZItem* pRHandItem = pItemHands->GetRHandItem(); pRHandItem != nullptr && pRHandItem->GetRef() == itemRef)
            {
                pItemHands->AttachRHandItem(0);
            }
            else if (ZItem* pLHandItem = pItemHands->GetLHandItem(); pLHandItem != nullptr && pLHandItem->GetRef() == itemRef)
            {
                pItemHands->AttachLHandItem(0);
            }
        }

        pItem->SetItemOwner(GetRef(), this, false, true);

        ZMat3x3 mat;
        ZVector3 pos;
        ZItem* pPrevious = OccupyPos(pItem);
        if (pPrevious != nullptr)
        {
            pPrevious->GetMatPos(mat, pos);
        }
        else
        {
            Glacier::mreset(mat.data);
            pos = ZVector3(0.f, 0.f, 0.f);
        }

        pItem->SetMatPos(mat, pos);

        if (pPrevious == nullptr)
        {
            ZMat3x3 rootMat;
            ZVector3 rootPos;
            pItem->GetRootTM(rootMat, rootPos);
            Glacier::mreset(rootMat.data);
            pItem->SetRootTM(rootMat, rootPos);
        }

        pItem->Freeze(false);

        m_prtContainedItems->Add(itemRef);
    }

    // vtbl slot 176  PC 0x5130C0 / PS2 0x27D1A4
    void ZItemContainer::RemoveItem(Glacier::ZREF itemRef)
    {
        ZASSERT(itemRef != 0);
        ZASSERT(m_prtContainedItems != nullptr && m_prtContainedItems->Exists(itemRef));

        if (itemRef == 0 || m_prtContainedItems == nullptr || !m_prtContainedItems->Exists(itemRef))
            return;

        ZGEOM* pGeom = ZGEOM::RefToPtr(itemRef);
        ZASSERT(pGeom != nullptr && pGeom->IsDerivedFrom<ZItem>());
        if (pGeom == nullptr || !pGeom->IsDerivedFrom<ZItem>())
            return;

        ZItem* pItem = static_cast<ZItem*>(pGeom);

        // Remember the item's world transform so it can be restored once it leaves the container.
        ZMat3x3 mat;
        ZVector3 pos;
        pItem->GetMainItemRootTM(mat.data, pos.Get());

        if ((m_baseGeom->m_lControl & 0x200000u) != 0)
            m_baseGeom->SetOwnerDraw(false);

        pItem->SetItemOwner(0, GetOwner(true), false, true);

        const ZItemTemplate* pTemplate = GetItemTemplate();
        if (pTemplate != nullptr && static_cast<const ZItemTemplateContainer*>(pTemplate)->m_bHideItem)
            pItem->SetState(IS_SHOW, nullptr);

        m_prtContainedItems->Remove(itemRef);

        pItem->SetRootTM(mat, pos);
        FreePos(pItem);
    }

    void ZItemContainer::FreePos(ZItem* item)
    {
        if (m_rPlacePoses.Count() == 0 || item == nullptr)
            return;

        const uint32_t itemRef = item->GetRef();
        RefRun run;
        m_rPlacePoses.RunInitNxtRef(&run);
        for (uint32_t* entry = m_rPlacePoses.RunNxtRefPtr(&run); entry != nullptr;
             entry = m_rPlacePoses.RunNxtRefPtr(&run))
        {
            if (entry[1] == itemRef)
                entry[1] = 0;
        }
    }

    ZItem* ZItemContainer::OccupyPos(ZItem* item)
    {
        if (item == nullptr || m_rPlacePoses.Count() == 0)
            return nullptr;

        RefRun run;
        m_rPlacePoses.RunInitNxtRef(&run);
        for (uint32_t* entry = m_rPlacePoses.RunNxtRefPtr(&run); entry != nullptr;
             entry = m_rPlacePoses.RunNxtRefPtr(&run))
        {
            if (entry[1] == 0)
            {
                const uint32_t previousRef = entry[0];
                entry[1] = item->GetRef();
                return static_cast<ZItem*>(ZGEOM::RefToPtr(previousRef));
            }
        }

        return nullptr;
    }

    bool ZItemContainer::IsContainerFull()
    {
        ZASSERT(m_prtContainedItems != nullptr);
        if (m_prtContainedItems == nullptr)
            return true;

        const ZItemTemplate* itemTemplate = GetItemTemplate();
        ZASSERT(itemTemplate != nullptr);
        if (itemTemplate == nullptr)
            return true;

        const auto* containerTemplate = static_cast<const ZItemTemplateContainer*>(itemTemplate);
        const int maxItems = containerTemplate->m_iMaxNumOfItems;
        if (maxItems == -1)
            return false;
        if (maxItems == 0)
            return true;

        return m_prtContainedItems->Count() >= maxItems;
    }

    bool ZItemContainer::CanContainItem(ZItem* item)
    {
        if (item == this)
            return false;

        const ZItemTemplate* itemTemplate = GetItemTemplate();
        if (itemTemplate == nullptr)
            return false;

        return const_cast<ZItemTemplateContainer*>(static_cast<const ZItemTemplateContainer*>(itemTemplate))
            ->CanContainItem(item);
    }

    REFTAB* ZItemContainer::GetContainedItems()
    {
        ZASSERT(m_prtContainedItems != nullptr);
        return m_prtContainedItems;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // Chain is declared tail-first; head (m_pActionPlaceItem) is passed to
        // DECLARE_GEOM_CLASS_IMPL.
        static RTP::ZDataProperty<REFTAB*> ContainedItems{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_prtContainedItems", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZItemContainer, m_prtContainedItems)};

        static RTP::ZDataProperty<uint> PlaceAction{
            .m_Node = {.m_Next = ContainedItems, .m_Name = "m_pActionPlaceItem", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = reinterpret_cast<uint*>(CLASS_PROPERTY(ZItemContainer, m_pActionPlaceItem))};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItemContainer, // ClassName
        ZItem, // BaseClass
        0x0099C208, // OldClassInfoAddr
        "ZItemContainer", // FactoryName
        0x00773F00, // FactoryNameAddr
        cProperties::PlaceAction, // FirstProperty
        0x0080C980, // PropertiesAddr
        0x0099BF68, // IdAddr
        0x0099BF6C // MaskAddr
    );
#   pragma endregion
}
