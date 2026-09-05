#include <Glacier/Render/Material/ZRenderMaterialBinderTextureD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderFloatD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderIntD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderCullD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderScrollD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderSpriteD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderRenderStateD3DFX.h>
#include <Glacier/Render/Material/ZRenderBinderContextD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderParserD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderParser.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRenderBinderList.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderMaterialBinderParserD3DFX::ZRenderMaterialBinderParserD3DFX(ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hTechnique)
        : ZRenderMaterialBinderParserD3D()
        , m_pEffect(pEffect)
        , m_hTechnique(hTechnique)
    {
    }

    ZRenderMaterialBinderParserD3DFX::~ZRenderMaterialBinderParserD3DFX() = default;

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderTexture(
            ZRenderBinderList* pBinderList,
            const char* pszBinderName,
            uint32_t lTextureId,
            const char* pszMinFilter,
            const char* pszMagFilter,
            const char* pszMipFilter,
            const char* pszTilingU,
            const char* pszTilingV,
            const char* pszTilingW)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;

        D3DXHANDLE hParam = pD3DXEffect->GetParameterByName(nullptr, pszBinderName);

        if (hParam != nullptr && pD3DXEffect->IsParameterUsed(hParam, m_hTechnique))
        {
            ZTextureD3D* pTexture = g_pRenderDll->m_pTexCon->GetTexture(lTextureId, 0);

            ZRenderMaterialBinderTextureD3DFX* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
                pszBinderName,
                pTexture,
                m_pEffect,
                hParam,
                pszMinFilter,
                pszMagFilter,
                pszMipFilter,
                pszTilingU,
                pszTilingV,
                pszTilingW,
                m_hTechnique);

            pBinderList->m_pBinders[pBinderList->m_lNumBinders++] = pNewBinder;
        }
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderFloat(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pFloats, uint32_t lNumFloats)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;

        D3DXHANDLE hParam = pD3DXEffect->GetParameterByName(nullptr, pszBinderName);

        if (hParam != nullptr && pD3DXEffect->IsParameterUsed(hParam, m_hTechnique))
        {
            D3DXPARAMETER_DESC sParamDesc;
            pD3DXEffect->GetParameterDesc(hParam, &sParamDesc);

            if (sParamDesc.Type == D3DXPT_INT)
            {
                ZASSERT(lNumFloats < 4);

                uint32_t aInts[4];
                for (uint32_t i = 0; i < lNumFloats; ++i)
                {
                    aInts[i] = static_cast<uint32_t>(pFloats[i]);
                }

                ZRenderBinder* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderIntD3DFX>(
                    pszBinderName, aInts, lNumFloats, m_pEffect, hParam);
                if (pNewBinder)
                {
                    pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
                }
            }
            else if (sParamDesc.Type == D3DXPT_FLOAT)
            {
                ZRenderBinder* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderFloatD3DFX>(
                    pszBinderName, pFloats, lNumFloats, m_pEffect, hParam);
                if (pNewBinder)
                {
                    pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
                }
            }
        }
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderBool(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const uint32_t* pBools, uint32_t lNumBools)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;
        if (!pD3DXEffect)
            return;

        D3DXHANDLE hParam = pD3DXEffect->GetParameterByName(nullptr, pszBinderName);

        if (hParam != nullptr && pD3DXEffect->IsParameterUsed(hParam, m_hTechnique))
        {
            ZRenderBinder* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderIntD3DFX>(
                pszBinderName, const_cast<uint32_t*>(pBools), lNumBools, m_pEffect, hParam);
            pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
        }
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderEnum(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const char* pszEnum)
    {
        if (stricmp(pszBinderName, "Culling") != 0) return;

        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;
        if (!pD3DXEffect)
            return;

        D3DXHANDLE hParam = pD3DXEffect->GetParameterByName(nullptr, "Culling");

        if (hParam != nullptr && pD3DXEffect->IsParameterUsed(hParam, m_hTechnique))
        {
            D3DCULL nCullMode = D3DCULL_CW;

            if (_stricmp(pszEnum, "TwoSided") == 0)
            {
                nCullMode = D3DCULL_NONE;
            }

            ZRenderBinder* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderCullD3DFX>(
                pszBinderName, nCullMode, m_pEffect, hParam);
            pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
        }
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderColor(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pColor)
    {
        CreatePropertyBinderFloat(pMaterialBinderList, pszBinderName, pColor, 4);
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderContext(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, uint32_t lContextType)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;
        if (!pD3DXEffect)
            return;

        D3DXHANDLE hParam = pD3DXEffect->GetParameterByName(nullptr, pszBinderName);

        if (hParam != nullptr && pD3DXEffect->IsParameterUsed(hParam, m_hTechnique))
        {
            ZRenderBinder* pNewBinder = nullptr;

            switch (static_cast<RT_NAME>(lContextType))
            {
                case RT_NAME::RTN_MAP_CUBE_NORMALIZER: // 0x19 (25)
                    pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
                        pszBinderName, &g_texShadowMapDepth, m_pEffect, hParam,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique);
                    break;

                case RT_NAME::RTN_MAP_SCATTER: // 0x1A (26)
                    pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
                        pszBinderName, &g_texNormalizer, m_pEffect, hParam,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique);
                    break;

                case RT_NAME::RTN_MAP_PHASE: // 0x1B (27)
                    pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
                        pszBinderName, &g_texShadowCubeMapColor, m_pEffect, hParam,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique);
                    break;

                case RT_NAME::RTN_MAP_REFRACTION: // 0x1C (28)
                    pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
                        pszBinderName, &g_texRefractionMap, m_pEffect, hParam,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique);
                    break;

                case RT_NAME::RTN_MAP_CUBE_SHADOW_COLOR: // 0x20 (32)
                    pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
                        pszBinderName, &g_texCubeShadowColor, m_pEffect, hParam,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique);
                    break;

                case RT_NAME::RTN_MAP_SHADOW_DEPTH: // 0x21 (33)
                    pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
                        pszBinderName, &g_texShadowDepth, m_pEffect, hParam,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique);
                    break;

                default:
                    pNewBinder = ZUniMemory::New<ZRenderBinderContextD3DFX>(
                        pszBinderName,
                        lContextType,
                        m_pEffect,
                        hParam);
                    break;
            }

            pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
        }
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderSprite(ZRenderBinderList* pMaterialBinderList)
    {
        ZRenderBinder* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderSpriteD3DFX>(
            m_pEffect, m_hTechnique);
        pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderRenderState(
        ZRenderBinderList* pMaterialBinderList,
        bool bBlendEnabled,
        const char* pszBlendMode,
        float fOpacity,
        bool bAlphaTestEnabled,
        uint32_t lAlphaTestRef,
        bool bFogEnabled,
        const char* pszCullMode,
        uint32_t lZBias,
        float fZOffset)
    {
        ZRenderBinder* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderRenderStateD3DFX>(
            bBlendEnabled,
            pszBlendMode,
            fOpacity,
            bAlphaTestEnabled,
            lAlphaTestRef,
            bFogEnabled,
            pszCullMode,
            lZBias,
            fZOffset,
            m_pEffect,
            m_hTechnique);
        pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderScroll(ZRenderBinderList* pMaterialBinderList, const float* pfScrollSpeed)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;

        D3DXHANDLE hScrollPosition = pD3DXEffect->GetParameterByName(nullptr, "gm_vScrollPosition");

        if (hScrollPosition != nullptr && pD3DXEffect->IsParameterUsed(hScrollPosition, m_hTechnique))
        {
            ZRenderBinder* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderScrollD3DFX>(
                m_pEffect, hScrollPosition, pfScrollSpeed);
            pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
        }
    }
}
