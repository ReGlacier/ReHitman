#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/Material/ZRenderBinderContextD3D.h>


namespace Glacier
{
    class ZRenderBinderContextD3DFX : public ZRenderBinderContextD3D
    {
    public:
        // vtbl
        ~ZRenderBinderContextD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;
        virtual void SetFloatArray(const float* pFloats, uint32_t lNumFloats) override;
        virtual void SetTexture(IDirect3DBaseTexture9* pTexture) override;

        // methods
        ZRenderBinderContextD3DFX(const char* pszName, uint32_t lContextType, ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hParamter);

        // members
        ZRenderMaterialEffectD3DFX* m_pEffect;
        D3DXHANDLE m_hParamter;
    };
    RE_VERIFY_SIZE(ZRenderBinderContextD3DFX, 0x18); // Verified PC alloc
}
