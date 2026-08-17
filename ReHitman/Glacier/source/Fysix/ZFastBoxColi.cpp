#include <Glacier/Fysix/ZFastBoxColi.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZFastBoxColi::ZFastBoxColi(float fExtraSpace, int lGeomConMask)
    {
        m_fExtraSpace = fExtraSpace;
        m_lGeomConMask = lGeomConMask;
        m_bBoxSet = 0;
        m_bExtendedMode = 0;
        m_bIgnoreMovingObjects = 0;
        m_vCen = { 0.0f };
        m_vSize = { -1.f };
        m_pFaceList = ZUniMemory::New<REFTAB>(64, 30);
    }

    ZFastBoxColi::~ZFastBoxColi()
    {
        ZUniMemory::Delete(m_pFaceList);
    }

    void ZFastBoxColi::SetBox(ZVector3& vPos, ZMat3x3& mMat, ZVector3& vSize, bool bRefresh)
    {
        // TODO: Finish me
    }
    
    bool ZFastBoxColi::ChkLineColi(const ZVector3& v0, const ZVector3& v1, bool)
    {
        // TODO: Finish me
        return false;
    }
    
    void ZFastBoxColi::Update()
    {
        // TODO: Finish me
    }

    void ZFastBoxColi::EnableExtendedMode(bool value)
    {
        m_bExtendedMode = value;
    }
    
    void ZFastBoxColi::IgnoreMovingObjects(bool value)
    {
        m_bIgnoreMovingObjects = value;
    }

    REFTAB* ZFastBoxColi::GetFacesPtr()
    {
        return m_pFaceList;
    }
    
    uint32_t ZFastBoxColi::GetNumFaces()
    {
        // TODO: Finish me
        return 0;
    }
    
    void ZFastBoxColi::GetFaces()
    {
        // TODO: Finish me
    }

    void ZFastBoxColi::AddFace(const ZBaseGeom* pBaseGeom, const float* pVert0, const float* pVert1, const float* pVert2)
    {
        // TODO: Finish me
    }
}