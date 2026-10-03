#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceSpritesArrayRainD3D : public ZRenderObjectInstanceD3D
    {
    public:
        ~ZRenderObjectInstanceSpritesArrayRainD3D() override;
        void Draw(ZRenderContext* pContext) override;

        using ZRenderObjectInstanceD3D::ZRenderObjectInstanceD3D;
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceSpritesArrayRainD3D, 0x18);
}
