#include <Glacier/Render/Object/ZRenderObjectInstanceScatterD3D.h>
#include <Glacier/Render/Object/ZRenderObjectScatterD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    void ZRenderObjectInstanceScatterD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectScatterD3D>();
        if (!pObject->m_VertexContainer.m_lNumVertices || !pObject->m_IndexContainer.m_pSubRanges)
        {
            return;
        }

        auto* pSharedResources = ZSharedResourcesD3D::g_pInstance;
        auto* pVertexBuffer = pSharedResources->m_pSVB->Interface();
        g_pd3dDevice->SetStreamSource(0u, pVertexBuffer, pObject->m_VertexContainer.m_lVertexOffset, 0x14u);
        g_pd3dDevice->SetIndices(pSharedResources->m_pSIB->Interface());
        DrawIndexedTriangles(
            &pObject->m_IndexContainer,
            pObject->m_VertexContainer.m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            0u);
    }
}
