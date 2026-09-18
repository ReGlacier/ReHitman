#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceD3D;
    void DrawSpriteArrayD3D(ZRenderObjectInstanceD3D* pInstance, ZRenderContext* pContext);

    class ZRenderObjectInstanceSpritesArrayD3D : public ZRenderObjectInstanceD3D
    {
    public:
        ~ZRenderObjectInstanceSpritesArrayD3D() override;
        void Draw(ZRenderContext* pContext) override;

        using ZRenderObjectInstanceD3D::ZRenderObjectInstanceD3D;
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceSpritesArrayD3D, 0x18);
}
