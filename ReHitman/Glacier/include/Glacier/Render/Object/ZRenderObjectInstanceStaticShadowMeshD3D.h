#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceStaticShadowMeshD3D : public ZRenderObjectInstanceD3D
    {
    public:
        ~ZRenderObjectInstanceStaticShadowMeshD3D() override;
        void Draw(ZRenderContext* pCtx) override;

        ZRenderObjectInstanceStaticShadowMeshD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceStaticShadowMeshD3D, 0x18);
}
