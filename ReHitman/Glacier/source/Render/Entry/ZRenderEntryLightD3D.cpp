#include <Glacier/Render/Entry/ZRenderEntryLightD3D.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderEntryLightD3D::ZRenderEntryLightD3D(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryLight(sInfo)
    {
    }

    ZRenderEntryLightD3D::~ZRenderEntryLightD3D() = default;

    void ZRenderEntryLightD3D::GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList)
    {
        // TODO: Finish me (PC 004B6540)
    }

    ZRenderEntryLightD3D* ZRenderEntryLightD3D::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        return ZUniMemory::New<ZRenderEntryLightD3D>(sInfo);
    }
}
