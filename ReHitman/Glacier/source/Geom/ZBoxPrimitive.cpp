#include <Glacier/Geom/ZBoxPrimitive.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    // debug stuff
    static ZDebugInt g_iDrawBoxes { "DebugDrawBoxes", "Whether or not to draw ZBoxPrimitives", 0, 0, 1, 1, "Debug/" };

    ZBoxPrimitive::ZBoxPrimitive(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
        , m_vScale(10.0f)
    {
    }

    ZBoxPrimitive::~ZBoxPrimitive() = default;

    bool ZBoxPrimitive::PostLoad(ISerializerStream& stream)
    {
        Display(true);
        return true;
    }

    const RTP::ZPropertyInfo& ZBoxPrimitive::GetProperties() const
    {
        return ZBoxPrimitive::Info;
    }

    uint32_t ZBoxPrimitive::GetObjectId() const
    {
        return ZBoxPrimitive::m_Id;
    }

    void ZBoxPrimitive::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZBoxPrimitive::m_Id;
        mask = ZBoxPrimitive::m_Mask;
    }

    ZGEOMCLASSINFO* ZBoxPrimitive::GetOldClassInfo() const
    {
        return ZBoxPrimitive::m_OldClassInfo;
    }

    void ZBoxPrimitive::CalcCenSize()
    {
        ZGEOM::SetSize(m_vScale);

        const float fRadius = std::sqrtf(
            (BaseGeom()->Size()[0] * BaseGeom()->Size()[0]) +
            (BaseGeom()->Size()[1] * BaseGeom()->Size()[1]) +
            (BaseGeom()->Size()[2] * BaseGeom()->Size()[2])
        );

        ZGEOM::SetRadius(fRadius);
    }

    bool ZBoxPrimitive::CheckPointInside(ZVector3& pPoint, float fDotDist)
    {
        return (m_vScale[0] - fDotDist) >= fabsf(pPoint[0]) &&
               (m_vScale[1] - fDotDist) >= fabsf(pPoint[1]) &&
               (m_vScale[2] - fDotDist) >= fabsf(pPoint[2]);
    }

    bool ZBoxPrimitive::CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* s0)
    {
        ZVector3 v0;
        ZMat3x3 m0;

        return rectBoxColi(m0, v0, m_vScale, mMat, vPos, s0);
    }

    void ZBoxPrimitive::CopyData(const ZGEOM* Source)
    {
        const auto* pSource = static_cast<const ZBoxPrimitive*>(Source);

        ZGEOM::CopyData(pSource);

        m_vScale = pSource->m_vScale;
        ZGEOM::SetSize(m_vScale);
        Display(true);
    }

    void ZBoxPrimitive::CalcPositionInBox(ZVector3& pos)
    {
        const float fsZ = g_pSysInterface->FRand(nullptr, 0);
        const float fsY = g_pSysInterface->FRand(nullptr, 0);
        const float fsX = g_pSysInterface->FRand(nullptr, 0);

        pos = {
            (2 * fsX - 1.0f) * m_vScale.x,
            (2 * fsY - 1.0f) * m_vScale.y,
            (2 * fsZ - 1.0f) * m_vScale.z,
        };

        ZGEOM::GetRootPoint(pos);
    }

    bool ZBoxPrimitive::GetClosestPosDirInBox(const ZVector3& vTarget, float edgeDist, ZVector3& pos, ZVector3& dir)
    {
        const ZVector3 vScaleMinusEdge = m_vScale - ZVector3 { edgeDist };
        dir.Reset();

        // X-Axis
        const float fSignX = (vTarget.x < 0.0f) ? -1.0f : 1.0f;
        if (std::fabsf(vTarget.x) > vScaleMinusEdge.x)
        {
            pos.x = vScaleMinusEdge.x * fSignX;
            dir.x = fSignX;
        }
        else
        {
            pos.x = vTarget.x;
        }

        // Y-Axis
        const float fSignY = (vTarget.y < 0.0f) ? -1.0f : 1.0f;
        if (std::fabsf(vTarget.y) > vScaleMinusEdge.y)
        {
            pos.y = vScaleMinusEdge.y * fSignY;
            dir.y = fSignY;
        }
        else
        {
            pos.y = vTarget.y;
        }

        // Z-Axis
        const float fSignZ = (vTarget.z < 0.0f) ? -1.0f : 1.0f;
        if (std::fabsf(vTarget.z) > vScaleMinusEdge.z)
        {
            pos.z = vScaleMinusEdge.z * fSignZ;
            dir.z = fSignZ;
        }
        else
        {
            pos.z = vTarget.z;
        }

        CheckPointInside(pos, 0.0f); // Why? In asm it's just vftable call without result usage
        vnorm(dir);

        GetRootPoint(pos);
        GetRootVect(dir);

        return true;
    }

    void ZBoxPrimitive::SetScale(float x, float y, float z)
    {
        m_vScale = { x, y, z };
    }

    void ZBoxPrimitive::GetScale(ZVector3& vScale)
    {
        vScale = m_vScale;
    }

    void ZBoxPrimitive::GetScaleX(float& fScaleX)
    {
        fScaleX = m_vScale.x;
    }

    void ZBoxPrimitive::SetScaleX(const float& fScaleX)
    {
        m_vScale.x = fScaleX;
    }

    void ZBoxPrimitive::GetScaleY(float& fScaleY)
    {
        fScaleY = m_vScale.y;
    }

    void ZBoxPrimitive::SetScaleY(const float& fScaleY)
    {
        m_vScale.y = fScaleY;
    }

    void ZBoxPrimitive::GetScaleZ(float& fScaleZ)
    {
        fScaleZ = m_vScale.z;
    }

    void ZBoxPrimitive::SetScaleZ(const float& fScaleZ)
    {
        m_vScale.z = fScaleZ;
    }

#   pragma region "RTTI"
    namespace cProperties
    {
        static RTP::ZVirtualProperty<float> NamespaceItem_65 {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "scaleZ",
                .m_Filter = 5
            },
            .m_VirtualTable = VirtualTable_VP__11,
            .m_Get = &ZBoxPrimitive::GetScaleZ,
            .m_Set = &ZBoxPrimitive::SetScaleZ
        };

        static RTP::ZVirtualProperty<float> NamespaceItem_64 {
            .m_Node = {
                .m_Next = NamespaceItem_65,
                .m_Name = "scaleY",
                .m_Filter = 5
            },
            .m_VirtualTable = VirtualTable_VP__11,
            .m_Get = &ZBoxPrimitive::GetScaleY,
            .m_Set = &ZBoxPrimitive::SetScaleY
        };

        static RTP::ZVirtualProperty<float> NamespaceItem_63 {
            .m_Node = {
                .m_Next = NamespaceItem_64,
                .m_Name = "scaleX",
                .m_Filter = 5
            },
            .m_VirtualTable = VirtualTable_VP__11,
            .m_Get = &ZBoxPrimitive::GetScaleX,
            .m_Set = &ZBoxPrimitive::SetScaleX
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZBoxPrimitive, // ClassName
        ZSTDOBJ,     // BaseClassName
        0x00973300, // OldClassInfo addr
        "ZBoxPrimitive", // FactoryName
        0x0076D354, // FactoryName Addr
        cProperties::NamespaceItem_63, // FirstProperty
        0x00806C88, // Properties Addr
        0x009732AC, // ID Addr
        0x009732B0 // Mask Addr
    );
#   pragma endregion
}
