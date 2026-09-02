#include <Glacier/GUI/ZFONT.h>


namespace Glacier
{
    ZFONT::ZFONT(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINOBJ(psName, pBaseGeom)
    {
    }

    ZFONT::~ZFONT() = default;

    const RTP::ZPropertyInfo& ZFONT::GetProperties() const
    {
        return ZFONT::Info;
    }

    uint32_t ZFONT::GetObjectId() const
    {
        return ZFONT::m_Id;
    }

    void ZFONT::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZFONT::m_Id;
        mask = ZFONT::m_Mask;
    }

    ZGEOMCLASSINFO* ZFONT::GetOldClassInfo() const
    {
        return ZFONT::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_PURE_IMPL(ZFONT, ZWINOBJ, 0x009A2928, "ZFONT", 0x0077CFA4, nullptr, 0x0080A8F4, 0x009A27DC, 0x009A27E0);
#   pragma endregion
}
