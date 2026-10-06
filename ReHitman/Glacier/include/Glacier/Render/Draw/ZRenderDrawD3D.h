#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>


namespace Glacier
{
    class ZRenderDrawD3D : public ZRenderDraw
    {
    public:
        // vtbl
        void InitAllocation() override;
        void BeginFrame() override;
        void Update(ZRender* pRender) override;
        ZRenderViewBase* NewView(ZRender* pRender, uint32_t lViewNumber, uint32_t lViewId) override;
        void CalcBoneLightSources(ZRenderEntryBones* pRenderEntryBones, float* pDirectLights) override;

        // methods
        ZRenderDrawD3D();

        bool HasLightBoneSelfShadow(ZREF rLight, ZREF rBones) const;
    };
    RE_VERIFY_SIZE(ZRenderDrawD3D, 0x58864); // Verified PC allocation
}
