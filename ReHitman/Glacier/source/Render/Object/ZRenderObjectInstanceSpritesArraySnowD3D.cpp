#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArraySnowD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArrayD3D.h>


namespace Glacier
{
    ZRenderObjectInstanceSpritesArraySnowD3D::~ZRenderObjectInstanceSpritesArraySnowD3D() = default;

    void ZRenderObjectInstanceSpritesArraySnowD3D::Draw(ZRenderContext* pContext)
    {
        DrawSpriteArrayD3D(this, pContext);
    }
}
