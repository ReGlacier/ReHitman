#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/Render/ZRVertexContainer.h>
#include <cstdint>

namespace Glacier
{
    class ZRenderObjectWaterPatchD3D : public ZRenderObjectD3D
    {
    public:
        ~ZRenderObjectWaterPatchD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        ZRenderObjectWaterPatchD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        void* m_pUnknown; // +0x10
        void* m_pTempVertices; // +0x14
        ZRIndexContainer m_IndexContainer; // +0x18
        ZRVertexContainer m_VertexContainer; // +0x34
        uint8_t m_lUnknown; // +0x4C
        uint8_t m_lMode; // +0x4D
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZRenderObjectWaterPatchD3D, 0x50); // Verified PC alloc
    RE_VERIFY_OFFSET(ZRenderObjectWaterPatchD3D, m_pUnknown, 0x10);
    RE_VERIFY_OFFSET(ZRenderObjectWaterPatchD3D, m_pTempVertices, 0x14);
    RE_VERIFY_OFFSET(ZRenderObjectWaterPatchD3D, m_IndexContainer, 0x18);
    RE_VERIFY_OFFSET(ZRenderObjectWaterPatchD3D, m_VertexContainer, 0x34);
    RE_VERIFY_OFFSET(ZRenderObjectWaterPatchD3D, m_lMode, 0x4D);
}
