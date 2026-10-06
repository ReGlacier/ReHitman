#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Grid of axis-aligned sub-boxes used for local point-containment tests.
     *
     * Reversed from `Z:\code\engine\geomsbase\zcubegrid.cpp`. Each sub-box is split
     * into five tetrahedra (`GetThetrasFromSubBox`) and a local point is considered
     * inside if it lies within any of them (`IsLocalPointInside`). This is the fast
     * path consumed by `ZActorHeroCheckInside::IsInside`.
     */
    class ZCubeGrid : public ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZCubeGrid, 0x2000E7u);

        // constants
        static constexpr float kfDefaultCubeSize = 100.0f;

        // types
        struct ZSubBox
        {
            uint16_t  m_lSubBoxNr;        // +0x00
            uint16_t  m_lEnabled;         // +0x02
            uint16_t  m_SideSubBoxes[6];  // +0x04
            ZVector3  m_vCenter;          // +0x10
            float     m_vCorners[24];     // +0x1C (8 corners, 3 floats each)
        };

        struct ZSubBoxHeader
        {
            uint32_t  m_lVersion;         // +0x00
            uint32_t  m_lNrSubBoxes;      // +0x04
            ZVector3  m_vSize;            // +0x08
            ZVector3  m_vBaseCubeSize;    // +0x14
        };

        // vtbl
        ~ZCubeGrid() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;

        // methods
        ZCubeGrid(const char* psName, ZBaseGeom* pBaseGeom);

        bool IsLocalPointInside(const float* pPoint) const;

#       pragma region " --- RTTI Methods --- "
        // Reversed from PS2 0x208908 / XBOX_KL2 0x823D6658 (SetSubBoxes) and
        // PS2 0x208878 / XBOX_KL2 0x823D63E0 (GetSubBoxes). The original groups these
        // as private, but the RTP property table in `cProperties` takes their address,
        // so they are exposed here just like ZSTDOBJ's `GetInvisible`/`SetInvisible`.
        void GetSubBoxes(ZRawData& data);
        void SetSubBoxes(const ZRawData& data);
#       pragma endregion

        // members
        ZVector3 m_vSize;         // +0x10, defaulted to (100, 100, 100)
        ZVector3 m_vCenter;       // +0x1C
        ZVector3 m_vBaseCubeSize; // +0x28
        ZSubBox* m_pSubBoxes;     // +0x34
        uint32_t m_lNrSubBoxes;   // +0x38

    protected:
        void GetThetrasFromSubBox(float* pThetras, const ZSubBox* pSubBox) const;
        bool CheckPointInsideThetra(const float* pPoint, const float* pThetra) const;
        bool CheckPointInsideSubBox(const float* pPoint, const ZSubBox* pSubBox) const;
    };
    RE_VERIFY_SIZE(ZCubeGrid, 0x3C); // Verified against PC ZNonResourceClassInfo (class id 0x2000E7)
    RE_VERIFY_OFFSET(ZCubeGrid, m_vSize, 0x10);
    RE_VERIFY_OFFSET(ZCubeGrid, m_vCenter, 0x1C);
    RE_VERIFY_OFFSET(ZCubeGrid, m_vBaseCubeSize, 0x28);
    RE_VERIFY_OFFSET(ZCubeGrid, m_pSubBoxes, 0x34);
    RE_VERIFY_OFFSET(ZCubeGrid, m_lNrSubBoxes, 0x38);
    RE_VERIFY_SIZE(ZCubeGrid::ZSubBox, 0x7C);
    RE_VERIFY_OFFSET(ZCubeGrid::ZSubBox, m_vCenter, 0x10);
    RE_VERIFY_OFFSET(ZCubeGrid::ZSubBox, m_vCorners, 0x1C);
    RE_VERIFY_SIZE(ZCubeGrid::ZSubBoxHeader, 0x20);
}
