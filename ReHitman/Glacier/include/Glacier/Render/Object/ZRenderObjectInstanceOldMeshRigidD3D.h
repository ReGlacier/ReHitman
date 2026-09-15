#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceOldMeshRigidD3D : public ZRenderObjectInstanceD3D
    {
    public:
        ~ZRenderObjectInstanceOldMeshRigidD3D() override = default;
        void Draw(ZRenderContext* pCtx) override;

        using ZRenderObjectInstanceD3D::ZRenderObjectInstanceD3D;
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceOldMeshRigidD3D, 0x18);
}
