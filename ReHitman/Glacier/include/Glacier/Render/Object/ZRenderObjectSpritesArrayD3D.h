#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>


namespace Glacier
{
    class ZRenderObjectSpritesArrayD3D : public ZRenderObjectD3D
    {
    public:
        ~ZRenderObjectSpritesArrayD3D() override = default;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        ZRenderObjectSpritesArrayD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);
    };
    RE_VERIFY_SIZE(ZRenderObjectSpritesArrayD3D, 0x10);
}
