#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZPrimHandle.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZRenderObjectInstanceOldMeshD3D::ZRenderObjectInstanceOldMeshD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom)
        : ZRenderObjectInstanceD3D(pRenderObject, pBaseGeom)
    {
        const auto& hPrim = pRenderObject->m_hPrim;

        const SPrimMesh* pMesh = hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstMesh = pSubMeshTable[0];
        ZASSERT(lFirstMesh);
        const SPrimSubMesh* pSubMesh = ZPrimHandle{lFirstMesh};

        m_VertexContrainer.Create(pSubMesh->lNumVertices * pMesh->lNumFrames, 4u, ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator, 0u);
    }

    ZRenderObjectInstanceOldMeshD3D::~ZRenderObjectInstanceOldMeshD3D()
    {
        // Idk why, but cleanup called twice...
        m_VertexContrainer.Release();
        m_VertexContrainer.Release();

        /**
         * Original code looks like
        auto* p = &m_VertexContrainer;
        m_VertexContrainer.Release();
        p->Release();
         */
    }

    void ZRenderObjectInstanceOldMeshD3D::Draw(ZRenderContext* pContext)
    {
        // TODO: Finish me
    }

    void ZRenderObjectInstanceOldMeshD3D::UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights)
    {
        // TODO: Finish me
    }
}