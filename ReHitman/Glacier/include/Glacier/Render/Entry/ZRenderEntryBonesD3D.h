#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryBones.h>


namespace Glacier
{
    class ZRenderEntryBonesD3D : public ZRenderEntryBones
    {
    public:
        // vtbl
        ~ZRenderEntryBonesD3D() override;

        // methods
        ZRenderEntryBonesD3D(const ZRenderEntryGeomCreateInfo& sInfo);

        static ZRenderEntryBonesD3D* Create(const ZRenderEntryGeomCreateInfo& sInfo);

        // members
    };
    RE_VERIFY_SIZE(ZRenderEntryBonesD3D, 0xB0); // Verified by PC allocation at ZRenderEntryBonesD3D::Produce
}
