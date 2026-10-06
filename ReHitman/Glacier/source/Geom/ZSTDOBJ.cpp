#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    ZSTDOBJ::ZSTDOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGEOM(psName, pBaseGeom)
    {
    }

    ZSTDOBJ::~ZSTDOBJ() = default;

    const RTP::ZPropertyInfo& ZSTDOBJ::GetProperties() const
    {
        return ZSTDOBJ::Info;
    }

    uint32_t ZSTDOBJ::GetObjectId() const
    {
        return ZSTDOBJ::GetClassId();
    }

    void ZSTDOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSTDOBJ::GetClassId();
        mask = ZSTDOBJ::m_Mask;
    }

    ZGEOMCLASSINFO* ZSTDOBJ::GetOldClassInfo() const
    {
        // Actually, it must return pointer ZSTDOBJ_Producer, but they are same
        return ZSTDOBJ::m_OldClassInfo;
    }

    void ZSTDOBJ::CalcCenSize()
    {
        BaseGeom()->CalcCenSize(true);
    }

    void ZSTDOBJ::Select(bool OnOff)
    {
        ZGEOM::Select(OnOff);

        if ((ZGEOM::GeomControl() & 0xE) == 0)
        {
            SetGeomControl(0, 1u);
        }
    }

    bool ZSTDOBJ::RequestCustomDraw()
    {
        if (!ZGEOM::Is<ZSTDOBJ>())
        {
            SetGeomControl(0x20, 0);
        }

        return ZGEOM::RequestCustomDraw();
    }

    eGlobalTreeType ZSTDOBJ::GetBoundTreeType() const
    {
        return eGlobalTreeType::GT_StdObjs;
    }

    void ZSTDOBJ::DispBound(bool OnOff)
    {
        ZSTDOBJ::Select(OnOff);

        if ((GeomControl() & 0xE) == 0)
        {
            SetGeomControl(0, 1u);
        }
    }

    void ZSTDOBJ::ClassInit()
    {
        if (Control() & ZCHASDYNAMICPARENT)
        {
            SetControl(0x2000000, 0);
        }
    }

    bool ZSTDOBJ::ChkLineColi(COLI* pColi, bool bWantInvisible)
    {
        std::ignore = bWantInvisible; // unused

        SExtendedImpactInfo sImpactInfo {};
        ZVector3 vLP = pColi->lp, vLN = pColi->ln;

        if (ZCollisionBase::GetCollisionInterface()->CalcLineCollision(&sImpactInfo, Prim(), vLP, vLN, pColi->m_bBothSides, 0xFFFFFFFFu))
        {
            pColi->ColiRef = sImpactInfo.pBaseGeom->GetRef();
            pColi->t = sImpactInfo.fPercent;
            pColi->cp = sImpactInfo.vPosition;
            pColi->m_HitCache = sImpactInfo.m_HitCache;
            return true;
        }

        return false;
    }

    bool ZSTDOBJ::CheckPointInside(ZVector3& pPoint, float fDotDist)
    {
        return ZPrimControlBase::Instance()->CheckPointInsidePrim(Prim(), pPoint, fDotDist);
    }

    bool ZSTDOBJ::CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* s0)
    {
        return ZPrimControlBase::Instance()->CheckBoxInsidePrim(Prim(), mMat, vPos, s0);
    }

    uint32_t ZSTDOBJ::GetClassId()
    {
        return ZSTDOBJ::m_Id;
    }

    void ZSTDOBJ::SetInvisible(const bool&)
    {
        // Do nothing
    }

    void ZSTDOBJ::GetInvisible(bool&)
    {
        // Do nothing
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        struct RTP::ZVirtualProperty<bool> NamespaceItem_1286 {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "Invisible",
                .m_Filter = 3
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZSTDOBJ::GetInvisible,
            .m_Set = &ZSTDOBJ::SetInvisible
        };
    }

    STATIC_CLASS_VAR_IMPL(ZSTDOBJ, RTP::ZPropertyInfo, Info, 0x0097B694, (RTP::ZPropertyInfo {
        .First = cProperties::NamespaceItem_1286,
        .Super = &ZGEOM::Info,
        .Name = ZSTDOBJ::FactoryName
    }));
    STATIC_CLASS_VAR_IMPL(ZSTDOBJ, const char*, FactoryName, 0x00769070, "ZSTDOBJ");
    DECLARE_ID_AND_MASK_IMPL(ZSTDOBJ, 0x0097B694, 0x0097B698);
    REGISTER_GLACIER_GEOM_CLASS(ZSTDOBJ, ZGEOM, 0x200002u, 0x0097B6E8);
#   pragma endregion
}
