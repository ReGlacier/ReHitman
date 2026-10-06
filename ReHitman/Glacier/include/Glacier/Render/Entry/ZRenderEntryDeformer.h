#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Render/Prim/ZPrimAccess.h>

namespace Glacier
{
    class ZRenderEntryDeformer : public ZRenderEntryGeom
    {
    public:
        ~ZRenderEntryDeformer() override;
        RENDERENTRY_BASETYPE GetType() const override;

        ZRenderEntryDeformer(const ZRenderEntryGeomCreateInfo& sInfo);
    };

    RE_VERIFY_SIZE(ZRenderEntryDeformer, 0x9C);
}
