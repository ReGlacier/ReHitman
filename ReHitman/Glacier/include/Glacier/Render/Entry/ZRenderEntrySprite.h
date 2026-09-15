#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <Glacier/Render/Entry/ZRenderEntryGeomCreateInfo.h>
#include <Glacier/Render/Prim/SPrimSpritesArray.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderEntrySprite : public ZRenderEntry
    {
    public:
        // vtbl
        ~ZRenderEntrySprite() override;
        RENDERENTRY_BASETYPE GetType() const override;
        void Notify(const SRenderEntryNotifyInfo* pEntry) override;
        void SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance) override;

        // methods
        ZRenderEntrySprite(ZRenderEntryGeomCreateInfo* pInfo);
        void SetLocal(bool bLocal);
        void SetSortValue(int32_t lSortValue);

        // members
        const SSpriteArray* m_pSpriteArray;
        bool m_bLocal;
        uint32_t m_lPrim;
    };

    RE_VERIFY_OFFSET(ZRenderEntrySprite, m_bLocal, 0x84); // Verified by ZDrawBufferSimple::DrawSpriteArray
    RE_VERIFY_OFFSET(ZRenderEntrySprite, m_lPrim, 0x88); // Verified by ZRenderEntrySprite::ZRenderEntrySprite
}