#include <Glacier/Geom/ZSHADOWMESHOBJ.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    ZSHADOWMESHOBJ::ZSHADOWMESHOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom),
          m_iFlags(0)
    {
    }

    ZSHADOWMESHOBJ::~ZSHADOWMESHOBJ() = default;

    const RTP::ZPropertyInfo& ZSHADOWMESHOBJ::GetProperties() const
    {
        return ZSHADOWMESHOBJ::Info;
    }

    uint32_t ZSHADOWMESHOBJ::GetObjectId() const
    {
        return ZSHADOWMESHOBJ::m_Id;
    }

    void ZSHADOWMESHOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSHADOWMESHOBJ::m_Id;
        mask = ZSHADOWMESHOBJ::m_Mask;
    }

    ZGEOMCLASSINFO* ZSHADOWMESHOBJ::GetOldClassInfo() const
    {
        return ZSHADOWMESHOBJ::m_OldClassInfo;
    }

    bool ZSHADOWMESHOBJ::DisableParentBoundAdjust()
    {
        return true;
    }

    void ZSHADOWMESHOBJ::ClassInit()
    {
        m_pLight = ref_cast<ZLIGHT>(m_rLight);
    }

    void ZSHADOWMESHOBJ::CopyData(const ZGEOM* Source)
    {
        Source->IsDerivedFrom<ZSHADOWMESHOBJ>();
        ZGEOM::CopyData(Source);
    }

    bool ZSHADOWMESHOBJ::GetParentBox(float* pMin, float* pMax)
    {
        return false;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<ZGEOMREF> NamespaceItem_267
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_rLight",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__5,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZSHADOWMESHOBJ, m_rLight))
        };

        static RTP::ZDataProperty<int> NamespaceItem_268
        {
            .m_Node = {
                .m_Next = NamespaceItem_267,
                .m_Name = "m_iFlags",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__7,
            .m_Offset = CLASS_PROPERTY(ZSHADOWMESHOBJ, m_iFlags)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZSHADOWMESHOBJ,
        ZSTDOBJ,
        0x0097B8A8,
        "ZSHADOWMESHOBJ",
        0x0076FA60,
        cProperties::NamespaceItem_268,
        0x00807D24,
        0x0097B858,
        0x0097B85C
    );
#   pragma endregion
}
