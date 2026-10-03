#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/Material/ZRenderBinderFloat.h>


namespace Glacier
{
    class ZRenderMaterialBinderFloatD3DFX : public ZRenderBinderFloat
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderFloatD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;
        const float* GetValues() const override;

        // methods
        ZRenderMaterialBinderFloatD3DFX(const char* pszName, const float* pFloats, uint32_t lNumFloats, ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hFloats);

        // members
        D3DXHANDLE m_hFloats;
        const float* m_pFloats;
        uint32_t m_lNumFloats;
        ZRenderMaterialEffectD3DFX* m_pEffect;
    };

    RE_VERIFY_SIZE(ZRenderMaterialBinderFloatD3DFX, 0x1C);
    RE_VERIFY_OFFSET(ZRenderMaterialBinderFloatD3DFX, m_pFloats, 0x10);
}
