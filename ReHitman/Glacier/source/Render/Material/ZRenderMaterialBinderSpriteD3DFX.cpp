#include <Glacier/Render/Material/ZRenderMaterialBinderTextureD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderSpriteD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    namespace 
    {
        inline D3DXVECTOR4 ColorToVector4(uint32_t dwColor)
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

    ZRenderMaterialBinderSpriteD3DFX::ZRenderMaterialBinderSpriteD3DFX(ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hTechnique)
        : ZRenderMaterialBinderSpriteD3D()
    {
        // ZRenderBinder
        m_pszName = "BinderSprite";
        m_lBinderType = 4;

        // ZRenderMaterialBinderSpriteD3D
        m_dwFlags = 0;
        m_pTextureDiffuse = nullptr;
        m_pTextureMask = nullptr;
        m_fSpriteBoxFar = 0.0f;
        m_dwSpriteBoxColor = 0;

        // Effect
        m_pEffect = pEffect;
        ID3DXEffect* pD3DXEffect = pEffect->m_pD3DXEffect;

        // mapDiffuse binder
        D3DXHANDLE hMapDiffuse = pD3DXEffect->GetParameterByName(nullptr, "mapDiffuse");
        if (hMapDiffuse && !pD3DXEffect->IsParameterUsed(hMapDiffuse, hTechnique))
        {
            hMapDiffuse = nullptr;
        }

        // map diffuse
        m_pBinderMapDiffuse = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            "mapDiffuse", &g_texWhite, m_pEffect, hMapDiffuse, 
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, hTechnique
        );

        // mapDiffuseMask binder
        D3DXHANDLE hMapDiffuseMask = pD3DXEffect->GetParameterByName(nullptr, "mapDiffuseMask");
        if (hMapDiffuseMask && !pD3DXEffect->IsParameterUsed(hMapDiffuseMask, hTechnique))
        {
            hMapDiffuseMask = nullptr;
        }

        // mapDiffuseMask binder
        m_pBinderMapDiffuseMask = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            "mapDiffuseMask", &g_texWhite, pEffect, hMapDiffuseMask,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, hTechnique
        );

        // Effect parameters
        m_hGiAlphaBlendEnabled   = pD3DXEffect->GetParameterByName(nullptr, "gi_AlphaBlendEnabled");
        m_hGiBlendSrc            = pD3DXEffect->GetParameterByName(nullptr, "gi_BlendSrc");
        m_hGiBlendDst            = pD3DXEffect->GetParameterByName(nullptr, "gi_BlendDst");
        m_hGiAlphaTestEnabled    = pD3DXEffect->GetParameterByName(nullptr, "gi_AlphaTestEnabled");
        m_hGiAlphaTestValue      = pD3DXEffect->GetParameterByName(nullptr, "gi_AlphaTestValue");
        m_hGiCulling             = pD3DXEffect->GetParameterByName(nullptr, "Culling");
        m_hGiFogEnabled          = pD3DXEffect->GetParameterByName(nullptr, "FogEnabled");
        m_hvFogColor             = pD3DXEffect->GetParameterByName(nullptr, "vFogColor");
        m_hvFogSettings          = pD3DXEffect->GetParameterByName(nullptr, "vFogSettings");
        m_hGiIWriteZBuffer       = pD3DXEffect->GetParameterByName(nullptr, "gi_iWriteZBuffer");
        m_hGiIZBufferFunc        = pD3DXEffect->GetParameterByName(nullptr, "gi_iZBufferFunc");
        m_hGiVSpriteBoxFar       = pD3DXEffect->GetParameterByName(nullptr, "gi_vSpriteBoxFar");
        m_hGiVSpriteBoxColor     = pD3DXEffect->GetParameterByName(nullptr, "gi_vSpriteBoxColor");
        m_hGiVSpriteBoxPositions = pD3DXEffect->GetParameterByName(nullptr, "gi_vSpriteBoxPositions");
        m_hGiVSpriteBoxAspect    = pD3DXEffect->GetParameterByName(nullptr, "gi_vSpriteBoxAspect");
    }

    ZRenderMaterialBinderSpriteD3DFX::~ZRenderMaterialBinderSpriteD3DFX()
    {
        if (m_pBinderMapDiffuse)
        {
            ZUniMemory::Delete(m_pBinderMapDiffuse);
            m_pBinderMapDiffuse = nullptr;
        }
        
        if (m_pBinderMapDiffuseMask)
        {
            ZUniMemory::Delete(m_pBinderMapDiffuse);
            m_pBinderMapDiffuse = nullptr;
        }
    }

    void ZRenderMaterialBinderSpriteD3DFX::Execute(const ZRenderContext* pContext)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;
        const uint32_t dwFlags = m_dwFlags;

        // Alpha Blending
        if (dwFlags & 0x01000000)
        {
            pD3DXEffect->SetBool(m_hGiAlphaBlendEnabled, TRUE);
            pD3DXEffect->SetInt(m_hGiBlendSrc, D3DBLEND_ONE);       // 1
            pD3DXEffect->SetInt(m_hGiBlendDst, D3DBLEND_INVSRCALPHA); // 3
        }
        else if (dwFlags & 0x00026003)
        {
            pD3DXEffect->SetBool(m_hGiAlphaBlendEnabled, TRUE);
            pD3DXEffect->SetInt(m_hGiBlendSrc, D3DBLEND_SRCALPHA); // 5

            if ((dwFlags & 3) == 3)
            {
                pD3DXEffect->SetInt(m_hGiBlendSrc, D3DBLEND_INVSRCALPHA); // 3
                pD3DXEffect->SetInt(m_hGiBlendDst, D3DBLEND_DESTCOLOR);    // 6
            }
            else if (dwFlags & 2)
            {
                pD3DXEffect->SetInt(m_hGiBlendDst, D3DBLEND_ONE);          // 2
            }
            else
            {
                pD3DXEffect->SetInt(m_hGiBlendDst, D3DBLEND_DESTCOLOR);    // 6
            }
        }
        else
        {
            pD3DXEffect->SetBool(m_hGiAlphaBlendEnabled, FALSE);
        }

        // Alpha Test
        if (dwFlags & 0x00000400)
        {
            pD3DXEffect->SetBool(m_hGiAlphaTestEnabled, TRUE);
            pD3DXEffect->SetInt(m_hGiAlphaTestValue, 0x7F);
        }
        else
        {
            pD3DXEffect->SetBool(m_hGiAlphaTestEnabled, FALSE);
        }

        // Culling
        if (m_hGiCulling)
        {
            pD3DXEffect->SetBool(m_hGiCulling, TRUE);
        }

        // Fog
        if (dwFlags & 0x00100000)
        {
            pD3DXEffect->SetBool(m_hGiFogEnabled, FALSE);
        }
        else
        {
            pD3DXEffect->SetBool(m_hGiFogEnabled, TRUE);

            // Fog color
            D3DXVECTOR4 vFogColor;
            if (dwFlags & 2)
            {
                // For additive mode use black fog
                vFogColor = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
            }
            else
            {
                // Use global fog
                vFogColor = ColorToVector4(g_dwFogColor);
            }
            pD3DXEffect->SetVector(m_hvFogColor, &vFogColor);

            // Calculate fog near/far
            const float fFogRange = 1.0f / (g_fFogFar - g_fFogNear);
            const D3DXVECTOR4 vFogSettings(
                g_fFogFar * fFogRange,
                fFogRange,
                0.0f,
                0.0f
            );

            pD3DXEffect->SetVector(m_hvFogSettings, &vFogSettings);
        }

        // Z-Buffer
        if (dwFlags & 0x00040000)
        {
            pD3DXEffect->SetBool(m_hGiIWriteZBuffer, FALSE);
        }
        else
        {
            pD3DXEffect->SetBool(m_hGiIWriteZBuffer, TRUE);
        }

        if (m_hGiIZBufferFunc)
        {
            if (dwFlags & 0x00020000)
                pD3DXEffect->SetInt(m_hGiIZBufferFunc, D3DCMP_ALWAYS);
            else
                pD3DXEffect->SetInt(m_hGiIZBufferFunc, D3DCMP_LESSEQUAL);
        }

        // Child texture binders
        const uint32_t dwBlendMode  = ((dwFlags & 0x8000) | 0x4000) >> 14;
        const uint32_t dwFilterMode = ((dwFlags & 0x4000) | 0x2000) >> 13;

        if (m_pBinderMapDiffuse)
        {
            // PC 004952E0: the sprite binder feeds the raw D3D texture and reuses
            // the AddressU/AddressV value slots as per-frame filter/blend modes
            m_pBinderMapDiffuse->m_pD3DTexture = reinterpret_cast<IDirect3DBaseTexture9*>(m_pTextureDiffuse);
            m_pBinderMapDiffuse->m_lAddressV = dwBlendMode;
            m_pBinderMapDiffuse->m_lAddressU = dwFilterMode;
            m_pBinderMapDiffuse->Execute(pContext);
        }

        if (m_pBinderMapDiffuseMask)
        {
            m_pBinderMapDiffuseMask->m_pD3DTexture = reinterpret_cast<IDirect3DBaseTexture9*>(m_pTextureMask);
            m_pBinderMapDiffuseMask->m_lAddressV = dwBlendMode;
            m_pBinderMapDiffuseMask->m_lAddressU = dwFilterMode;
            m_pBinderMapDiffuseMask->Execute(pContext);
        }

        // SpriteBox Parameters
        if (m_hGiVSpriteBoxFar)
        {
            pD3DXEffect->SetFloat(m_hGiVSpriteBoxFar, m_fSpriteBoxFar);
        }

        // Sprite color override
        if (m_hGiVSpriteBoxColor)
        {
            const D3DXVECTOR4 vSpriteBoxColor = ColorToVector4(m_dwSpriteBoxColor);
            pD3DXEffect->SetFloatArray(m_hGiVSpriteBoxColor, &vSpriteBoxColor.x, 4);
        }

        // Override positions
        if (m_hGiVSpriteBoxPositions)
        {
            pD3DXEffect->SetFloatArray(m_hGiVSpriteBoxPositions, m_aSpriteBoxPositions, 0x20);
        }

        // Update aspect ratio
        if (m_hGiVSpriteBoxAspect)
        {
            const float fHeight = static_cast<float>(pContext->m_pRender->GetSizeY());
            const float fWidth = static_cast<float>(pContext->m_pRender->GetSizeX());
            const float fAspect = fWidth / fHeight;

            const D3DXVECTOR4 vAspect(fAspect, 1.0f, 0.0f, 0.0f);
            pD3DXEffect->SetVector(m_hGiVSpriteBoxAspect, &vAspect);
        }

        // Commit changes to D3D
        pD3DXEffect->CommitChanges();
    }
}