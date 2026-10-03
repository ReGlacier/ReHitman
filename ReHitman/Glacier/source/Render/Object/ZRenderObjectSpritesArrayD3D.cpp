#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArrayD3D.h>
#include <Glacier/Render/Object/ZRenderObjectSpritesArrayD3D.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectSpritesArrayD3D::ZRenderObjectSpritesArrayD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
    {
    }

    ZRenderObjectInstance* ZRenderObjectSpritesArrayD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceSpritesArrayD3D>(this, pBaseGeom);
    }
}
