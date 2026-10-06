#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmoCustom.h>

#include <Glacier/RTP/VirtualTables.h>

namespace Hitman
{
    // PC 0x64B140. The base ZHM3ItemTemplateAmmo ctor resolves m_eHM3ItemType from the object name
    // and sets the ZHM3ItemTemplateAmmo vtable; this ctor only swaps in the custom vtable.
    ZHM3ItemTemplateAmmoCustom::ZHM3ItemTemplateAmmoCustom(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : ZHM3ItemTemplateAmmo(psName, pBaseGeom)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64A9B0
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemTemplateAmmoCustom::GetProperties() const
    {
        return ZHM3ItemTemplateAmmoCustom::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64B3A0
    uint32_t ZHM3ItemTemplateAmmoCustom::GetObjectId() const
    {
        return ZHM3ItemTemplateAmmoCustom::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64B3B0
    void ZHM3ItemTemplateAmmoCustom::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemTemplateAmmoCustom::m_Id;
        mask = ZHM3ItemTemplateAmmoCustom::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64A9C0 -> &ZHM3ItemTemplateAmmoCustom::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemTemplateAmmoCustom::GetOldClassInfo() const
    {
        return ZHM3ItemTemplateAmmoCustom::m_OldClassInfo;
    }

    // PC 0x64A9D0. Overrides ZItemTemplateAmmo::GetCanPenetrate; the custom variant stores its
    // own flag instead of always returning false.
    bool ZHM3ItemTemplateAmmoCustom::GetCanPenetrate()
    {
        return m_bCanPenetrate;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // PC 0x80FA9C (game offset 0xB8 -> class offset 0xBC). The PS2 record names this entry
        // "m_bCanPenetrate"; it is the only property of the chain (Info.First == 0x80FA9C).
        static Glacier::RTP::ZDataProperty<bool> CanPenetrate{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bCanPenetrate", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateAmmoCustom, m_bCanPenetrate)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemTemplateAmmoCustom,  // ClassName
        ZHM3ItemTemplateAmmo,        // BaseClass
        0x009B1838,                  // OldClassInfoAddr
        "ZHM3ItemTemplateAmmoCustom", // FactoryName
        0x0,                         // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::CanPenetrate,   // FirstProperty
        0x0080FAB0,                  // PropertiesAddr (ZHM3ItemTemplateAmmoCustom::Info)
        0x009B1530,                  // IdAddr (ZHM3ItemTemplateAmmoCustom::m_Id)
        0x009B1534                   // MaskAddr (ZHM3ItemTemplateAmmoCustom::m_Mask)
    );
#   pragma endregion
}
