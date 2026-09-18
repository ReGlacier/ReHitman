#include <Glacier/GUI/Font/ZKerningFont.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    ZKerningFont::ZKerningFont(const char* psName, ZBaseGeom* pBaseGeom)
        : ZTTFONT(psName, pBaseGeom)
    {
    }

    int32_t ZKerningFont::GetKerning(unsigned int uFirstChar, unsigned int uSecondChar) const
    {
        return ZTTFONT::GetKerning(uFirstChar, uSecondChar) - m_iExtraKerning;
    }

    const RTP::ZPropertyInfo& ZKerningFont::GetProperties() const
    {
        return ZKerningFont::Info;
    }

    uint32_t ZKerningFont::GetObjectId() const
    {
        return ZKerningFont::m_Id;
    }

    void ZKerningFont::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZKerningFont::m_Id;
        mask = ZKerningFont::m_Mask;
    }

    ZGEOMCLASSINFO* ZKerningFont::GetOldClassInfo() const
    {
        return ZKerningFont::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<int> ExtraKerning
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "m_iExtraKerning", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZKerningFont, m_iExtraKerning)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZKerningFont,
        ZTTFONT,
        0x009A2EC0,
        "ZKerningFont",
        0x0077FDEC,
        cProperties::ExtraKerning,
        0x0080ECEC,
        0x009A2E70,
        0x009A2E74
    );
#   pragma endregion
}
