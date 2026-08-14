#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/Render/ZEffectCacheD3DFX.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialResource
    {
    public:
        // vtbl
        virtual ~ZRenderMaterialResource() = default;

        // methods
        ZRenderMaterialResource() = default;
    };

    class ZRenderMaterialResourceD3D : public ZRenderMaterialResource
    {
    public:
        // vtbl
        ~ZRenderMaterialResourceD3D() override = default;

        // methods
        ZRenderMaterialResourceD3D() = default;
    };
    
    class ZRenderMaterialResourceD3DFX : public ZRenderMaterialResourceD3D
    {
    public:
        // constants
        static constexpr uint32_t MAX_NUM_FX_MATERIAL_LAYERS = 640;
        static constexpr uint32_t MAX_UNM_FX_EFFECTS = 256;

        // vtbl
        ~ZRenderMaterialResourceD3DFX() override;
        
        // methods
        ZRenderMaterialResourceD3DFX();

        static void GetShaderPath(zstring& sPath);
        void CreateEffect(const char* pszFileName, const D3DXMACRO* pDefined, uint32_t lNumDefines);
        void AddEffect(const char* pszFileName, ZRenderMaterialSubClassD3D* pMaterialSubClass);
        ZRenderMaterialLayerD3DFX* AddLayer(ZRenderMaterialLayerD3DFX* pLayer);
        void CreateMaterialLayer(ZRenderMaterialSubClassD3D* pMaterialSubClass, const ZRPropertyReader* pMatPropReader);
        void FreeResources();

        // members
        uint32_t m_lNumLayers; // 0x0004
        ZRenderMaterialLayerD3DFX* m_pMaterialLayers[MAX_NUM_FX_MATERIAL_LAYERS]; // 0x0008 .. 0x0A08
        uint32_t m_lNumEffects; // 0x0A08
        ZRenderMaterialEffectD3DFX* m_pEffects[MAX_UNM_FX_EFFECTS]; // 0x0A0C .. 0x0E0C
        uint32_t m_pad0E0C;
        ZEffectCacheD3DFX m_EffectCache; // 0x0E10 .. 0x2620
        uint32_t m_unknown2620;
    };
    RE_VERIFY_SIZE(ZRenderMaterialResourceD3DFX, 0x2628); // Verified with PC
    RE_VERIFY_OFFSET(ZRenderMaterialResourceD3DFX, m_EffectCache, 0x0E10); // Verified with PC ctor
}