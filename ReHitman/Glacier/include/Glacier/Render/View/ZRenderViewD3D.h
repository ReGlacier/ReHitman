#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/View/ZRenderView.h>


namespace Glacier
{
    class ZRenderViewD3D : public ZRenderView
    {
    public:
        // vtbl
        void CreateSurface(ZDrawSurface::TARGET lTarget) override;
        void CreatePostFilter() override;

        // methods
        using ZRenderView::ZRenderView;

        // members
    };
    RE_VERIFY_SIZE(ZRenderViewD3D, 0xF4); // Verified PC alloc
}