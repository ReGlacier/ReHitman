#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>


namespace Glacier
{
    class ZRenderDrawD3D : public ZRenderDraw
    {
    public:
        // vtbl
        void BeginFrame() override;
        ZRenderViewBase* NewView(ZRender* pRender, uint32_t lViewNumber, uint32_t lViewId) override;
        void CalcBoneLightSources(ZBaseGeom* pBaseGeom, float* pDirectLights) override;

        // methods
        ZRenderDrawD3D();

        // members (no new members?)
    };
    RE_VERIFY_SIZE(ZRenderDrawD3D, 0x58864); // Verified PC allocation
}