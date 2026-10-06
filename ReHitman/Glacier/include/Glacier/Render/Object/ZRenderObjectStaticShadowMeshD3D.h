#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/Render/ZRVertexContainer.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderObjectStaticShadowMeshD3D : public ZRenderObjectD3D
    {
    public:
        ~ZRenderObjectStaticShadowMeshD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        ZRenderObjectStaticShadowMeshD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        uint32_t m_Unk10; // +0x10
        ZRIndexContainer m_IndexContainer; // +0x14
        ZRVertexContainer m_VertexContainer; // +0x30
    };
    RE_VERIFY_SIZE(ZRenderObjectStaticShadowMeshD3D, 0x48);
    RE_VERIFY_OFFSET(ZRenderObjectStaticShadowMeshD3D, m_Unk10, 0x10);
    RE_VERIFY_OFFSET(ZRenderObjectStaticShadowMeshD3D, m_IndexContainer, 0x14);
    RE_VERIFY_OFFSET(ZRenderObjectStaticShadowMeshD3D, m_VertexContainer, 0x30);
}
