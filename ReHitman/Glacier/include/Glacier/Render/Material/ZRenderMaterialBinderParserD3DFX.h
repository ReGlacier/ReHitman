#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderParserD3D.h>
#include <Glacier/Render/D3D9.h>


namespace Glacier
{
    class ZRenderMaterialBinderParserD3DFX : public ZRenderMaterialBinderParserD3D
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderParserD3DFX() override;
        void CreatePropertyBinderTexture(
            ZRenderBinderList* pBinderList, 
            const char* pszBinderName, 
            uint32_t lTextureId, 
            const char* pszMinFilter, 
            const char* pszMagFilter, 
            const char* pszMipFilter, 
            const char* pszTilingU, 
            const char* pszTilingV, 
            const char* pszTilingW) override;
        virtual void CreatePropertyBinderFloat(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pFloats, uint32_t lNumFloats) override;
        virtual void CreatePropertyBinderBool(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const uint32_t* pBools, uint32_t lNumBools) override;
        virtual void CreatePropertyBinderEnum(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const char* pszEnum) override;
        virtual void CreatePropertyBinderColor(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, const float* pColor) override;
        virtual void CreatePropertyBinderContext(ZRenderBinderList* pMaterialBinderList, const char* pszBinderName, uint32_t lContextType) override;
        virtual void CreatePropertyBinderSprite(ZRenderBinderList* pMaterialBinderList) override;
        virtual void CreatePropertyBinderRenderState(
            ZRenderBinderList* pMaterialBinderList, 
            bool bBlendEnabled,
            const char* pszBlendMode, 
            float fOpacity, 
            bool bAlphaTestEnabled, 
            uint32_t lAlphaTestRef,
            bool bFogEnabled,
            const char* pszCullMode, 
            uint32_t lZBias,
            float fZOffset) override;
        void CreatePropertyBinderScroll(ZRenderBinderList* pMaterialBinderList, const float* pfScrollSpeed) override;

        // methods
        ZRenderMaterialBinderParserD3DFX(ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hTechnique);

        // TODO: Finish me

        // members
        ZRenderMaterialEffectD3DFX* m_pEffect { nullptr }; // +0x8
        D3DXHANDLE m_hTechnique { nullptr }; // +0xC
    };
    // ZRenderMaterialBinderParserD3DFX
}