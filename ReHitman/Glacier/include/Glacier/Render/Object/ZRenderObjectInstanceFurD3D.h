#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceFurD3D : public ZRenderObjectInstanceD3D
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceFurD3D() override;
        void Draw(ZRenderContext* pCtx) override;

        // methods
        ZRenderObjectInstanceFurD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceFurD3D, 0x18);
}
