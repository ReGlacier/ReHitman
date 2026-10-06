#include <BloodMoney/Game/Items/ZHM3Item.h>

#include <BloodMoney/Game/Items/ZHM3ItemTemplate.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/RTP/VirtualTables.h>

namespace Hitman
{
    ZHM3Item::ZHM3Item(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItem(psName, pBaseGeom)
        , m_pGround(nullptr)
        , m_pAnimUse(nullptr)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x649C40
    const Glacier::RTP::ZPropertyInfo& ZHM3Item::GetProperties() const
    {
        return ZHM3Item::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64B290
    uint32_t ZHM3Item::GetObjectId() const
    {
        return ZHM3Item::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64B2A0
    void ZHM3Item::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3Item::m_Id;
        mask = ZHM3Item::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x649C50 -> &ZHM3Item::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3Item::GetOldClassInfo() const
    {
        return ZHM3Item::m_OldClassInfo;
    }

    // PC 0x64B8D0
    EHM3ItemType ZHM3Item::GetHM3ItemType()
    {
        if (m_eOverriddenType != EHM3ItemType::eHM3NoType)
            return m_eOverriddenType;

        // The override wins; otherwise the type is asked from the HM3 item template.
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        if (pTemplate != nullptr &&
            (ZHM3ItemTemplate::m_Mask & pTemplate->GetObjectId()) == ZHM3ItemTemplate::m_Id)
        {
            return static_cast<ZHM3ItemTemplate*>(pTemplate)->GetHM3ItemType();
        }

        return EHM3ItemType::eHM3NoType;
    }

    // PC 0x6D1400
    void ZHM3Item::OverrideItemType(EHM3ItemType itemType)
    {
        m_eOverriddenType = itemType;
    }

    // PC 0x649DE0
    void ZHM3Item::UseItemActivateAnimation()
    {
        if (m_pGround != nullptr && m_pAnimUse != nullptr)
            m_pGround->ActivateAnim(m_pAnimUse, 1);
    }

    // PC 0x649DA0
    void ZHM3Item::RestoreBites(uint32_t numBites)
    {
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        const uint32_t lNumBites = pTemplate != nullptr
            ? static_cast<ZHM3ItemTemplate*>(pTemplate)->m_iNumBites
            : 0u;

        if (numBites != 0 && numBites <= lNumBites)
            m_iNumBitesRemoved = numBites;
        else
            m_iNumBitesRemoved = 0;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // The PC chain is laid out tail-first; the head (m_iNumBitesRemoved) is the FirstProperty
        // passed to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3Item::Info.First is 0x80F68C).
        static Glacier::RTP::ZDataProperty<bool> ForceUnpickable{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bForceUnpickable", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3Item, m_bForceUnpickable)};

        static Glacier::RTP::ZDataProperty<uint32_t> NumBitesRemoved{
            .m_Node = {.m_Next = ForceUnpickable, .m_Name = "m_iNumBitesRemoved", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZHM3Item, m_iNumBitesRemoved)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3Item,              // ClassName
        Glacier::ZItem,        // BaseClass
        0x009B1658,            // OldClassInfoAddr
        "ZHM3Item",            // FactoryName
        0x0,                   // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::NumBitesRemoved, // FirstProperty
        0x0080F6A0,            // PropertiesAddr (ZHM3Item::Info)
        0x009B1500,            // IdAddr (ZHM3Item::m_Id)
        0x009B1504             // MaskAddr (ZHM3Item::m_Mask)
    );
#   pragma endregion
}
