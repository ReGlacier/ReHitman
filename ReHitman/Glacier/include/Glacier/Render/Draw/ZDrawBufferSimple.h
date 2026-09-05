#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Draw/ZDrawBuffer.h>


namespace Glacier
{
    class ZDrawBufferSimple : public ZDrawBuffer
    {
    public:
        // vtbl
        virtual int GetSizeX() const override;
        virtual int GetSizeY() const override;
        virtual float ViewAspectXY() const override;
        virtual ZRenderEntry* DrawBonesArray(uint32_t lPrim, const SBonesArray* pBonesArray) override;
        virtual ZRenderEntry* DrawSpriteArray(uint32_t lPrim, const SSpriteArray* pSpriteArray, const float *m0, const float *p0, unsigned __int8 lPriority, bool bLocal, unsigned int lDrawMask) override;

        // methods
        ZDrawBufferSimple(ZRenderEntry** ppEntries, uint32_t lSizeX, uint32_t lSizeY);
        
        // members
        uint32_t m_lNumRenderEntries;
        ZRenderEntry** m_pRenderEntries;
        uint32_t m_lSizeX;
        uint32_t m_lSizeY;
    };
    RE_VERIFY_SIZE(ZDrawBufferSimple, 0x14);
}