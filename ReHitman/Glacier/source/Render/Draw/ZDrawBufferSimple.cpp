#include <Glacier/Render/Entry/ZRenderEntrySprite.h>
#include <Glacier/Render/Draw/ZDrawBufferSimple.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    ZDrawBufferSimple::ZDrawBufferSimple(ZRenderEntry** ppEntries, uint32_t lSizeX, uint32_t lSizeY)
        : m_lNumRenderEntries(0)
        , m_pRenderEntries(ppEntries)
        , m_lSizeX(lSizeX)
        , m_lSizeY(lSizeY)
    {
    }

    int ZDrawBufferSimple::GetSizeX() const
    {
        return m_lSizeX;
    }

    int ZDrawBufferSimple::GetSizeY() const
    {
        return m_lSizeY;
    }

    float ZDrawBufferSimple::ViewAspectXY() const
    {
        const auto fViewX = static_cast<float>(g_pSysInterface->WindowFirst->GetSizeX());
        const auto fViewY = static_cast<float>(g_pSysInterface->WindowFirst->GetSizeY());
        const float fBufferX = static_cast<float>(GetSizeX());
        const float fBufferY = static_cast<float>(GetSizeY());

        return (fBufferY / fViewY) / (fBufferX / fViewX);
    }

    ZRenderEntry* ZDrawBufferSimple::DrawBonesArray(uint32_t lPrim, const SBonesArray* pBonesArray)
    {
        auto* pDrawEntry = IDraw::Instance<ZRenderDraw>()->AddRenderEntryArray(lPrim, reinterpret_cast<const SDrawArray*>(pBonesArray));
        if (pDrawEntry)
        {
            m_pRenderEntries[m_lNumRenderEntries++] = pDrawEntry;
        }

        return pDrawEntry;
    }
    
    ZRenderEntry* ZDrawBufferSimple::DrawSpriteArray(uint32_t lPrim, const SSpriteArray* pSpriteArray, const float *m0, const float *p0, unsigned __int8 lPriority, bool bLocal, unsigned int lDrawMask)
    {
        auto* pDrawEntry = IDraw::Instance<ZRenderDraw>()->AddRenderEntrySprite(lPrim);
        if (!pDrawEntry)
        {
            return nullptr;
        }

        ZMatrix mMatrix
        {
            .m0 = m0,
            .p0 = p0
        };

        pDrawEntry->SetLocal(bLocal);
        pDrawEntry->SetObjectToWorldMatrix(mMatrix);
        pDrawEntry->m_pSpriteArray = pSpriteArray;
        
        pDrawEntry->SetSortValue(lPriority);
        m_pRenderEntries[m_lNumRenderEntries++] = pDrawEntry;
        return pDrawEntry;
    }
}