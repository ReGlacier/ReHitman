#pragma once

#include <Glacier/Render/Entry/ZRenderEntryGeomD3D.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    class ZRenderEntryEnvSamplerD3D : public ZRenderEntryGeomD3D
    {
    public:
        // PC keeps four independent sampler groups, with sixteen entries per group.
        STATIC_CLASS_VAR_ARRAY(ZRenderEntryEnvSamplerD3D, ZRenderEntryEnvSamplerD3D*, m_pActive, 4);
        STATIC_CLASS_VAR_ARRAY(ZRenderEntryEnvSamplerD3D, uint32_t, m_lNumSamplers, 4);
        STATIC_CLASS_VAR_ARRAY(ZRenderEntryEnvSamplerD3D, ZRenderEntryEnvSamplerD3D*, m_pSamplers, 64);
        STATIC_CLASS_VAR(ZRenderEntryEnvSamplerD3D, uint32_t, m_lLastFrameUpdated);

        ~ZRenderEntryEnvSamplerD3D() override;
        void GetVisible(ZCmdList*, ZRenderEntryGeom*, ZViewSpace*, ZRenderView*, ZRenderEntryLists*) override;
        RENDERENTRY_BASETYPE GetType() const override;
        ZRenderEntryEnvSamplerD3D(const ZRenderEntryGeomCreateInfo&);
        static ZRenderEntryEnvSamplerD3D* Create(const ZRenderEntryGeomCreateInfo&);
        uint32_t m_lIdentifier;
        float m_fCameraFar, m_fFogNear, m_fFogFar;
        ZCOLOR m_lFogColor;
        uint32_t m_lUpdateFrequency;
        uint8_t m_bCurrentRoomOnly;
        uint8_t m_PAD_B5[3];
        uint32_t m_lFacesToUpdate, m_lCurrentFace;
    };
    RE_VERIFY_SIZE(ZRenderEntryEnvSamplerD3D, 0xC0);
}
