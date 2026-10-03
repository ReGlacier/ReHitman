#include <Glacier/Render/Object/ZRenderObjectInstanceStaticShadowMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObjectStaticShadowMeshD3D.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SVertexStaticShadowWintel.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>


namespace Glacier
{
    ZRenderObjectInstanceStaticShadowMeshD3D::ZRenderObjectInstanceStaticShadowMeshD3D(
        ZRenderObject* pRenderObject,
        ZBaseGeom* pBaseGeom)
        : ZRenderObjectInstanceD3D(pRenderObject, pBaseGeom)
    {
    }

    ZRenderObjectInstanceStaticShadowMeshD3D::~ZRenderObjectInstanceStaticShadowMeshD3D() = default;

    void ZRenderObjectInstanceStaticShadowMeshD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectStaticShadowMeshD3D>();
        if (!pObject->m_VertexContainer.m_lNumVertices || !pObject->m_IndexContainer.m_pSubRanges)
        {
            return;
        }

        auto* pVertexBuffer = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
        g_pd3dDevice->SetStreamSource(
            0u,
            pVertexBuffer,
            pObject->m_VertexContainer.m_lVertexOffset,
            sizeof(SVertexStaticShadowWintel));
        g_pd3dDevice->SetIndices(ZSharedResourcesD3D::g_pInstance->m_pSIB->Interface());
        DrawIndexedTriangles(
            &pObject->m_IndexContainer,
            pObject->m_VertexContainer.m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            SPrimObject::SUBTYPE_STANDARD);
    }
}
