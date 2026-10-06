#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRVertexContainer.h>
#include <Glacier/Render/ZRIndexContainer.h>


namespace Glacier
{
    class ZRenderObjectOldMeshWeightedD3D : public ZRenderObjectD3D
    {
    public:
        // vtbl
        ~ZRenderObjectOldMeshWeightedD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        // methods
        ZRenderObjectOldMeshWeightedD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        // members
        ZRIndexContainer m_IndexContainer; // +0x10
        ZRVertexContainer m_VertexContainer; // +0x2C
    };
    RE_VERIFY_SIZE(ZRenderObjectOldMeshWeightedD3D, 0x44);
    RE_VERIFY_OFFSET(ZRenderObjectOldMeshWeightedD3D, m_IndexContainer, 0x10);
    RE_VERIFY_OFFSET(ZRenderObjectOldMeshWeightedD3D, m_VertexContainer, 0x2C);
}
