#include <Glacier/Render/Object/ZRenderObjectInstanceStandardMeshWeightedD3D.h>
#include <Glacier/Render/Object/ZRenderObjectStandardMeshWeightedD3D.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SVertexW4WintelDP3.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    ZRenderObjectInstanceStandardMeshWeightedD3D::ZRenderObjectInstanceStandardMeshWeightedD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom)
        : ZRenderObjectInstanceD3D(pRenderObject, pBaseGeom)
    {
    }

    ZRenderObjectInstanceStandardMeshWeightedD3D::~ZRenderObjectInstanceStandardMeshWeightedD3D() = default;

    void ZRenderObjectInstanceStandardMeshWeightedD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectStandardMeshWeightedD3D>();
        if (!pObject->m_VertexContainer.m_lNumVertices || !pObject->m_IndexContainer.m_pSubRanges)
        {
            return;
        }

        auto* pVertexBuffer = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
        g_pd3dDevice->SetStreamSource(0u, pVertexBuffer, pObject->m_VertexContainer.m_lVertexOffset, sizeof(SVertexW4WintelDP3));
        DrawIndexedTriangles(
            &pObject->m_IndexContainer,
            pObject->m_VertexContainer.m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            SPrimObject::SUBTYPE_WEIGHTED);
    }
}
