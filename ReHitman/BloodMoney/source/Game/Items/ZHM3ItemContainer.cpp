#include <BloodMoney/Game/Items/ZHM3ItemContainer.h>

#include <BloodMoney/Game/Items/ZHM3ItemTemplateContainer.h>
#include <BloodMoney/Game/Items/ZHM3ItemTool.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHitman3.h>
#include <BloodMoney/Game/ZItemUpgradeSelector.h> // sSuitcaseItem
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>

namespace Hitman
{
    // PC 0x64B1B0. Chains to the ZItemContainer base ctor only.
    ZHM3ItemContainer::ZHM3ItemContainer(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItemContainer(psName, pBaseGeom)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x649E10
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemContainer::GetProperties() const
    {
        return ZHM3ItemContainer::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64B2C0
    uint32_t ZHM3ItemContainer::GetObjectId() const
    {
        return ZHM3ItemContainer::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64B2D0
    void ZHM3ItemContainer::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemContainer::m_Id;
        mask = ZHM3ItemContainer::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x649E20 -> &ZHM3ItemContainer::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemContainer::GetOldClassInfo() const
    {
        return ZHM3ItemContainer::m_OldClassInfo;
    }

    // PC 0x64B970
    EHM3ItemType ZHM3ItemContainer::GetHM3ItemType()
    {
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        if (pTemplate != nullptr &&
            (ZHM3ItemTemplateContainer::m_Mask & pTemplate->GetObjectId()) == ZHM3ItemTemplateContainer::m_Id)
        {
            return static_cast<ZHM3ItemTemplateContainer*>(pTemplate)->GetHM3ItemType();
        }

        return EHM3ItemType::eHM3NoType;
    }

    // PC 0x649EB0
    bool ZHM3ItemContainer::IsDetectable()
    {
        // Only the sniper-rifle suitcase may be flagged as non-detectable.
        if (GetHM3ItemType() != EHM3ItemType::eHM3ContainerSuitcaseSniper01)
            return true;

        ZHM3GameData* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        ZASSERT(pGameData != nullptr);

        sSuitcaseItem item{};
        // NOTE: the PC body tests `!GetAvailableItem(...)`, i.e. the returned index == 0, because
        // GetAvailableItem returns a 0-based index or -1 rather than a bool.
        return !pGameData->m_LevelLinking.GetAvailableItem(
                   static_cast<int>(EHM3ItemType::eHM3Suitcase), item)
            || (item.m_lUpgradeMask & 0x200) == 0;
    }

    // PC 0x64D700
    bool ZHM3ItemContainer::CanContainItem(Glacier::ZItem* item)
    {
        if (item == nullptr || (Glacier::ZItem::m_Mask & item->GetObjectId()) != Glacier::ZItem::m_Id)
            return false;

        const EHM3ItemType eType = ZHM3ItemTool::GetHM3Type(item);

        ZHM3GameData* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        ZASSERT(pGameData != nullptr);
        ZHitman3* pHitman = pGameData->m_Hitman3;

        return eType != EHM3ItemType::eHM3ItemCoin
            && eType != EHM3ItemType::eHM3PainKillers
            && eType != EHM3ItemType::eHM3Adrenaline
            && pHitman->CanDropItem(item)
            && (!pHitman->IsItemSuitcase(this, false)
                || pHitman->IsItemSuitcase(this, true)
                // Custom weapons, rifles, snipers and SMGs are never stowed in another case.
                || (eType != EHM3ItemType::eHM3Rifle_Airrifle_Tranquilizer_01
                    && eType != EHM3ItemType::eHM3Rifle_Enfield_01
                    && eType != EHM3ItemType::eHM3Rifle_FN2000_01
                    && eType != EHM3ItemType::eHM3Rifle_M14_01
                    && eType != EHM3ItemType::eHM3Rifle_Remington_01
                    && eType != EHM3ItemType::eHM3Rifle_SG552_01
                    && eType != EHM3ItemType::eHM3Sniper
                    && eType != EHM3ItemType::eHM3SniperRifle_Browning_01
                    && eType != EHM3ItemType::eHM3SniperRifle_Dragunov_01
                    && eType != EHM3ItemType::eHM3SniperRifle_SakoTRG_01
                    && eType != EHM3ItemType::eHM3SMG_FamaeSAF_01
                    && eType != EHM3ItemType::eHM3SMG_MP5_01
                    && eType != EHM3ItemType::eHM3SMG_MP7_01
                    && eType != EHM3ItemType::eHM3SMG_RugerMP9_01
                    && eType != EHM3ItemType::eHM3SMG_SteyrTMP_01
                    && eType != EHM3ItemType::eHM3CustomSniper
                    && eType != EHM3ItemType::eHM3CustomSMG
                    && eType != EHM3ItemType::eHM3CustomMG
                    && eType != EHM3ItemType::eHM3CustomSG))
            && Glacier::ZItemContainer::CanContainItem(item);
    }

    // PC 0x649E60
    void ZHM3ItemContainer::InitItemContainerAction()
    {
        ZHM3GameData* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        ZASSERT(pGameData != nullptr);

        if (!pGameData->m_Hitman3->IsItemSuitcase(this, true))
            Glacier::ZItemContainer::InitItemContainerAction();
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // PC 0x80F76C (game offset 0xC8 -> class offset 0xCC). The PS2 record at 0x94C298 names
        // this entry "m_bForceUnpickable"; it is the only property of the chain
        // (ZHM3ItemContainer::Info.First == 0x80F76C).
        static Glacier::RTP::ZDataProperty<bool> ForceUnpickable{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bForceUnpickable", .m_Filter = 3},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemContainer, m_bForceUnpickable)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemContainer,        // ClassName
        Glacier::ZItemContainer,  // BaseClass
        0x009B18D8,               // OldClassInfoAddr
        "ZHM3ItemContainer",      // FactoryName
        0x0,                      // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::ForceUnpickable, // FirstProperty
        0x0080F780,               // PropertiesAddr (ZHM3ItemContainer::Info)
        0x009B1540,               // IdAddr (ZHM3ItemContainer::m_Id)
        0x009B1544                // MaskAddr (ZHM3ItemContainer::m_Mask)
    );
#   pragma endregion
}
