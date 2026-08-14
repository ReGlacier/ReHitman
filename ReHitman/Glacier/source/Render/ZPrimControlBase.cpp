#include <Glacier/Render/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZPrimControlBase* ZPrimControlBase::Instance()
    {
        if (!g_pRenderDll) return nullptr;

        return g_pRenderDll->m_pPrimControl;
    }

    void ZPrimControlBase::GetPrimExtraData(uint32_t, uint32_t, void*, uint32_t)
    {
        // Do nothing
    }

    uint8_t ZPrimControlBase::GetPrimPackType(uint32_t lPrim)
    {
        const auto* pData = GetPrimData(lPrim);
        ZASSERT(pData);

        return reinterpret_cast<const SPrimHeader*>(pData)->lPackType;
    }
    
    const void* ZPrimControlBase::GetPrimData(uint32_t lPrim)
    {
        if (!lPrim)
        {
            return nullptr;
        }

        ZASSERT(g_pRenderDll && g_pRenderDll->m_pPrimBuffer);
        return g_apPrimHandleToPointerTable[lPrim];
    }

    uint32_t ZPrimControlBase::GetPrimOffset(const void* ptr)
    {
        ZASSERT(false);
        return 0u;
    }

    bool ZPrimControlBase::CheckInPrimBuffer(const void* ptr)
    {
        return ptr >= g_pRenderDll->m_pPrimBuffer && ptr < g_pRenderDll->m_pPrimBufferEnd;
    }

    bool ZPrimControlBase::IsPrimUnique(uint32_t lPrim)
    {
        return lPrim >= g_lPrimHandleToPointerCount;
    }
    
    bool ZPrimControlBase::CheckPointInsidePrim(uint32_t lPrim, const ZVector3& vPoint, float fThreshold)
    {
        // TODO: Finish me
        return false;
    }
}