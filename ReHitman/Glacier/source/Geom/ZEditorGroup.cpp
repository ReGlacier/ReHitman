#include <Glacier/Geom/ZEditorGroup.h>


namespace Glacier
{
    ZEditorGroup::~ZEditorGroup() = default;

    ZEditorGroup::ZEditorGroup(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
    {
    }

    const RTP::ZPropertyInfo& ZEditorGroup::GetProperties() const
    {
        return ZEditorGroup::Info;
    }

    uint32_t ZEditorGroup::GetObjectId() const
    {
        return ZEditorGroup::m_Id;
    }

    void ZEditorGroup::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZEditorGroup::m_Id;
        mask = ZEditorGroup::m_Mask;
    }

    ZGEOMCLASSINFO* ZEditorGroup::GetOldClassInfo() const
    {
        return ZEditorGroup::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZEditorGroup,
        ZGROUP,
        0x009729D8,
        "ZEditorGroup",
        0x0076A988,
        nullptr, // No first property (PC Info.First is null)
        0x00814E9C,
        0x00972978,
        0x0097297C
    );
#   pragma endregion
}
