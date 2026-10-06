#pragma once

#include <Glacier/Render/Object/ZRenderObjectX86.h>


namespace Glacier
{
    class ZRenderObjectD3D : public ZRenderObjectX86
    {
    public:
        // vtbl
        ~ZRenderObjectD3D() override;

        // methods
        ZRenderObjectD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        // members
    };
    RE_VERIFY_SIZE(ZRenderObjectD3D, 0x10);
}
