#include <Glacier/Items/ZItemAmmo.h>

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    // vtbl slot 173  PS2 0x279844 / iOS 0x10038BE40
    ZItemAmmo::ZItemAmmo(const char* psName, ZBaseGeom* pBaseGeom)
        : ZItem(psName, pBaseGeom)
    {
        m_lNrProjectiles = 0;
    }

    ZItemAmmo::~ZItemAmmo() = default;

    // vtbl slot 12  PC 0x50FE50
    const RTP::ZPropertyInfo& ZItemAmmo::GetProperties() const
    {
        return ZItemAmmo::Info;
    }

    // vtbl slot 13  PC 0x510B20
    uint32_t ZItemAmmo::GetObjectId() const
    {
        return ZItemAmmo::m_Id;
    }

    // vtbl slot 14  PC 0x510B30
    void ZItemAmmo::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItemAmmo::m_Id;
        mask = ZItemAmmo::m_Mask;
    }

    // vtbl slot 15  PC 0x50FE60
    ZGEOMCLASSINFO* ZItemAmmo::GetOldClassInfo() const
    {
        return ZItemAmmo::m_OldClassInfo;
    }

    // PC 0x707DD0
    int ZItemAmmo::GetNrProjectiles()
    {
        return m_lNrProjectiles;
    }

    // PC 0x707DC0
    int ZItemAmmo::SetNrProjectiles(int value)
    {
        m_lNrProjectiles = value;
        return value;
    }

    // PC 0x50FE80
    void ZItemAmmo::AddNrProjectiles(int amount)
    {
        m_lNrProjectiles += amount;
    }

    // PC 0x50FEA0
    void ZItemAmmo::SubNrProjectiles(int amount)
    {
        m_lNrProjectiles -= amount;
    }

    // vtbl slot 104  PS2 0x279A3C
    void ZItemAmmo::CopyData(const ZGEOM* Source)
    {
        ZItem::CopyData(Source);

        if (Source->IsDerivedFrom<ZItemAmmo>())
        {
            const ZItemAmmo* pSource = static_cast<const ZItemAmmo*>(Source);
            m_lNrProjectiles = pSource->m_lNrProjectiles;
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<int> NrProjectiles{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_lNrProjectiles", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZItemAmmo, m_lNrProjectiles)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItemAmmo, // ClassName
        ZItem, // BaseClass
        0x0099C1B8, // OldClassInfoAddr
        "ZItemAmmo", // FactoryName
        0x00773EF4, // FactoryNameAddr
        cProperties::NrProjectiles, // FirstProperty
        0x0080C904, // PropertiesAddr
        0x0099BF60, // IdAddr
        0x0099BF64 // MaskAddr
    );
#   pragma endregion
}
