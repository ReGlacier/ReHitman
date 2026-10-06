#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryGeomD3D.h>

namespace Glacier
{
    class ZRenderEntryReflectorD3D : public ZRenderEntryGeomD3D
    {
    public:
        // vtbl
        ~ZRenderEntryReflectorD3D() override;
        void GetVisible(ZCmdList*, ZRenderEntryGeom*, ZViewSpace*, ZRenderView*, ZRenderEntryLists*) override;
        RENDERENTRY_BASETYPE GetType() const override;

        // methods
        ZRenderEntryReflectorD3D(const ZRenderEntryGeomCreateInfo& sInfo);
        static ZRenderEntryReflectorD3D* Create(const ZRenderEntryGeomCreateInfo& sInfo);

        // members
        uint8_t m_bReflectionEnabled;       // 0x9C
        uint8_t m_bRefractionEnabled;       // 0x9D
        uint8_t m_bTwoSided;                // 0x9E
        uint8_t m_bCurrentRoomOnly;         // 0x9F
        uint8_t m_bCustomFog;               // 0xA0
        uint8_t m_PAD_A1[3];
        float m_fFogNear;                   // 0xA4
        float m_fFogFar;                    // 0xA8
        uint32_t m_lFogColor;               // 0xAC
    };

    RE_VERIFY_OFFSET(ZRenderEntryReflectorD3D, m_bReflectionEnabled, 0x9C);
    RE_VERIFY_OFFSET(ZRenderEntryReflectorD3D, m_fFogNear, 0xA4);
    RE_VERIFY_OFFSET(ZRenderEntryReflectorD3D, m_lFogColor, 0xAC);
    RE_VERIFY_SIZE(ZRenderEntryReflectorD3D, 0xB0);
}
