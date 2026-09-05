#pragma once

#include <Glacier/ReGlacier.h>

// 764BEC	4		ZRenderObjectInstanceOldMeshWeightedD3D	ZRenderObjectInstanceOldMeshWeightedD3D: ZRenderObjectInstanceD3D, ZRenderObjectInstanceX86, ZRenderObjectInstance;
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceOldMeshWeightedD3D : public ZRenderObjectInstanceD3D
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceOldMeshWeightedD3D() override;
        void Draw(ZRenderContext* pCtx) override;

        // methods
        using ZRenderObjectInstanceD3D::ZRenderObjectInstanceD3D;
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceOldMeshWeightedD3D, 0x18);
}