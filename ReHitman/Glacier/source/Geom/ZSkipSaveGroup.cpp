#include <Glacier/Geom/ZSkipSaveGroup.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    ZSkipSaveGroup::ZSkipSaveGroup(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
    {
    }

    ZSkipSaveGroup::~ZSkipSaveGroup() = default;

    const RTP::ZPropertyInfo& ZSkipSaveGroup::GetProperties() const
    {
        return ZSkipSaveGroup::Info;
    }

    uint32_t ZSkipSaveGroup::GetObjectId() const
    {
        return ZSkipSaveGroup::m_Id;
    }

    void ZSkipSaveGroup::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSkipSaveGroup::m_Id;
        mask = ZSkipSaveGroup::m_Mask;
    }

    ZGEOMCLASSINFO* ZSkipSaveGroup::GetOldClassInfo() const
    {
        return ZSkipSaveGroup::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZSkipSaveGroup,
        ZGROUP,
        0x0099C930,
        "ZSkipSaveGroup",
        0x00776AB0,
        nullptr, // No first property
        0x0080D1FC,
        0x0099C8E0,
        0x0099C8E4
    );
#   pragma endregion
}
