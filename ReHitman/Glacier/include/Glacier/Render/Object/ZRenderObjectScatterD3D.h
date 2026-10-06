#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/Render/ZRVertexContainer.h>


namespace Glacier
{
    class ZRenderObjectScatterD3D : public ZRenderObjectD3D
    {
    public:
        // vtbl
        ~ZRenderObjectScatterD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;
        void Update() override;

        // methods
        ZRenderObjectScatterD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        // members
        uint32_t m_lTotalIndexCount; // +0x10
        ZRIndexContainer m_IndexContainer; // +0x14
        ZRVertexContainer m_VertexContainer; // +0x30
        bool m_bIndexBufferLimited; // +0x48
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZRenderObjectScatterD3D, 0x4C);
    RE_VERIFY_OFFSET(ZRenderObjectScatterD3D, m_lTotalIndexCount, 0x10);
    RE_VERIFY_OFFSET(ZRenderObjectScatterD3D, m_IndexContainer, 0x14);
    RE_VERIFY_OFFSET(ZRenderObjectScatterD3D, m_VertexContainer, 0x30);
    RE_VERIFY_OFFSET(ZRenderObjectScatterD3D, m_bIndexBufferLimited, 0x48);
}
