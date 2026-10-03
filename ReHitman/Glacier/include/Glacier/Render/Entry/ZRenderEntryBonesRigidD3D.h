#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryBonesRigid.h>

namespace Glacier
{
    class ZRenderEntryBonesRigidD3D : public ZRenderEntryBonesRigid
    {
    public:
        ~ZRenderEntryBonesRigidD3D() override;

        ZRenderEntryBonesRigidD3D(const ZRenderEntryGeomCreateInfo& sInfo);
        static ZRenderEntryBonesRigidD3D* Create(const ZRenderEntryGeomCreateInfo& sInfo);
    };

    RE_VERIFY_SIZE(ZRenderEntryBonesRigidD3D, 0xB4);
}
