#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryLight.h>


namespace Glacier
{
    class ZRenderEntryLightD3D : public ZRenderEntryLight
    {
    public:
        // vtbl
        ~ZRenderEntryLightD3D() override;
        void GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList) override;

        // methods
        ZRenderEntryLightD3D(const ZRenderEntryGeomCreateInfo& sInfo);

        static ZRenderEntryLightD3D* Create(const ZRenderEntryGeomCreateInfo& sInfo);
    };
    RE_VERIFY_SIZE(ZRenderEntryLightD3D, 0xA0); // Verified PC alloc
}
