#include <BloodMoney/Game/ZHitman3.h>

#include <BloodMoney/Engine/ZHM3Camera.h>

#include <BloodMoney/Game/Items/ZHM3Item.h>
#include <BloodMoney/Game/Items/ZHM3ItemAmmo.h>
#include <BloodMoney/Game/Items/ZHM3ItemContainer.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplate.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateContainer.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>

namespace Hitman
{
    // PC 0x6186E0
    EHM3ItemType ZHM3Inventory::GetItemType(Glacier::ZGEOM* pItem)
    {
        if (pItem == nullptr)
            return EHM3ItemType::eHM3NoType;

        // Templates first, then the runtime items; the order mirrors the PC body.
        if ((ZHM3ItemTemplate::m_Mask & pItem->GetObjectId()) == ZHM3ItemTemplate::m_Id)
            return static_cast<ZHM3ItemTemplate*>(pItem)->GetHM3ItemType();

        if ((ZHM3ItemTemplateContainer::m_Mask & pItem->GetObjectId()) == ZHM3ItemTemplateContainer::m_Id)
            return static_cast<ZHM3ItemTemplateContainer*>(pItem)->GetHM3ItemType();

        if ((ZHM3ItemTemplateWeapon::m_Mask & pItem->GetObjectId()) == ZHM3ItemTemplateWeapon::m_Id)
            return static_cast<ZHM3ItemTemplateWeapon*>(pItem)->GetHM3ItemType();

        if ((ZHM3Item::m_Mask & pItem->GetObjectId()) == ZHM3Item::m_Id)
            return static_cast<ZHM3Item*>(pItem)->GetHM3ItemType();

        if ((ZHM3ItemWeapon::m_Mask & pItem->GetObjectId()) == ZHM3ItemWeapon::m_Id)
            return static_cast<ZHM3ItemWeapon*>(pItem)->GetHM3ItemType();

        if ((ZHM3ItemContainer::m_Mask & pItem->GetObjectId()) == ZHM3ItemContainer::m_Id)
            return static_cast<ZHM3ItemContainer*>(pItem)->GetHM3ItemType();

        if ((ZHM3ItemAmmo::m_Mask & pItem->GetObjectId()) == ZHM3ItemAmmo::m_Id)
            return static_cast<ZHM3ItemAmmo*>(pItem)->GetHM3ItemType();

        return EHM3ItemType::eHM3NoType;
    }

    // PC 0x5EC270
    bool ZHitman3::CanDropItem(Glacier::ZItem* pItem) const
    {
        // GetItemType is non-const on the PC build; the original const-strips `this` to reach it.
        const EHM3ItemType eType = const_cast<ZHitman3*>(this)->GetItemType(pItem);

        return eType != EHM3ItemType::eHM3CC_FiberWire_01
            && eType != EHM3ItemType::eHM3Binoculars
            && eType != EHM3ItemType::eHM3RemoteControl
            && eType != EHM3ItemType::eHM3NoType;
    }

    // PC 0x5ED450
    bool ZHitman3::IsItemSuitcase(Glacier::ZItem* pItem, bool bSuitcase) const
    {
        // GetItemType is non-const on the PC build; the original const-strips `this` to reach it.
        switch (const_cast<ZHitman3*>(this)->GetItemType(pItem))
        {
        // Only a suitcase while bSuitcase is false.
        case EHM3ItemType::eHM3Suitcase:
        case EHM3ItemType::eHM3M07SuitcaseNegotiator:
        case EHM3ItemType::eHM3ContainerBriefcase:
        case EHM3ItemType::eHM3ContainerBriefcaseAfrikaner:
        case EHM3ItemType::eHM3ContainerBriefcaseDiamonds:
        case EHM3ItemType::eHM3ContainerBriefcaseSheik:
        case EHM3ItemType::eHM3EquipSuitcaseFoilPadded:
        case EHM3ItemType::eHM3ContainerSuitcaseWoman:
            return !bSuitcase;

        // Always a suitcase.
        case EHM3ItemType::eHM3M11FlightcaseRifle:
        case EHM3ItemType::eHM3ContainerRifle:
        case EHM3ItemType::eHM3ContainerSuitcaseSniper01:
            return true;

        default:
            return false;
        }
    }

    // PC 0x5F1810
    bool ZHitman3::IsInScopeMode() const
    {
        if (m_pCameraControl == nullptr)
            return false;

        if (m_bFakeScopeMode)
            return true;

        // Camera modes 3 and 9 are the scope / binocular views.
        const int lCameraMode = m_pCameraControl->m_fieldD4;
        return lCameraMode == 3 || lCameraMode == 9;
    }
}
