#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>


namespace Glacier
{
    class ZRenderObjectSpritesArrayRainD3D : public ZRenderObjectD3D
    {
    public:
        ~ZRenderObjectSpritesArrayRainD3D() override = default;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        ZRenderObjectSpritesArrayRainD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);
    };
    RE_VERIFY_SIZE(ZRenderObjectSpritesArrayRainD3D, 0x10);
}
