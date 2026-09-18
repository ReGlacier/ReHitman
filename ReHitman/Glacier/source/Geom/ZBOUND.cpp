#include <Glacier/Geom/ZBOUND.h>

namespace Glacier
{
    ZBOUND::ZBOUND(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
    {
    }

    ZBOUND::~ZBOUND() = default;

    bool ZBOUND::PostLoad(ISerializerStream& stream)
    {
        BaseGeom()->SetControl(ZCINVISIBLE, 0x3);
        return true;
    }

    const RTP::ZPropertyInfo& ZBOUND::GetProperties() const
    {
        return ZBOUND::Info;
    }

    uint32_t ZBOUND::GetObjectId() const
    {
        return ZBOUND::m_Id;
    }

    void ZBOUND::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZBOUND::m_Id;
        mask = ZBOUND::m_Mask;
    }

    ZGEOMCLASSINFO* ZBOUND::GetOldClassInfo() const
    {
        return ZBOUND::m_OldClassInfo;
    }

    eGlobalTreeType ZBOUND::GetBoundTreeType() const
    {
        return eGlobalTreeType::GT_None;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZBOUND,
        ZSTDOBJ,
        0x009727B0,
        "ZBOUND",
        0x00769A18,
        nullptr,
        0x00809574,
        0x00972760,
        0x00972764
    );
#   pragma endregion
}
