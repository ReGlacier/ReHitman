#include <Glacier/Geom/ZTemplate.h>


namespace Glacier
{
    ZTemplate::~ZTemplate() = default;

    ZTemplate::ZTemplate(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
    {
    }

    const RTP::ZPropertyInfo& ZTemplate::GetProperties() const
    {
        return ZTemplate::Info;
    }

    uint32_t ZTemplate::GetObjectId() const
    {
        return ZTemplate::m_Id;
    }

    void ZTemplate::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZTemplate::m_Id;
        mask = ZTemplate::m_Mask;
    }

    ZGEOMCLASSINFO* ZTemplate::GetOldClassInfo() const
    {
        return ZTemplate::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZTemplate,
        ZGROUP,
        0x00972A88,  // OldClassInfo addr
        "ZTemplate", // FactoryName
        0x0076AE40,  // FactoryName Addr
        nullptr,     // No first property (PC Info.First is null)
        0x0080F558,  // Properties Addr
        0x00972A30,  // ID Addr
        0x00972A34   // Mask Addr
    );
#   pragma endregion
}
