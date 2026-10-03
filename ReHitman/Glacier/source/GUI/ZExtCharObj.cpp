#include <Glacier/GUI/ZExtCharObj.h>


namespace Glacier
{
    ZExtCharObj::ZExtCharObj(const char* psName, ZBaseGeom* pBaseGeom)
        : ZCHAROBJ(psName, pBaseGeom)
    {
    }

    ZExtCharObj::~ZExtCharObj() = default;

    const RTP::ZPropertyInfo& ZExtCharObj::GetProperties() const
    {
        return ZExtCharObj::Info;
    }

    uint32_t ZExtCharObj::GetObjectId() const
    {
        return ZExtCharObj::m_Id;
    }

    void ZExtCharObj::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZExtCharObj::m_Id;
        mask = ZExtCharObj::m_Mask;
    }

    ZGEOMCLASSINFO* ZExtCharObj::GetOldClassInfo() const
    {
        return ZExtCharObj::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(ZExtCharObj, ZCHAROBJ, 0x009A2B90, "ZExtCharObj", 0x0077E2FC, nullptr, 0x0080F54C, 0x009A2B40, 0x009A2B44);
#   pragma endregion
}
