#include <Glacier/Render/Object/ZRenderObjectInstanceStandardMeshRigidD3D.h>
#include <Glacier/Render/Object/ZRenderObjectStandardMeshRigidD3D.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SVertexWintelDP3.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    ZRenderObjectInstanceStandardMeshRigidD3D::ZRenderObjectInstanceStandardMeshRigidD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom)
        : ZRenderObjectInstanceD3D(pRenderObject, pBaseGeom)
    {
    }

    ZRenderObjectInstanceStandardMeshRigidD3D::~ZRenderObjectInstanceStandardMeshRigidD3D() = default;

    void ZRenderObjectInstanceStandardMeshRigidD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectStandardMeshRigidD3D>();
        if (!pObject->m_VertexContainer.m_lNumVertices || !pObject->m_IndexContainer.m_pSubRanges)
        {
            return;
        }

        auto* pVertexBuffer = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
        g_pd3dDevice->SetStreamSource(0u, pVertexBuffer, pObject->m_VertexContainer.m_lVertexOffset, sizeof(SVertexWintelDP3));
        DrawIndexedTriangles(
            &pObject->m_IndexContainer,
            pObject->m_VertexContainer.m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            SPrimObject::SUBTYPE_RIGID);
    }
}
