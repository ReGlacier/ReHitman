#include <BloodMoney/Game/Items/ZHM3ItemAmmo.h>

#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmo.h>

namespace Hitman
{
    // PC 0x64B0E0. Only chains to the ZItemAmmo base ctor; the HM3 variant adds no data members.
    ZHM3ItemAmmo::ZHM3ItemAmmo(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItemAmmo(psName, pBaseGeom)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64A920
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemAmmo::GetProperties() const
    {
        return ZHM3ItemAmmo::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64B370
    uint32_t ZHM3ItemAmmo::GetObjectId() const
    {
        return ZHM3ItemAmmo::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64B380
    void ZHM3ItemAmmo::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemAmmo::m_Id;
        mask = ZHM3ItemAmmo::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64A930 -> &ZHM3ItemAmmo::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemAmmo::GetOldClassInfo() const
    {
        return ZHM3ItemAmmo::m_OldClassInfo;
    }

    // PC 0x64CBD0
    EHM3ItemType ZHM3ItemAmmo::GetHM3ItemType()
    {
        Glacier::ZItemTemplate* pTemplate = GetItemTemplate();
        if (pTemplate != nullptr &&
            (ZHM3ItemTemplateAmmo::m_Mask & pTemplate->GetObjectId()) == ZHM3ItemTemplateAmmo::m_Id)
        {
            return static_cast<ZHM3ItemTemplateAmmo*>(pTemplate)->GetHM3ItemType();
        }

        return EHM3ItemType::eHM3NoType;
    }

#   pragma region " --- RTTI --- "
    // ZHM3ItemAmmo adds no serialized properties of its own: the PC Info record at 0x80FA90 has
    // First == 0 (only a Super link to ZItemAmmo::Info at 0x80C904). The PS2 Info at 0x94C698 also
    // has First == 0.
    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemAmmo,          // ClassName
        Glacier::ZItemAmmo,    // BaseClass
        0x009B17E8,            // OldClassInfoAddr
        "ZHM3ItemAmmo",        // FactoryName
        0x0,                   // FactoryNameAddr (documentation only; not bound by the macro)
        nullptr,               // FirstProperty (ZHM3ItemAmmo::Info.First == 0)
        0x0080FA90,            // PropertiesAddr (ZHM3ItemAmmo::Info)
        0x009B1528,            // IdAddr (ZHM3ItemAmmo::m_Id)
        0x009B152C             // MaskAddr (ZHM3ItemAmmo::m_Mask)
    );
#   pragma endregion
}
