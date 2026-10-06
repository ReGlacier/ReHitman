#include <Glacier/Render/Entry/ZRenderEntryDeformer.h>

namespace Glacier
{
    ZRenderEntryDeformer::ZRenderEntryDeformer(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryGeom(sInfo)
    {
    }

    ZRenderEntryDeformer::~ZRenderEntryDeformer() = default;

    ZRenderEntryDeformer::RENDERENTRY_BASETYPE ZRenderEntryDeformer::GetType() const
    {
        return RT_DEFORMER;
    }
}
