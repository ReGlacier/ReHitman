#include <Glacier/Render/Entry/ZRenderEntrySprite.h>
#include <Glacier/Render/Entry/SRenderEntryInstance.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    ZRenderEntrySprite::~ZRenderEntrySprite() = default;
        
    ZRenderEntrySprite::ZRenderEntrySprite(ZRenderEntryGeomCreateInfo* pInfo)
        : ZRenderEntry()
    {
        ZPrimHandle hPrim{pInfo->m_lPrim};
        
        // TODO: Finish me


        m_ObjectToWorldMatrix.Reset();
        m_bLocal = false;
        m_pSpriteArray = nullptr;

        // Extract prim
        const SPrimHeader* pPrimHeader = hPrim;
        ZASSERT(pPrimHeader->lType == PTSPRITES);
        
        auto* pDraw = IDraw::Instance<ZRenderDraw>();
        auto* pEntry = pDraw->CreateRenderEntryInstance(hPrim, this, nullptr, false);

        if (pEntry)
        {
            // TODO: Finish me (PC sub_476D10: per-instance setup)
            AddRenderEntryInstances(&pEntry, 1u);
        }

        m_lLODLevelsWanted = 0xFF;
    }

    ZRenderEntry::RENDERENTRY_BASETYPE ZRenderEntrySprite::GetType() const
    {
        return ZRenderEntry::RENDERENTRY_BASETYPE::RT_SPRITE;
    }

    void ZRenderEntrySprite::Notify(const SRenderEntryNotifyInfo* pEntry)
    {
        m_lLODLevelsWanted = 0xFF;
    }

    void ZRenderEntrySprite::SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance)
    {
        // PC 0x00476DC0
        if (m_bLocal)
        {
            ZMatrix mMat;
            mmtmul(mMat.m0, m_ObjectToWorldMatrix.m0, pContext->m_WorldToViewMatrix.m0);
            vsub(mMat.p0.Get(), m_ObjectToWorldMatrix.p0.Get(), pContext->m_WorldToViewMatrix.p0.Get());
            vmtmul(mMat.p0.Get(), pContext->m_WorldToViewMatrix.m0.Get());
            pContext->m_ObjectToWorldMatrix = mMat;
        }
        else
        {
            ZRenderEntry::SetRenderContext(pContext, pObjInstance);
        }
    }
    
    void ZRenderEntrySprite::SetLocal(bool bLocal)
    {
        m_bLocal = bLocal;
    }

    void ZRenderEntrySprite::SetSortValue(int32_t lSortValue)
    {
        // PC 0x00476E40
        if (!m_pRenderEntryInstances)
        {
            return;
        }

        for (uint16_t i = 0; i < m_lNumRenderEntryInstances; ++i)
        {
            auto* pRenderObjectInstance = m_pRenderEntryInstances[i]->pRenderObjectInstance;
            pRenderObjectInstance->m_lSortValue = (static_cast<uint32_t>(lSortValue) << 16) | static_cast<uint16_t>(pRenderObjectInstance->m_lSortValue);
        }
    }
}