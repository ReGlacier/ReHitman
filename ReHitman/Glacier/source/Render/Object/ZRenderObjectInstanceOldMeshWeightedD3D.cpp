#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshWeightedD3D.h>
#include <Glacier/Render/Object/ZRenderObjectOldMeshWeightedD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>
#include <Glacier/Render/Prim/SVertexW4Wintel.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    ZRenderObjectInstanceOldMeshWeightedD3D::~ZRenderObjectInstanceOldMeshWeightedD3D() = default;

    void ZRenderObjectInstanceOldMeshWeightedD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectOldMeshWeightedD3D>();
        
        if (pObject->m_VertexContainer.m_lNumVertices)
        {
            if (pObject->m_IndexContainer.m_pSubRanges)
            {
                const uint32_t lVertexOffset = pObject->m_VertexContainer.m_lVertexOffset;
                auto* pVertexBuffer = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();

                RE_VERIFY_SIZE(SVertexW4Wintel, 0x34);
                g_pd3dDevice->SetStreamSource(0, pVertexBuffer, lVertexOffset, sizeof(SVertexW4Wintel));

                DrawIndexedStrips(
                    &pObject->m_IndexContainer, 
                    pObject->m_VertexContainer.m_lNumVertices, 
                    reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender), 
                    3u  // < Constant?
                );
            }
        }
    }
}