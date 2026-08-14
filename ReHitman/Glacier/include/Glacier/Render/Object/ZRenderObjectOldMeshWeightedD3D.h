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
        ZRIndexContainer m_IndexContainer;
        ZRVertexContainer m_VertexContainer;
    };
    RE_VERIFY_SIZE(ZRenderObjectOldMeshWeightedD3D, 0x44);
}