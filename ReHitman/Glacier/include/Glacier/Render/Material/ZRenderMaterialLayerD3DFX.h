#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Material/ZRenderMaterialLayer.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialLayerD3DFX : public ZRenderMaterialLayer
    {
    public:
        // vtbl
        ~ZRenderMaterialLayerD3DFX() override;
        uint32_t BeginSubClass(const ZRenderMaterialSubClass* pSubClass, const ZRenderContext* pContext) override;
        void EndSubClass() override;
        void BeginPass(uint32_t, const ZRenderContext* pContext) override;
        void EndPass() override;
        void BeginInstance(const ZRenderMaterialInstance* pMaterialInstance, const ZRenderMaterialBinderList* pBinderList, const ZRenderContext* pContext) override;
        void EndInstance() override;
        void BeginObject(const ZRenderMaterialInstance* pMaterialInstance, const ZRenderMaterialBinderList* pBinderList, const ZRenderObjectInstance* pInstance, const ZRenderContext* pContext) override;
        void EndObject() override;
        uint32_t NumPasses() override;
        void CreateBinders(int nParam, ZRenderMaterialInstance* pMaterialInstance, ZRenderMaterialBinderList* pBinderList) override;

        // methods
        ZRenderMaterialLayerD3DFX(const char* pszName, D3DXHANDLE hTechnique, ZRenderMaterialEffectD3DFX* pEffect);

        // members
        ZRenderMaterialEffectD3DFX* m_pEffect { nullptr };
        D3DXHANDLE m_hTechnique {nullptr};
        D3DXTECHNIQUE_DESC m_TechniqueDesc {};
    };
    RE_VERIFY_SIZE(ZRenderMaterialLayerD3DFX, 0x1C); // Verified PC
}