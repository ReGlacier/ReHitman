#include <Glacier/Physics/ZCollisionWintel.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/SPrimStrips.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZCollisionWintel::~ZCollisionWintel() = default;

    bool ZCollisionWintel::CalcLineCollision(SExtendedImpactInfo* pImpact, uint32_t lPrim, const float* vLineStart, const float* vLineDirection, bool bTwoSided, uint32_t lColiMask)
    {
        // TODO: Finish me
        return false;
    }

    bool ZCollisionWintel::CalcLineCollision(SExtendedImpactInfo* pImpact, const STempStripsUniqueId* pStripId, const float* vLineStart, const float* vLineDirection, bool bTwoSided, uint32_t lColiMask)
    {
        // TODO: Finish me
        return false;
    }
    
    bool ZCollisionWintel::CheckSphereCollision(uint32_t lPrim, uint8_t lColiMask, const float* vSpherePosition, const float* vSphereMatrix, const float* vSphereDimensions)
    {
        // TODO: Finish me
        return false;        
    }

    void ZCollisionWintel::CreateSubStripId(STempStripsUniqueId* pDst, const STempStripsUniqueId* pSrc, const uint32_t lCount)
    {
        pDst->___u0.__s0.lIdLo = lCount | pSrc->___u0.__s0.lIdLo;
        pDst->___u0.__s0.lIdHi = pSrc->___u0.__s0.lIdHi;
    }

    STempStrips* ZCollisionWintel::GetStripsFromPrim(const uint32_t lPrimId)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint32_t ZCollisionWintel::GetPrimFromStripUniqueId(const STempStripsUniqueId* pId)
    {
        return pId->___u0.__s0.lIdHi;
    }

    uint32_t ZCollisionWintel::GetColiBits(const STempStripsUniqueId* pId)
    {
        return *reinterpret_cast<uint32_t*>(&g_pRenderDll->m_pPrimBuffer[pId->___u0.__s0.lIdHi + 0x34]); // wtf is 0x34?
    }

    char* ZCollisionWintel::GetStripVerticesFromId(ZRawStrip* pRawStrips, const STempStripsUniqueId* pId, uint32_t lColiMask, const float* m0, const float* p0, const float* s0)
    {
        // TODO: Finish me
        return nullptr;
    }

    bool ZCollisionWintel::CheckLineCollision(const STempStripsUniqueId* pId, const float* vLineStart, const float* pLineDirection, bool bTwoSided, uint32_t lColiMask)
    {
        // TODO: Finish me
        return false;
    }
    
    void ZCollisionWintel::GetStripVerticesStrip(ZRawStrip* pRawStrips, const SPrimStripsWintel* pStrips, const STempStripsUniqueId* pId, const float* m0, float const* p0, float const* s0, bool bDirection, uint32_t lEntryNr, uint32_t lStripLength)
    {
        // TODO: Finish me
    } 

    void ZCollisionWintel::GetStripVerticesObject(ZRawStrip* pRawStrips, SPrimObjectHeader* pPrimHeader, const STempStripsUniqueId* pId, const float* m0, const float* p0, const float* s0)
    {
        // TODO: Finish me
    }
    
    void ZCollisionWintel::GetStripVerticesMesh(ZRawStrip* pRawStrips,uint32_t,ZPrimAccessMesh* pPrimAccess, const STempStripsUniqueId* pId, const float* m0, const float* p0, const float* s0)
    {
        // TODO: Finish me
    }

    ZCollisionBase* ZCollisionBase::InitCollision(bool bPackEnabled)
    {
        ZASSERT(!bPackEnabled); // from original IOI code

        auto* pInstance = ZUniMemory::New<ZCollisionWintel>(bPackEnabled);
        ZCollisionBase::s_pCollisionBase = pInstance;
        return pInstance;
    }
}