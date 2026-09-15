#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArrayRainD3D.h>
#include <Glacier/Render/Object/ZRenderObjectSpritesArrayRainD3D.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectSpritesArrayRainD3D::ZRenderObjectSpritesArrayRainD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
    {
    }

    ZRenderObjectInstance* ZRenderObjectSpritesArrayRainD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceSpritesArrayRainD3D>(this, pBaseGeom);
    }
}
