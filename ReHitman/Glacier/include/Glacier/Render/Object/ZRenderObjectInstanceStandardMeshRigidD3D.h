#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceStandardMeshRigidD3D : public ZRenderObjectInstanceD3D
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceStandardMeshRigidD3D() override;
        void Draw(ZRenderContext* pCtx) override;

        // methods
        ZRenderObjectInstanceStandardMeshRigidD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceStandardMeshRigidD3D, 0x18);
}
