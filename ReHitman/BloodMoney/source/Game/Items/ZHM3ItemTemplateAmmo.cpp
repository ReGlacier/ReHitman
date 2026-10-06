#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmo.h>

#include <BloodMoney/Game/Items/ZHM3ItemTool.h>

namespace Hitman
{
    // PC 0x64B070. The HM3 ammo template resolves its item type from the object name (the PC ctor
    // stores ZHM3ItemTool::GetHM3Type(psName) at +0xA4, i.e. m_eHM3ItemType).
    ZHM3ItemTemplateAmmo::ZHM3ItemTemplateAmmo(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItemTemplateAmmo(psName, pBaseGeom)
    {
        m_eHM3ItemType = ZHM3ItemTool::GetHM3Type(psName);
    }

    // vtbl slot 12 (RTTI)  PC 0x64A8D0
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemTemplateAmmo::GetProperties() const
    {
        return ZHM3ItemTemplateAmmo::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64B340
    uint32_t ZHM3ItemTemplateAmmo::GetObjectId() const
    {
        return ZHM3ItemTemplateAmmo::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64B350
    void ZHM3ItemTemplateAmmo::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemTemplateAmmo::m_Id;
        mask = ZHM3ItemTemplateAmmo::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64A8E0 -> &ZHM3ItemTemplateAmmo::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemTemplateAmmo::GetOldClassInfo() const
    {
        return ZHM3ItemTemplateAmmo::m_OldClassInfo;
    }

    // PC 0x64A900
    EHM3ItemType ZHM3ItemTemplateAmmo::GetHM3ItemType()
    {
        return m_eHM3ItemType;
    }

#   pragma region " --- RTTI --- "
    // ZHM3ItemTemplateAmmo adds no serialized properties of its own: the PC Info record at 0x80FA84
    // has First == 0 (only a Super link to ZItemTemplateAmmo::Info at 0x80C8E4). The PS2 Info at
    // 0x94C688 also has First == 0.
    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemTemplateAmmo,        // ClassName
        Glacier::ZItemTemplateAmmo,  // BaseClass
        0x009B1798,                  // OldClassInfoAddr
        "ZHM3ItemTemplateAmmo",      // FactoryName
        0x0,                         // FactoryNameAddr (documentation only; not bound by the macro)
        nullptr,                     // FirstProperty (ZHM3ItemTemplateAmmo::Info.First == 0)
        0x0080FA84,                  // PropertiesAddr (ZHM3ItemTemplateAmmo::Info)
        0x009B1520,                  // IdAddr (ZHM3ItemTemplateAmmo::m_Id)
        0x009B1524                   // MaskAddr (ZHM3ItemTemplateAmmo::m_Mask)
    );
#   pragma endregion
}
