#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Glacier
{
    class ZFastBoxColi
    {
    public:
        // constants
        static constexpr uint32_t kMaxStripsInsideBox = 204800; // stack buffer size for GetStripsInsideBox (XBOX: 0x32000)

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

    private:
        struct SFace // binary layout of one m_pFaceList element (0x7C bytes)
        {
            ZREF m_GeomRef;            // +0x00
            ZVector3 m_vNormal;        // +0x04
            float m_aInvSys[9];        // +0x10
            ZVector3 m_vVert0;         // +0x34
            ZVector3 m_vVert1;         // +0x40 (extended mode)
            ZVector3 m_vVert2;         // +0x4C (extended mode)
            ZVector3 m_vEdgeNormal0;   // +0x58 (extended mode)
            ZVector3 m_vEdgeNormal1;   // +0x64 (extended mode)
            ZVector3 m_vEdgeNormal2;   // +0x70 (extended mode)
        };
    };
    RE_VERIFY_SIZE(ZFastBoxColi, 0x54); // Verified PC alloc at ConstrainedParticleSystem::FindFaces
}
