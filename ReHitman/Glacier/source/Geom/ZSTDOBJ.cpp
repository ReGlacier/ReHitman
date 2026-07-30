#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Geom/GeomControlMasks.h>
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
        // TODO: Finish after ZCollisionBase reversed
        return false;
    }

    bool ZSTDOBJ::CheckPointInside(ZVector3& pPoint, float fDotDist)
    {
        // TODO: Finish after ZRenderBaseDll reversed
        return false;
    }

    bool ZSTDOBJ::CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* s0)
    {
         // TODO: Finish after ZRenderBaseDll reversed
        return false;
    }

    uint32_t ZSTDOBJ::GetClassId()
    {
        return ZSTDOBJ::m_Id;
    }

    STATIC_CLASS_VAR_IMPL(ZSTDOBJ, RTP::ZPropertyInfo, Info, 0x0097B694, {});
    STATIC_CLASS_VAR_IMPL(ZSTDOBJ, const char*, FactoryName, 0x00769070, "ZSTDOBJ");
    DECLARE_ID_AND_MASK_IMPL(ZSTDOBJ, 0x0097B694, 0x0097B698);
    REGISTER_GLACIER_GEOM_CLASS(ZSTDOBJ, ZGEOM, 0x200002u, 0x0097B6E8);
}