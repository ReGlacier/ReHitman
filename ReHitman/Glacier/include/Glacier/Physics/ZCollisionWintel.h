#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimStripsWintel.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Physics/ZRawStrip.h>
#include <cstdint>


namespace Glacier
{
    class ZCollisionWintel : public ZCollisionBase
    {
    public:
        // vtbl
        ~ZCollisionWintel() override;
        bool CalcLineCollision(SExtendedImpactInfo* pImpact, uint32_t lPrim, const float* vLineStart, const float* vLineDirection, bool bTwoSided, uint32_t lColiMask) override;
        bool CalcLineCollision(SExtendedImpactInfo* pImpact, const STempStripsUniqueId* pStripId, const float* vLineStart, const float* vLineDirection, bool bTwoSided, uint32_t lColiMask) override;
        bool CheckSphereCollision(uint32_t lPrim, uint8_t lColiMask, const float* vSpherePosition, const float* vSphereMatrix, const float* vSphereDimensions) override;
        void CreateSubStripId(STempStripsUniqueId* pDst, const STempStripsUniqueId* pSrc, const uint32_t lCount) override;
        STempStrips* GetStripsFromPrim(const uint32_t lPrimId) override;
        uint32_t GetPrimFromStripUniqueId(const STempStripsUniqueId* pId) override;
        uint32_t GetColiBits(const STempStripsUniqueId* pId) override;
        char* GetStripVerticesFromId(ZRawStrip* pRawStrips, const STempStripsUniqueId* pId, uint32_t lColiMask, const float* m0, const float* p0, const float* s0) override;

        virtual bool CheckLineCollision(const STempStripsUniqueId* pId, const float* vLineStart, const float* pLineDirection, bool bTwoSided, uint32_t lColiMask);
        virtual void GetStripVerticesStrip(ZRawStrip* pRawStrips, const SPrimStripsWintel* pStrips, const STempStripsUniqueId* pId, const float* m0, float const* p0, float const* s0, bool bDirection, uint32_t lEntryNr, uint32_t lStripLength);
        virtual void GetStripVerticesObject(ZRawStrip* pRawStrips, SPrimObjectHeader* pPrimHeader, const STempStripsUniqueId* pId, const float* m0, const float* p0, const float* s0);
        virtual void GetStripVerticesMesh(ZRawStrip* pRawStrips,uint32_t,ZPrimAccessMesh* pPrimAccess, const STempStripsUniqueId* pId, const float* m0, const float* p0, const float* s0);

        // methods
        using ZCollisionBase::ZCollisionBase;
    };
    RE_VERIFY_SIZE(ZCollisionWintel, 0x20B4); // must be same as ZCollisionBase
}