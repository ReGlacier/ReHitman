#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectD3D.h>
#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/Render/ZRVertexContainer.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderObjectFurD3D : public ZRenderObjectD3D // PC vtable 0x765454
    {
    public:
        // vtbl
        ~ZRenderObjectFurD3D() override;
        ZRenderObjectInstance* CreateInstance(ZBaseGeom* pBaseGeom) override;

        // methods
        ZRenderObjectFurD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance);

        // members
        uint32_t m_Unk10; // +0x10
        ZRIndexContainer m_MainIndexContainer; // +0x14
        ZRVertexContainer m_MainVertexContainer; // +0x30
        uint32_t m_Unk48; // +0x48
        ZRIndexContainer m_FinIndexContainer; // +0x4C
        ZRVertexContainer m_FinVertexContainer; // +0x68
        uint32_t m_Unk80; // +0x80
        uint32_t m_Unk84; // +0x84
        ZRIndexContainer m_ShellIndexContainer; // +0x88
        ZRVertexContainer m_ShellVertexContainer; // +0xA4
    };
    RE_VERIFY_SIZE(ZRenderObjectFurD3D, 0xBC);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_Unk10, 0x10);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_MainIndexContainer, 0x14);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_MainVertexContainer, 0x30);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_Unk48, 0x48);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_FinIndexContainer, 0x4C);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_FinVertexContainer, 0x68);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_Unk80, 0x80);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_Unk84, 0x84);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_ShellIndexContainer, 0x88);
    RE_VERIFY_OFFSET(ZRenderObjectFurD3D, m_ShellVertexContainer, 0xA4);
}
