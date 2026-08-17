#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GlacierFWD.h>


namespace Glacier
{
    struct ZFastBoxColi
    {
        // methods
        ZFastBoxColi(float fExtraSpace, int lGeomConMask);
        ~ZFastBoxColi();

        void SetBox(ZVector3& vPos, ZMat3x3& mMat, ZVector3& vSize, bool bRefresh);
        bool ChkLineColi(const ZVector3& v0, const ZVector3& v1, bool);
        void Update();
        void EnableExtendedMode(bool);
        void IgnoreMovingObjects(bool);
        REFTAB* GetFacesPtr();
        uint32_t GetNumFaces();
        void GetFaces();
        void AddFace(const ZBaseGeom* pBaseGeom, const float* pVert0, const float* pVert1, const float* pVert2);

        // members
        REFTAB* m_pFaceList;
        bool m_bBoxSet;
        ZVector3 m_vCen;
        ZMat3x3 m_m;
        ZVector3 m_vSize;
        float m_fExtraSpace;
        int32_t m_lGeomConMask;
        bool m_bExtendedMode;
        bool m_bIgnoreMovingObjects;
        uint32_t m_lNrTrisInAll;
    };
    RE_VERIFY_SIZE(ZFastBoxColi, 0x54);
}