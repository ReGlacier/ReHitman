#include <Glacier/Geom/ZCigs.h>



namespace Glacier
{
    ZCigs::~ZCigs() = default;

    ZCigs::ZCigs(const char* psName, ZBaseGeom* pBaseGeom)
        : ZAllocMany(psName, pBaseGeom)
    {
    }

    const RTP::ZPropertyInfo& ZCigs::GetProperties() const
    {
        return ZCigs::Info;
    }

    uint32_t ZCigs::GetObjectId() const
    {
        return ZCigs::m_Id;
    }

    void ZCigs::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZCigs::m_Id;
        mask = ZCigs::m_Mask;
    }

    ZGEOMCLASSINFO* ZCigs::GetOldClassInfo() const
    {
        return ZCigs::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZCigs,
        ZAllocMany,
        0x009B08E8,
        "ZCigs",
        0x00770154,
        nullptr,
        0x0080F208,
        0x009B0898,
        0x009B089C
    );
#   pragma endregion
}
