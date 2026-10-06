#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/Material/ZRenderBinderInt.h>


namespace Glacier
{
    class ZRenderMaterialBinderIntD3DFX : public ZRenderBinderInt
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderIntD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;
        int32_t* GetValues() override;

        // methods
        ZRenderMaterialBinderIntD3DFX(const char* pszName, uint32_t* pInts, uint32_t lNumInts, ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hInts);

        // members
        D3DXHANDLE m_hInts;
        int32_t m_Ints[4];
        uint32_t m_lNumInts;
        ZRenderMaterialEffectD3DFX* m_pEffect;
    };

    RE_VERIFY_SIZE(ZRenderMaterialBinderIntD3DFX, 0x28); // Verified PC alloc
    RE_VERIFY_OFFSET(ZRenderMaterialBinderIntD3DFX, m_Ints, 0x10); // Verified GetValues
}
