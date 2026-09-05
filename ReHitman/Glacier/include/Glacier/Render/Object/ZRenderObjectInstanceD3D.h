#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceX86.h>


namespace Glacier
{
    class ZRenderObjectInstanceD3D : public ZRenderObjectInstanceX86
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceD3D() override = default;

        // methods
        using ZRenderObjectInstanceX86::ZRenderObjectInstanceX86;

        void DrawIndexedTriangles(const ZRIndexContainer* pIndexContainer, uint32_t lNumIndices, ZRenderWintelD3D* pRender, uint32_t lObjectSubType);
        void DrawIndexedStrips(const ZRIndexContainer* pIndexContainer, uint32_t lNumIndices, ZRenderWintelD3D* pRender, uint32_t lObjectSubType);
    };
}