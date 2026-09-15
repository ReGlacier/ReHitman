#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshWeightedD3D.h>
#include <Glacier/Render/Object/ZRenderObjectOldMeshWeightedD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>
#include <Glacier/Render/Prim/SPrimObject.h>
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
        if (!(pObject->m_VertexContainer.m_lNumVertices && pObject->m_IndexContainer.m_pSubRanges))
        {
            return;
        }

        auto* pVertexBuffer = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
        g_pd3dDevice->SetStreamSource(0u, pVertexBuffer, pObject->m_VertexContainer.m_lVertexOffset, 0x34u);
        DrawIndexedStrips(
            &pObject->m_IndexContainer,
            pObject->m_VertexContainer.m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            SPrimObject::SUBTYPE_WEIGHTED
        );
    }
}
