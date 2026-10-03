#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArraySnowD3D.h>
#include <Glacier/Render/Object/ZRenderObjectSpritesArraySnowD3D.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectSpritesArraySnowD3D::ZRenderObjectSpritesArraySnowD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
    {
    }

    ZRenderObjectInstance* ZRenderObjectSpritesArraySnowD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceSpritesArraySnowD3D>(this, pBaseGeom);
    }
}
