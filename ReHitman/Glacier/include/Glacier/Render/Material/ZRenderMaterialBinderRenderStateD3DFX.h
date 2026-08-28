#pragma once

#include <Glacier/Render/ZRenderBinder.h>
#include <Glacier/Render/D3D9.h>


namespace Glacier
{
    class ZRenderMaterialBinderRenderStateD3DFX : public ZRenderBinder
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderRenderStateD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;

        // methods
        ZRenderMaterialBinderRenderStateD3DFX(
            bool bBlendEnabled,
            const char* pszBlendMode,
            float fOpacity,
            bool bAlphaTestEnabled,
            uint32_t lAlphaTestRef,
            bool bFogEnabled,
            const char* pszCullMode,
            uint32_t lZBias,
            float fZOffset,
            ZRenderMaterialEffectD3DFX* pEffect,
            D3DXHANDLE hTechnique
        );

        // members
        const char*                  m_pszBlendMode;        // +0x0C
        uint32_t                     m_bBlendEnabled;       // +0x10
        D3DBLEND                     m_BlendSrc;            // +0x14 (per-blend-mode, see ctor "TRANS"/"ADD"/"SHADOW" branches)
        D3DBLEND                     m_BlendDst;            // +0x18
        uint32_t                     m_bWriteZBuffer;       // +0x1C (forwarded to gi_iWriteZBuffer in Execute)
        float                        m_fOpacity;            // +0x20
        uint32_t                     m_bAlphaTestEnabled;   // +0x24
        uint32_t                     m_lAlphaTestRef;       // +0x28
        D3DCULL                      m_eCullMode;           // +0x2C (2 = D3DCULL_CW; 1 = D3DCULL_NONE for "TwoSided")
        uint32_t                     m_bFogEnabled;         // +0x30
        uint32_t                     m_bFogColorBlack;      // +0x34 (1 forces black fog color in Execute)
        float                        m_fZBias;              // +0x38 (1.0 - fZOffset * (1.0f/1275.0f))
        ZRenderMaterialEffectD3DFX*  m_pEffect;             // +0x3C
        D3DXHANDLE                   m_hOpacity;            // +0x40 "v4Opacity"
        D3DXHANDLE                   m_hBlendSrc;           // +0x44 "BlendSrc"
        D3DXHANDLE                   m_hBlendDst;           // +0x48 "BlendDst"
        D3DXHANDLE                   m_hWriteZBuffer;       // +0x4C "gi_iWriteZBuffer"
        D3DXHANDLE                   m_hAlphaTestEnabled;   // +0x50 "AlphaTestEnabled"
        D3DXHANDLE                   m_hAlphaTestRef;       // +0x54 "AlphaTestValue"
        D3DXHANDLE                   m_hCullMode;           // +0x58 "Culling"
        D3DXHANDLE                   m_hZBiasOffset;        // +0x5C "gm_vZBiasOffset"
        D3DXHANDLE                   m_hFogEnabled;         // +0x60 "FogEnabled"
        D3DXHANDLE                   m_hFogSettings;        // +0x64 "vFogSettings"
        D3DXHANDLE                   m_hFogColor;           // +0x68 "vFogColor"
        D3DXHANDLE                   m_hTechnique;          // +0x6C
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderRenderStateD3DFX, 0x70); // Verified PC alloc

    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_pszBlendMode, 0x0C); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_BlendSrc, 0x14); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_BlendDst, 0x18); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_bWriteZBuffer, 0x1C); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_bFogColorBlack, 0x34); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_fZBias, 0x38); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_pEffect, 0x3C); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hOpacity, 0x40); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hBlendSrc, 0x44); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hBlendDst, 0x48); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hWriteZBuffer, 0x4C); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hAlphaTestEnabled, 0x50); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hAlphaTestRef, 0x54); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hCullMode, 0x58); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hZBiasOffset, 0x5C); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hFogEnabled, 0x60); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hFogSettings, 0x64); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hFogColor, 0x68); // Verified PC ctor
    RE_VERIFY_OFFSET(ZRenderMaterialBinderRenderStateD3DFX, m_hTechnique, 0x6C); // Verified PC ctor
}
