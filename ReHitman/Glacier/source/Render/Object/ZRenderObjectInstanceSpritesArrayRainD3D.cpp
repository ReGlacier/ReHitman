#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArrayRainD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArrayD3D.h>


namespace Glacier
{
    ZRenderObjectInstanceSpritesArrayRainD3D::~ZRenderObjectInstanceSpritesArrayRainD3D() = default;

    void ZRenderObjectInstanceSpritesArrayRainD3D::Draw(ZRenderContext* pContext)
    {
        DrawSpriteArrayD3D(this, pContext);
    }
}
