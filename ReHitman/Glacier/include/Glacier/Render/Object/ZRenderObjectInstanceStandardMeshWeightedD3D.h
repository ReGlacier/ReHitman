#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceStandardMeshWeightedD3D : public ZRenderObjectInstanceD3D
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceStandardMeshWeightedD3D() override;
        void Draw(ZRenderContext* pCtx) override;

        // methods
        ZRenderObjectInstanceStandardMeshWeightedD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceStandardMeshWeightedD3D, 0x18);
}
