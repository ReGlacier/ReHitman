#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/ZRenderBinderSprite.h>


namespace Glacier
{
    class ZRenderMaterialBinderSpriteD3D : public ZRenderBinderSprite
    {
    public:
        // members
        uint32_t m_dwFlags{0}; //+0xC
        void* m_pTextureDiffuse{nullptr}; //+0x10
        void* m_pTextureMask{nullptr}; //+0x14
        float m_fSpriteBoxFar{0.0f}; //+0x18
        uint32_t m_dwSpriteBoxColor{0}; //+0x1C
        float m_aSpriteBoxPositions[0x20]{0}; //+0x20
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderSpriteD3D, 0xA0); // Verified PC (ZRenderMaterialBinderSpriteD3DFX::m_pEffect at +0xA0)
}