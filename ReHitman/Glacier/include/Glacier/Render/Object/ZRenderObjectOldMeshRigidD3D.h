#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/Render/ZRVertexContainer.h>


namespace Glacier
{
    class ZRenderObjectOldMeshRigidD3D : public ZRenderObjectD3D
    {
    public:
        ~ZRenderObjectOldMeshRigidD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        ZRenderObjectOldMeshRigidD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        ZRIndexContainer m_IndexContainer; // +0x10
        ZRVertexContainer m_VertexContainer; // +0x2C
    };
    RE_VERIFY_SIZE(ZRenderObjectOldMeshRigidD3D, 0x44);
    RE_VERIFY_OFFSET(ZRenderObjectOldMeshRigidD3D, m_IndexContainer, 0x10);
    RE_VERIFY_OFFSET(ZRenderObjectOldMeshRigidD3D, m_VertexContainer, 0x2C);
}
