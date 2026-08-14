#include <Glacier/Render/Material/ZRenderMaterialBinderParserD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderParser.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/ZRenderBinderList.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


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
        // TODO: Finish me
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderFloat(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pFloats, uint32_t lNumFloats)
    {
        // TODO: Finish me
    }

    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderBool(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const uint32_t* pBools, uint32_t lNumBools)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;
        if (!pD3DXEffect)
            return;

        D3DXHANDLE hParam = pD3DXEffect->GetParameterByName(nullptr, pszBinderName);

        if (hParam != nullptr && pD3DXEffect->IsParameterUsed(hParam, m_hTechnique))
        {
            // TODO: Finish me
            // auto* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderIntD3DFX>(lNumBools, pszBinderName, pBools, lNumBools, m_pEffect, hParam);
            // if (pNewBinder)
            // {
            //     pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
            // }
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
            uint32_t nCullMode = D3DCULL_CW;

            if (_stricmp(pszEnum, "TwoSided") == 0)
            {
                nCullMode = D3DCULL_NONE;
            }

            // TODO: Finish me
            // auto* pNewBinder = ZUniMemory::New<ZRenderMaterialBinderCullD3DFX>(pszBinderName, nCullMode, m_pEffect, hParam);
            // if (pNewBinder)
            // {
            //     pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
            // }
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

            // TODO: Finish me
            // switch (static_cast<RT_NAME>(lContextType))
            // {
            //     case RT_NAME::RTN_MAP_CUBE_NORMALIZER: // 0x19 (25)
            //         pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            //             hParam, pszBinderName, &g_texShadowMapDepth, m_pEffect, hParam,
            //             nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique
            //         );
            //         break;

            //     case RT_NAME::RTN_MAP_SCATTER: // 0x1A (26)
            //         pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            //             hParam, pszBinderName, &unk_90ACA8, m_pEffect, hParam,
            //             nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique
            //         );
            //         break;

            //     case RT_NAME::RTN_MAP_PHASE: // 0x1B (27)
            //         pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            //             hParam, pszBinderName, &unk_90AC60, m_pEffect, hParam,
            //             nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique
            //         );
            //         break;

            //     case RT_NAME::RTN_MAP_REFRACTION: // 0x1C (28)
            //         pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            //             hParam, pszBinderName, &unk_90AC18, m_pEffect, hParam,
            //             nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique
            //         );
            //         break;

            //     case RT_NAME::RTN_MAP_CUBE_SHADOW_COLOR: // 0x20 (32)
            //         pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            //             hParam, pszBinderName, &unk_90AAC0, m_pEffect, hParam,
            //             nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique
            //         );
            //         break;

            //     case RT_NAME::RTN_MAP_SHADOW_DEPTH: // 0x21 (33)
            //         pNewBinder = ZUniMemory::New<ZRenderMaterialBinderTextureD3DFX>(
            //             hParam, pszBinderName, &unk_90AA78, m_pEffect, hParam,
            //             nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_hTechnique
            //         );
            //         break;

            //     default:
            //         // В ветке default компилятор заинлайнил конструктор ZRenderBinderContextD3DFX (24 байта / 0x18)
            //         pNewBinder = ZUniMemory::New<ZRenderBinderContextD3DFX>(
            //             pszBinderName,
            //             3, // pContext / Type
            //             lContextType,
            //             m_pEffect,
            //             hParam
            //         );
            //         break;
            // }

            if (pNewBinder)
            {
                pMaterialBinderList->m_pBinders[pMaterialBinderList->m_lNumBinders++] = pNewBinder;
            }
        }
    }
    
    void ZRenderMaterialBinderParserD3DFX::CreatePropertyBinderSprite(ZRenderBinderList* pMaterialBinderList)
    {

    }
    // virtual void CreatePropertyBinderRenderState(
    //     ZRenderBinderList* pMaterialBinderList, 
    //     bool bBlendEnabled,
    //     const char* pszBlendMode, 
    //     float fOpacity, 
    //     bool bAlphaTestEnabled, 
    //     uint32_t lAlphaTestRef,
    //     bool bFogEnabled,
    //     const char* pszCullMode, 
    //     uint32_t lZBias,
    //     float fZOffset) override;
    // void CreatePropertyBinderScroll(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pfScrollSpeed) override;
}