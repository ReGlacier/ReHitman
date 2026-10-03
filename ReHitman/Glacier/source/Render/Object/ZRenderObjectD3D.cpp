#include <Glacier/Render/Object/ZRenderObjectD3D.h>


namespace Glacier
{
    ZRenderObjectD3D::~ZRenderObjectD3D() = default;
    
    ZRenderObjectD3D::ZRenderObjectD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectX86(hPrim, pMaterialInstance)
    {
    }
}