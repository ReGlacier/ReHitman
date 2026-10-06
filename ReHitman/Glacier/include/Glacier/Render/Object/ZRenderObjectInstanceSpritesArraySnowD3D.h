#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceSpritesArraySnowD3D : public ZRenderObjectInstanceD3D
    {
    public:
        ~ZRenderObjectInstanceSpritesArraySnowD3D() override;
        void Draw(ZRenderContext* pContext) override;

        using ZRenderObjectInstanceD3D::ZRenderObjectInstanceD3D;
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceSpritesArraySnowD3D, 0x18);
}
