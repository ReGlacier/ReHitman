#include <Glacier/Render/Material/ZRenderMaterialBinderRenderStateD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Globals.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        D3DXHANDLE GetParameterByNameIfUsed(ID3DXEffect* pD3DXEffect, D3DXHANDLE hTechnique, const char* pszName)
        {
            D3DXHANDLE hParameter = pD3DXEffect->GetParameterByName(nullptr, pszName);
            if (hParameter && !pD3DXEffect->IsParameterUsed(hParameter, hTechnique))
            {
                hParameter = nullptr;
            }
            return hParameter;
        }

        D3DXVECTOR4 ColorToVector4(uint32_t dwColor)
        {
            constexpr float fScale = 1.0f / 255.0f;

            return D3DXVECTOR4
            {
                static_cast<float>((dwColor >> 16) & 0xFF) * fScale,
                static_cast<float>((dwColor >> 8)  & 0xFF) * fScale,
                static_cast<float>( dwColor        & 0xFF) * fScale,
                static_cast<float>((dwColor >> 24) & 0xFF) * fScale
            };
        }
    }

    ZRenderMaterialBinderRenderStateD3DFX::ZRenderMaterialBinderRenderStateD3DFX(
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
    )
    {
        // ZRenderBinder
        m_pszName = "RenderState";
        m_lBinderType = 5;

        m_pszBlendMode = pszBlendMode;
        m_bBlendEnabled = bBlendEnabled;
        m_fOpacity = fOpacity;
        m_bAlphaTestEnabled = bAlphaTestEnabled;
        m_lAlphaTestRef = lAlphaTestRef;
        m_eCullMode = D3DCULL_CW;
        m_bFogEnabled = bFogEnabled;
        m_hTechnique = hTechnique;

        int32_t lZOffset = static_cast<int32_t>(lZBias);
        if (lZOffset >= 7)
        {
            lZOffset = 7;
        }

        m_pEffect = pEffect;
        m_fZBias = 1.0f - static_cast<float>(lZOffset) * 0.00078431371f;

        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;

        // Effect parameters, only those actually used by the technique are kept
        m_hAlphaTestEnabled = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "AlphaTestEnabled");
        m_hAlphaTestRef     = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "AlphaTestValue");
        m_hWriteZBuffer     = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "gi_iWriteZBuffer");
        m_hCullMode         = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "Culling");
        m_hFogEnabled       = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "FogEnabled");
        m_hFogSettings      = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "vFogSettings");
        m_hFogColor         = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "vFogColor");
        m_hZBiasOffset      = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "gm_vZBiasOffset");

        if (stricmp(pszCullMode, "TwoSided") == 0)
        {
            m_eCullMode = D3DCULL_NONE;
        }

        m_bFogColorBlack = 0;

        if (!bBlendEnabled)
        {
            m_bWriteZBuffer = 1;
            return;
        }

        m_hBlendSrc = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "BlendSrc");
        m_hBlendDst = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "BlendDst");

        if (strcmp(pszBlendMode, "TRANS") == 0)
        {
            m_BlendDst = D3DBLEND_INVSRCALPHA;
            m_bWriteZBuffer = 1;
            m_BlendSrc = D3DBLEND_SRCALPHA;
        }
        else if (strcmp(pszBlendMode, "TRANS_ON_OPAQUE") == 0)
        {
            m_BlendDst = D3DBLEND_INVSRCALPHA;
            m_bWriteZBuffer = 0;
            m_BlendSrc = D3DBLEND_SRCALPHA;
        }
        else if (strcmp(pszBlendMode, "TRANSADD_ON_OPAQUE") == 0)
        {
            m_BlendSrc = D3DBLEND_ONE;
            m_BlendDst = D3DBLEND_INVSRCALPHA;
            m_bWriteZBuffer = 0;
            m_bFogColorBlack = 1;
        }
        else if (strcmp(pszBlendMode, "ADD") == 0
            || strcmp(pszBlendMode, "ADD_BEFORE_TRANS") == 0
            || strcmp(pszBlendMode, "ADD_ON_OPAQUE") == 0)
        {
            m_BlendDst = D3DBLEND_ONE;
            m_bWriteZBuffer = 0;
            m_bFogColorBlack = 1;
            m_BlendSrc = D3DBLEND_SRCALPHA;
        }
        else if (strcmp(pszBlendMode, "SHADOW") == 0)
        {
            m_BlendSrc = D3DBLEND_ZERO;
            m_BlendDst = D3DBLEND_INVSRCCOLOR;
            m_bWriteZBuffer = 0;
            m_bFogColorBlack = 1;
        }
        else if (strcmp(pszBlendMode, "STATICSHADOW") == 0)
        {
            m_BlendDst = D3DBLEND_INVSRCALPHA;
            m_bWriteZBuffer = 0;
            m_BlendSrc = D3DBLEND_SRCALPHA;
        }

        m_hOpacity = GetParameterByNameIfUsed(pD3DXEffect, hTechnique, "v4Opacity");
    }

    ZRenderMaterialBinderRenderStateD3DFX::~ZRenderMaterialBinderRenderStateD3DFX() = default;

    void ZRenderMaterialBinderRenderStateD3DFX::Execute(const ZRenderContext* pContext)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;

        // Z-Buffer
        if (m_hWriteZBuffer)
        {
            pD3DXEffect->SetInt(m_hWriteZBuffer, m_bWriteZBuffer);
        }

        // Culling
        if (m_hCullMode)
        {
            D3DCULL dwCull = m_eCullMode;

            // TODO: Finish this place after ZRender will be reversed
            // Expected decompiled call:
            //   if (dwCull == D3DCULL_CW && *reinterpret_cast<const uint8_t*>(reinterpret_cast<const char*>(pContext->m_pRender) + 0x133D))
            //       dwCull = D3DCULL_CCW;

            pD3DXEffect->SetInt(m_hCullMode, dwCull);
        }

        // Fog
        if (m_hFogEnabled)
        {
            pD3DXEffect->SetInt(m_hFogEnabled, m_bFogEnabled);
        }

        if (m_hFogColor)
        {
            const D3DXVECTOR4 vFogColor = m_bFogColorBlack ? D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f) : ColorToVector4(g_dwFogColor);
            pD3DXEffect->SetFloatArray(m_hFogColor, &vFogColor.x, 4);
        }

        if (m_hFogSettings)
        {
            const float fFogRange = 1.0f / (g_fFogFar - g_fFogNear);
            const D3DXVECTOR4 vFogSettings(g_fFogFar * fFogRange, fFogRange, 0.0f, 0.0f);
            pD3DXEffect->SetFloatArray(m_hFogSettings, &vFogSettings.x, 4);
        }

        // Blending
        if (m_bBlendEnabled)
        {
            if (m_hBlendSrc)
            {
                pD3DXEffect->SetInt(m_hBlendSrc, m_BlendSrc);
            }

            if (m_hBlendDst)
            {
                pD3DXEffect->SetInt(m_hBlendDst, m_BlendDst);
            }

            if (m_hOpacity)
            {
                const float aOpacity[4] = { m_fOpacity, m_fOpacity, m_fOpacity, m_fOpacity };
                pD3DXEffect->SetFloatArray(m_hOpacity, aOpacity, 4);
            }
        }

        // Alpha test
        if (m_hAlphaTestEnabled)
        {
            pD3DXEffect->SetInt(m_hAlphaTestEnabled, m_bAlphaTestEnabled);
        }

        if (m_hAlphaTestRef)
        {
            pD3DXEffect->SetInt(m_hAlphaTestRef, m_lAlphaTestRef);
        }

        // Z-Bias
        // PC forwards the stored m_pEffect value as the 2nd ZBias component
        // (XBOX stores m_fZOffset there instead); preserved for parity.
        const float aZBias[2] = { m_fZBias, *reinterpret_cast<const float*>(&m_pEffect) };
        if (m_hZBiasOffset)
        {
            pD3DXEffect->SetFloatArray(m_hZBiasOffset, aZBias, 2);
        }
        const_cast<ZRenderContext*>(pContext)->m_fZBias = m_fZBias;
    }
}
