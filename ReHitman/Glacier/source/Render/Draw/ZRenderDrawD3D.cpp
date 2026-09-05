#include <Glacier/Render/Draw/ZRenderDrawD3D.h>
#include <Glacier/Render/View/ZRenderViewD3D.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderDrawD3D::ZRenderDrawD3D()
        : ZRenderDraw()
    {
        // Do nothing
    }

    void ZRenderDrawD3D::BeginFrame()
    {
        // TODO: Finish me
    }

    ZRenderViewBase* ZRenderDrawD3D::NewView(ZRender* pRender, uint32_t lViewNumber, uint32_t lViewId)
    {
        return ZUniMemory::New<ZRenderViewD3D>(pRender, this, lViewNumber, lViewId);
    }

    void ZRenderDrawD3D::CalcBoneLightSources(ZBaseGeom* pBaseGeom, float* pDirectLights)
    {
        // TODO: Finish me
    }
}