#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>
#include <Glacier/Render/ZRVertexContainer.h>


namespace Glacier
{
    class ZRenderObjectInstanceOldMeshD3D : public ZRenderObjectInstanceD3D
    {
    public:
        // vtbl
        ~ZRenderObjectInstanceOldMeshD3D() override;
        void Draw(ZRenderContext* pCtx) override;
        void UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights) override;

        // methods
        ZRenderObjectInstanceOldMeshD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom);

        // members
        ZRVertexContainer m_VertexContrainer;
    };
    RE_VERIFY_SIZE(ZRenderObjectInstanceOldMeshD3D, 0x30); // Verified PC ZRenderObjectOldMeshD3D::CreateInstance
}