#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderSpriteD3D.h>


namespace Glacier
{
    class ZRenderMaterialBinderSpriteD3DFX : public ZRenderMaterialBinderSpriteD3D
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderSpriteD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;

        // methods
        ZRenderMaterialBinderSpriteD3DFX(ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hTechnique);

        // members
        ZRenderMaterialEffectD3DFX*         m_pEffect;                // +0xA0
        ZRenderMaterialBinderTextureD3DFX*  m_pBinderMapDiffuse;      // +0xA4
        ZRenderMaterialBinderTextureD3DFX*  m_pBinderMapDiffuseMask;  // +0xA8
        D3DXHANDLE                          m_hGiAlphaBlendEnabled;   // +0xAC
        D3DXHANDLE                          m_hGiBlendSrc;            // +0xB0
        D3DXHANDLE                          m_hGiBlendDst;            // +0xB4
        D3DXHANDLE                          m_hGiAlphaTestEnabled;    // +0xB8
        D3DXHANDLE                          m_hGiAlphaTestValue;      // +0xBC
        D3DXHANDLE                          m_hGiCulling;             // +0xC0
        D3DXHANDLE                          m_hGiFogEnabled;          // +0xC4
        D3DXHANDLE                          m_hvFogColor;             // +0xC8
        D3DXHANDLE                          m_hvFogSettings;          // +0xCC
        D3DXHANDLE                          m_hGiIWriteZBuffer;       // +0xD0
        D3DXHANDLE                          m_hGiIZBufferFunc;        // +0xD4
        D3DXHANDLE                          m_hGiVSpriteBoxFar;       // +0xD8
        D3DXHANDLE                          m_hGiVSpriteBoxColor;     // +0xDC
        D3DXHANDLE                          m_hGiVSpriteBoxPositions; // +0xE0
        D3DXHANDLE                          m_hGiVSpriteBoxAspect;    // +0xE4
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderSpriteD3DFX, 0xE8); // Verified PC, must fit

    RE_VERIFY_OFFSET(ZRenderMaterialBinderSpriteD3DFX, m_hGiAlphaBlendEnabled, 0xAC); // Verified ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderSpriteD3DFX, m_hGiBlendSrc, 0xB0); // Verified ctor

}