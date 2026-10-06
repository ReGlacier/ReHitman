#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>


namespace Glacier
{
    class ZRenderObjectSpritesArraySnowD3D : public ZRenderObjectD3D
    {
    public:
        ~ZRenderObjectSpritesArraySnowD3D() override = default;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        ZRenderObjectSpritesArraySnowD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);
    };
    RE_VERIFY_SIZE(ZRenderObjectSpritesArraySnowD3D, 0x10);
}
