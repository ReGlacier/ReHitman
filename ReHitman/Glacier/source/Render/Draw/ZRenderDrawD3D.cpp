#include <Glacier/Render/Draw/ZRenderDrawD3D.h>
#include <Glacier/Render/View/ZRenderViewD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryBones.h>
#include <Glacier/Geom/ZBaseGeom.h>
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
        m_DecalMarks.BeginFrame();
    }

    ZRenderViewBase* ZRenderDrawD3D::NewView(ZRender* pRender, uint32_t lViewNumber, uint32_t lViewId)
    {
        return ZUniMemory::New<ZRenderViewD3D>(pRender, this, lViewNumber, lViewId);
    }

    void ZRenderDrawD3D::CalcBoneLightSources(ZBaseGeom* pBaseGeom, float* pDirectLights)
    {
        if (!pBaseGeom || !pBaseGeom->m_lDrawId)
            return;

        auto* pEntry = m_apRenderEntryLookup[pBaseGeom->m_lDrawId & 0x7FFFu];
        if (pEntry && (pEntry->m_lControl & ZRenderEntry::RE_HASBONES) != 0)
            static_cast<ZRenderEntryBones*>(pEntry)->m_pLightData = pDirectLights;
    }
}
