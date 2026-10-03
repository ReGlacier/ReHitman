#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>
#include <Glacier/Render/ZRVertexContainer.h>


namespace Glacier
{
    struct SVertexColorD3D;
    struct SVertexWintelDP3;

    class ZRenderObjectInstanceStandardMeshD3D : public ZRenderObjectInstanceD3D
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceStandardMeshD3D() override;
        void Draw(ZRenderContext* pCtx) override;
        void UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights) override;

        // methods
        ZRenderObjectInstanceStandardMeshD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom);
        void CalcLightVertices(SVertexColorD3D* pDstVertices, const SVertexWintelDP3* pSrcVertices, uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode);
        void UnpackVertices(const SVertexWintelDP3* pSrcVertices, uint32_t lNumVertices, uint32_t lDrawMode);
        void PackVertices(SVertexColorD3D* pDstVertices, const SVertexWintelDP3* pSrcVertices, uint32_t lNumVertices);

        // members
        ZRVertexContainer m_VertexContainer; // +0x18
        bool m_bLightUpdated; // +0x30
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceStandardMeshD3D, 0x34);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceStandardMeshD3D, m_VertexContainer, 0x18);
    RE_VERIFY_OFFSET(ZRenderObjectInstanceStandardMeshD3D, m_bLightUpdated, 0x30);
}
