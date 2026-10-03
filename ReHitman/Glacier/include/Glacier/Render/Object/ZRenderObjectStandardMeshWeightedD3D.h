#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/Render/ZRVertexContainer.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderObjectStandardMeshWeightedD3D : public ZRenderObjectD3D
    {
    public:
        ~ZRenderObjectStandardMeshWeightedD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;
        void Update() override;

        ZRenderObjectStandardMeshWeightedD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        uint32_t m_lVertices; // +0x10
        ZRIndexContainer m_IndexContainer; // +0x14
        ZRVertexContainer m_VertexContainer; // +0x30
        bool m_bVerticesUploaded; // +0x48
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZRenderObjectStandardMeshWeightedD3D, 0x4C);
    RE_VERIFY_OFFSET(ZRenderObjectStandardMeshWeightedD3D, m_lVertices, 0x10);
    RE_VERIFY_OFFSET(ZRenderObjectStandardMeshWeightedD3D, m_IndexContainer, 0x14);
    RE_VERIFY_OFFSET(ZRenderObjectStandardMeshWeightedD3D, m_VertexContainer, 0x30);
    RE_VERIFY_OFFSET(ZRenderObjectStandardMeshWeightedD3D, m_bVerticesUploaded, 0x48);
}
