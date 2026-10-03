#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/Material/ZRenderBinderInt.h>


namespace Glacier
{
    class ZRenderMaterialBinderCullD3DFX : public ZRenderBinderInt
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderCullD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;
        int32_t* GetValues() override;

        // methods
        ZRenderMaterialBinderCullD3DFX(const char *pszName, D3DCULL dwCull, ZRenderMaterialEffectD3DFX *pEffect, D3DXHANDLE hCull);

        // members
        D3DXHANDLE m_hCull;
        D3DCULL m_dwCull;
        ZRenderMaterialEffectD3DFX* m_pEffect;
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderCullD3DFX, 0x18); // Verified PC alloc
}
