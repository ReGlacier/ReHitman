#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRVertexContainer.h>
#include <Glacier/Render/ZRIndexContainer.h>


namespace Glacier
{
    class ZRenderObjectOldMeshD3D : public ZRenderObjectD3D
    {
    public:
        // vtbl
        ~ZRenderObjectOldMeshD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        // methods
        ZRenderObjectOldMeshD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        // members
        ZRIndexContainer m_IndexContainer; // +0x10
        ZRVertexContainer m_VertexContainer; // 0x2C
    };
    RE_VERIFY_SIZE(ZRenderObjectOldMeshD3D, 0x44); // Verified PC
    RE_VERIFY_OFFSET(ZRenderObjectOldMeshD3D, m_IndexContainer, 0x10); // Verified
    RE_VERIFY_OFFSET(ZRenderObjectOldMeshD3D, m_VertexContainer, 0x2C); // Verified
}