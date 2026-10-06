#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>
#include <Glacier/Render/ZRVertexContainer.h>
#include <cstdint>


namespace Glacier
{
    struct SWaterPatchInst;

    class ZRenderObjectInstanceWaterPatchD3D : public ZRenderObjectInstanceD3D
    {
    public:
        ~ZRenderObjectInstanceWaterPatchD3D() override;
        void Draw(ZRenderContext* pContext) override;
        void UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights) override;

        ZRenderObjectInstanceWaterPatchD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom, uint8_t lMode);

        void UpdateVertices();
        void UpdateVertexLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights);

        ZRVertexContainer m_VertexContainer; // +0x18
        SWaterPatchInst* m_pWaterPatch; // +0x30
        uint8_t m_lMode; // +0x34
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceWaterPatchD3D, 0x38);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceWaterPatchD3D, m_VertexContainer, 0x18);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceWaterPatchD3D, m_pWaterPatch, 0x30);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceWaterPatchD3D, m_lMode, 0x34);
}
