#include <Glacier/Render/Object/ZRenderObjectInstanceFurD3D.h>
#include <Glacier/Render/Object/ZRenderObjectFurD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/Render/Prim/SVertexW4Wintel.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    ZRenderObjectInstanceFurD3D::ZRenderObjectInstanceFurD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom)
        : ZRenderObjectInstanceD3D(pRenderObject, pBaseGeom)
    {
    }

    ZRenderObjectInstanceFurD3D::~ZRenderObjectInstanceFurD3D() = default;

    void ZRenderObjectInstanceFurD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectFurD3D>();
        const uint32_t lLayer = pObject->m_pMaterialInstance->m_pMaterialSubClass->m_lLayer;
        const ZRIndexContainer* pIndexContainer = nullptr;
        const ZRVertexContainer* pVertexContainer = nullptr;

        switch (lLayer)
        {
        case 1u:
            pIndexContainer = &pObject->m_MainIndexContainer;
            pVertexContainer = &pObject->m_MainVertexContainer;
            break;
        case 14u:
            pIndexContainer = &pObject->m_FinIndexContainer;
            pVertexContainer = &pObject->m_FinVertexContainer;
            break;
        case 15u:
            pIndexContainer = &pObject->m_ShellIndexContainer;
            pVertexContainer = &pObject->m_ShellVertexContainer;
            break;
        default:
            return;
        }

        if (!pVertexContainer->m_lNumVertices || !pIndexContainer->m_pSubRanges)
        {
            return;
        }

        auto* pVertexBuffer = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
        g_pd3dDevice->SetStreamSource(0u, pVertexBuffer, pVertexContainer->m_lVertexOffset, sizeof(SVertexW4Wintel));
        DrawIndexedTriangles(
            pIndexContainer,
            pVertexContainer->m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            3u
        );
    }
}
