#include <Glacier/Geom/Z2DOBJ.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    Z2DOBJ::Z2DOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
    {
        m_bRollAlign = false;
    }

    Z2DOBJ::~Z2DOBJ() = default;

    const RTP::ZPropertyInfo& Z2DOBJ::GetProperties() const
    {
        return Z2DOBJ::Info;
    }

    uint32_t Z2DOBJ::GetObjectId() const
    {
        return Z2DOBJ::m_Id;
    }

    void Z2DOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = Z2DOBJ::m_Id;
        mask = Z2DOBJ::m_Mask;
    }

    ZGEOMCLASSINFO* Z2DOBJ::GetOldClassInfo() const
    {
        return Z2DOBJ::m_OldClassInfo;
    }

    void Z2DOBJ::CalcCenSize()
    {
        BaseGeom()->CalcCenSize(true);

        float fMax = BaseGeom()->m_vSize.x;
        if (fMax < BaseGeom()->m_vSize.y)
        {
            fMax = BaseGeom()->m_vSize.y;
        }
        if (fMax < BaseGeom()->m_vSize.z)
        {
            fMax = BaseGeom()->m_vSize.z;
        }

        SetSize(ZVector3(fMax, fMax, fMax));
    }

    void Z2DOBJ::CopyData(const ZGEOM* Source)
    {
        if (const auto* pSource = geom_cast<Z2DOBJ>(Source))
        {
            m_bRollAlign = pSource->m_bRollAlign;
        }

        ZGEOM::CopyData(Source);
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<bool> NamespaceItem_1324
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_bRollAlign",
                .m_Filter = 3
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(Z2DOBJ, m_bRollAlign)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        Z2DOBJ,
        ZSTDOBJ,
        0x009726A8,
        "Z2DOBJ",
        0x00769254,
        cProperties::NamespaceItem_1324,
        0x0080B974,
        0x00972658,
        0x0097265C
    );
#   pragma endregion
}
