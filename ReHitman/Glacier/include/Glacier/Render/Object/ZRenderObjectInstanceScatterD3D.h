#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>


namespace Glacier
{
    class ZRenderObjectInstanceScatterD3D : public ZRenderObjectInstanceD3D
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceScatterD3D() override = default;
        void Draw(ZRenderContext* pCtx) override;

        // methods
        using ZRenderObjectInstanceD3D::ZRenderObjectInstanceD3D;

        // members
        uint32_t m_Unk18;
        uint32_t m_Unk1C;
        RE_ADD_PADDING(4);
        uint32_t m_Unk24;
        uint32_t m_Unk28;
        uint32_t m_Unk2C;
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceScatterD3D, 0x30);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceScatterD3D, m_Unk18, 0x18);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceScatterD3D, m_Unk1C, 0x1C);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceScatterD3D, m_Unk24, 0x24);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceScatterD3D, m_Unk28, 0x28);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceScatterD3D, m_Unk2C, 0x2C);
}
