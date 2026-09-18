#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Material/ZRenderBinderContext.h>

namespace Glacier
{
    class ZRenderBinderContextD3D : public ZRenderBinderContext
    {
    public:
        // vtbl
        ~ZRenderBinderContextD3D() override;
        uint32_t GetContextType() const override;
        virtual void SetFloatArray(const float* pFloats, uint32_t lNumFloats) = 0;
        virtual void SetTexture(IDirect3DBaseTexture9* pTexture) = 0;

        // methods
        using ZRenderBinderContext::ZRenderBinderContext;

        // memebers
        uint32_t m_lContextType;
    };

    RE_VERIFY_OFFSET(ZRenderBinderContextD3D, m_lContextType, 0xC); // Verified by GetContextType
}
