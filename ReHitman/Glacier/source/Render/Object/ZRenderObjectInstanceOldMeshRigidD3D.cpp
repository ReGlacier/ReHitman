#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshRigidD3D.h>
#include <Glacier/Render/Object/ZRenderObjectOldMeshRigidD3D.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    void ZRenderObjectInstanceOldMeshRigidD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectOldMeshRigidD3D>();
        if (!pObject->m_VertexContainer.m_lNumVertices || !pObject->m_IndexContainer.m_pSubRanges)
        {
            return;
        }

        auto* pSharedResources = ZSharedResourcesD3D::g_pInstance;
        g_pd3dDevice->SetStreamSource(
            0u,
            pSharedResources->m_pSVB->Interface(),
            pObject->m_VertexContainer.m_lVertexOffset,
            0x24u);
        g_pd3dDevice->SetIndices(pSharedResources->m_pSIB->Interface());
        DrawIndexedStrips(
            &pObject->m_IndexContainer,
            pObject->m_VertexContainer.m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            SPrimObject::SUBTYPE_RIGID);
    }
}
