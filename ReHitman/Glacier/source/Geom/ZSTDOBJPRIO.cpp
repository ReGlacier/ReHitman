#include <Glacier/Geom/ZSTDOBJPRIO.h>
#include <Glacier/RTP/VirtualTables.h>



namespace Glacier
{
    ZSTDOBJPRIO::ZSTDOBJPRIO(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
    {
        m_lSortPriority = 7;
    }

    ZSTDOBJPRIO::~ZSTDOBJPRIO() = default;

    const RTP::ZPropertyInfo& ZSTDOBJPRIO::GetProperties() const
    {
        return ZSTDOBJPRIO::Info;
    }

    uint32_t ZSTDOBJPRIO::GetObjectId() const
    {
        return ZSTDOBJPRIO::m_Id;
    }

    void ZSTDOBJPRIO::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSTDOBJPRIO::m_Id;
        mask = ZSTDOBJPRIO::m_Mask;
    }

    ZGEOMCLASSINFO* ZSTDOBJPRIO::GetOldClassInfo() const
    {
        return ZSTDOBJPRIO::m_OldClassInfo;
    }

    uint8_t ZSTDOBJPRIO::GetSortPriority() const
    {
        return m_lSortPriority;
    }

    void ZSTDOBJPRIO::CopyData(const ZGEOM* Source)
    {
        ZGEOM::CopyData(Source);

        if (const auto* pSource = geom_cast<ZSTDOBJPRIO>(Source))
        {
            m_lSortPriority = pSource->m_lSortPriority;
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<uint8_t> NamespaceItem_1337
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_lSortPriority",
                .m_Filter = 3
            },
            .m_VirtualTable = VirtualTable_DP__32,
            .m_Offset = CLASS_PROPERTY(ZSTDOBJPRIO, m_lSortPriority)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZSTDOBJPRIO,
        ZSTDOBJ,
        0x009B37F0,
        "ZSTDOBJPRIO",
        0x007A27F8,
        cProperties::NamespaceItem_1337,
        0x0080E718,
        0x009B36E8,
        0x009B36EC
    );
#   pragma endregion

}
