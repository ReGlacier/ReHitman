#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/ZRenderBinder.h>


namespace Glacier
{
    class ZRenderMaterialBinderScrollD3DFX : public ZRenderBinder
    {
    public:
        // vtbl
        ~ZRenderMaterialBinderScrollD3DFX() override;
        void Execute(const ZRenderContext* pContext) override;

        // methods
        ZRenderMaterialBinderScrollD3DFX(ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hScrollPosition, const float* pfScrollSpeed);

        // members
        float m_fScrollSpeed[4];
        D3DXHANDLE m_hScrollPosition;
        ZRenderMaterialEffectD3DFX* m_pEffect;
    };
    RE_VERIFY_SIZE(ZRenderMaterialBinderScrollD3DFX, 0x24); // Verified PC alloc
    RE_VERIFY_OFFSET(ZRenderMaterialBinderScrollD3DFX, m_fScrollSpeed, 0x0C); // Verified PC Execute
    RE_VERIFY_OFFSET(ZRenderMaterialBinderScrollD3DFX, m_hScrollPosition, 0x1C); // Verified PC Execute
    RE_VERIFY_OFFSET(ZRenderMaterialBinderScrollD3DFX, m_pEffect, 0x20); // Verified PC Execute
}
