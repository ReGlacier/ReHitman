#include <Glacier/Render/Material/ZRenderMaterialBinderTextureD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZTextureD3D.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        struct STextureModeMapEntry
        {
            const char* pszName;
            uint32_t    lMode;
        };

        // PC 00494630
        int GetTextureAddressMode(const char* pszName)
        {
            static const STextureModeMapEntry s_aTable[3] =
            {
                { "None",     D3DTADDRESS_CLAMP  },
                { "Mirrored", D3DTADDRESS_MIRROR },
                { "Tiled",    D3DTADDRESS_WRAP   },
            };

            if (!pszName)
                return D3DTADDRESS_MIRROR;

            for (int lIndex = 0; lIndex < 3; ++lIndex)
            {
                if (!stricmp(s_aTable[lIndex].pszName, pszName))
                    return s_aTable[lIndex].lMode;
            }

            return D3DTADDRESS_CLAMP;
        }

        // PC 004945E0
        int GetTextureFilterMode(const char* pszFilterModeName)
        {
            static const STextureModeMapEntry s_aTable[3] =
            {
                { "Point",       D3DTEXF_POINT       },
                { "Bilinear",    D3DTEXF_LINEAR      },
                { "Anisotropic", D3DTEXF_ANISOTROPIC },
            };

            if (!pszFilterModeName)
                return D3DTEXF_LINEAR;

            for (int lIndex = 0; lIndex < 3; ++lIndex)
            {
                if (!stricmp(s_aTable[lIndex].pszName, pszFilterModeName))
                    return s_aTable[lIndex].lMode;
            }

            return D3DTEXF_POINT;
        }
    }

    ZRenderMaterialBinderTextureD3DFX::ZRenderMaterialBinderTextureD3DFX(
        const char* pszName,
        ZTextureD3D* pTexture,
        ZRenderMaterialEffectD3DFX *pEffect,
        D3DXHANDLE hTexture,
        const char* pszMinFilter,
        const char* pszMagFilter,
        const char* pszMipFilter,
        const char* pszTilingU,
        const char* pszTilingV,
        const char* pszTilingW,
        D3DXHANDLE hTechnique
    )
    {
        m_pszName = pszName;
        m_lBinderType = 2;

        m_pTexture = pTexture;
        m_pD3DTexture = nullptr;
        m_lAddressU = D3DTADDRESS_WRAP;
        m_lAddressV = D3DTADDRESS_WRAP;
        m_lAddressW = D3DTADDRESS_WRAP;
        m_lMinFilter = D3DTEXF_LINEAR;
        m_lMagFilter = D3DTEXF_LINEAR;
        m_lMipFilter = D3DTEXF_LINEAR;
        m_lMaxAnisotropy = 1;
        m_hTexture = hTexture;
        m_pEffect = pEffect;

        // Sampler state parameters are named "gm_<name><State>"
        char szParameterName[64];
        szParameterName[0] = '\0';
        if (strncmp(pszName, "gm_", 3))
        {
            strcpy(szParameterName, "gm_");
        }
        strcat(szParameterName, pszName);
        char* pszSuffix = szParameterName + strlen(szParameterName);

        ID3DXEffect* pD3DXEffect = pEffect->m_pD3DXEffect;

        strcpy(pszSuffix, "AddressU");
        m_hAddressU = pD3DXEffect->GetParameterByName(nullptr, szParameterName);
        if (m_hAddressU && !pD3DXEffect->IsParameterUsed(m_hAddressU, hTechnique))
        {
            m_hAddressU = nullptr;
        }
        if (m_hAddressU && pszTilingU)
        {
            m_lAddressU = GetTextureAddressMode(pszTilingU);
        }

        strcpy(pszSuffix, "AddressV");
        m_hAddressV = pD3DXEffect->GetParameterByName(nullptr, szParameterName);
        if (m_hAddressV && !pD3DXEffect->IsParameterUsed(m_hAddressV, hTechnique))
        {
            m_hAddressV = nullptr;
        }
        if (m_hAddressV && pszTilingV)
        {
            m_lAddressV = GetTextureAddressMode(pszTilingV);
        }

        strcpy(pszSuffix, "AddressW");
        m_hAddressW = pD3DXEffect->GetParameterByName(nullptr, szParameterName);
        if (m_hAddressW && !pD3DXEffect->IsParameterUsed(m_hAddressW, hTechnique))
        {
            m_hAddressW = nullptr;
        }
        if (m_hAddressW && pszTilingW)
        {
            m_lAddressW = GetTextureAddressMode(pszTilingW);
        }

        strcpy(pszSuffix, "MinFilter");
        m_hMinFilter = pD3DXEffect->GetParameterByName(nullptr, szParameterName);
        if (m_hMinFilter && !pD3DXEffect->IsParameterUsed(m_hMinFilter, hTechnique))
        {
            m_hMinFilter = nullptr;
        }
        if (m_hMinFilter && pszMinFilter)
        {
            m_lMinFilter = GetTextureFilterMode(pszMinFilter);
        }

        strcpy(pszSuffix, "MagFilter");
        m_hMagFilter = pD3DXEffect->GetParameterByName(nullptr, szParameterName);
        if (m_hMagFilter && !pD3DXEffect->IsParameterUsed(m_hMagFilter, hTechnique))
        {
            m_hMagFilter = nullptr;
        }
        if (m_hMagFilter && pszMagFilter)
        {
            m_lMagFilter = GetTextureFilterMode(pszMagFilter);
        }

        strcpy(pszSuffix, "MipFilter");
        m_hMipFilter = pD3DXEffect->GetParameterByName(nullptr, szParameterName);
        if (m_hMipFilter && !pD3DXEffect->IsParameterUsed(m_hMipFilter, hTechnique))
        {
            m_hMipFilter = nullptr;
        }
        if (m_hMipFilter && pszMipFilter)
        {
            m_lMipFilter = GetTextureFilterMode(pszMipFilter);
        }

        strcpy(pszSuffix, "MaxAnisotropy");
        D3DXHANDLE hMaxAnisotropy = pD3DXEffect->GetParameterByName(nullptr, szParameterName);
        if (hMaxAnisotropy && pD3DXEffect->IsParameterUsed(hMaxAnisotropy, hTechnique))
        {
            m_hMaxAnisotropy = hMaxAnisotropy;
        }
        else
        {
            m_hMaxAnisotropy = nullptr;
        }
    }

    ZRenderMaterialBinderTextureD3DFX::~ZRenderMaterialBinderTextureD3DFX() = default;

    void ZRenderMaterialBinderTextureD3DFX::Execute(const ZRenderContext* pContext)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;
        ZTextureD3D* pTexture = m_pTexture;

        // Animated texture: pick the frame by game time (25 frames per second)
        const uint32_t* pAnimData = pTexture->m_pAnimData;
        if (pAnimData)
        {
            const uint32_t lFrame = static_cast<uint32_t>(std::fmod(
                static_cast<double>(g_pSysInterface->FrameTime.secs) * (25.0 / 1024.0),
                static_cast<double>(pAnimData[0] - 1)));
            ZASSERT(lFrame < pAnimData[0]);
            pTexture = g_pRenderDll->m_pTexCon->GetTexture(pAnimData[lFrame + 1], 0u);
        }

        IDirect3DBaseTexture9* pD3DTexture = static_cast<IDirect3DBaseTexture9*>(pTexture->m_pUserData);
        if (m_pD3DTexture)
        {
            pD3DTexture = m_pD3DTexture;
        }

        if (m_hTexture)
        {
            pD3DXEffect->SetTexture(m_hTexture, pD3DTexture);
        }

        if (m_hMinFilter)
        {
            uint32_t lMinFilter = m_lMinFilter;
            if (g_lMaxAnisotropy > 1)
            {
                lMinFilter = D3DTEXF_ANISOTROPIC;
            }
            pD3DXEffect->SetInt(m_hMinFilter, lMinFilter);
        }

        if (m_hMagFilter)
        {
            uint32_t lMagFilter = m_lMagFilter;
            if (g_lMaxAnisotropy > 1)
            {
                lMagFilter = D3DTEXF_LINEAR;
            }
            pD3DXEffect->SetInt(m_hMagFilter, lMagFilter);
        }

        if (m_hMipFilter)
        {
            uint32_t lMipFilter = m_lMipFilter;
            if (g_lMaxAnisotropy > 1)
            {
                lMipFilter = D3DTEXF_LINEAR;
            }
            pD3DXEffect->SetInt(m_hMipFilter, lMipFilter);
        }

        if (m_hAddressU)
        {
            pD3DXEffect->SetInt(m_hAddressU, m_lAddressU);
        }

        if (m_hAddressV)
        {
            pD3DXEffect->SetInt(m_hAddressV, m_lAddressV);
        }

        if (m_hAddressW)
        {
            pD3DXEffect->SetInt(m_hAddressW, m_lAddressW);
        }

        if (m_hMaxAnisotropy)
        {
            pD3DXEffect->SetInt(m_hMaxAnisotropy, g_lMaxAnisotropy);
        }
    }

    void ZRenderMaterialBinderTextureD3DFX::PrintInfo()
    {
    }

    uint32_t ZRenderMaterialBinderTextureD3DFX::GetTextureId()
    {
        return m_pTexture->m_lTextureId;
    }
}
