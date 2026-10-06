#include <Glacier/Render/Object/ZRenderObjectStandardMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceStandardMeshD3D.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SVertexWintelDP3.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectStandardMeshD3D::ZRenderObjectStandardMeshD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
        , m_lVertices(0)
        , m_bVerticesUploaded(false)
    {
        const SPrimMesh* pMesh = hPrim;
        const uint32_t* pSubMeshTable = ZPrimHandle{pMesh->lSubMeshTable};
        ZASSERT(pSubMeshTable[0]);
        const SPrimSubMesh* pSubMesh = ZPrimHandle{pSubMeshTable[0]};
        m_lVertices = pSubMesh->lVertices;

        m_VertexContainer.Create(
            pSubMesh->lNumVertices * pMesh->lNumFrames,
            sizeof(SVertexWintelDP3),
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator,
            0u);

        const uint16_t* pIndices = ZPrimHandle{pSubMesh->lIndices};
        m_IndexContainer.Create(pIndices, ZSharedResourcesD3D::g_pInstance->m_pIndexAllocator, pSubMesh->lNumIndices);
        if (m_IndexContainer.m_pSubRanges)
        {
            auto* pBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSIB->LockedData());
            const uint16_t* pStream = pIndices + 1;
            for (uint32_t i = 0; i < m_IndexContainer.m_lSubRangeCount; ++i)
            {
                const uint32_t lIndexCount = *pStream;
                const ZRIndexSubRange& subRange = m_IndexContainer.m_pSubRanges[i];
                ZASSERT(subRange.m_lIndexCount == lIndexCount);
                memcpy(reinterpret_cast<uint16_t*>(pBuffer) + subRange.m_lIndexOffset, pStream + 1, sizeof(uint16_t) * lIndexCount);
                pStream += lIndexCount + 1;
            }
        }
        Update();
    }

    ZRenderObjectStandardMeshD3D::~ZRenderObjectStandardMeshD3D()
    {
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectStandardMeshD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceStandardMeshD3D>(this, pBaseGeom);
    }

    void ZRenderObjectStandardMeshD3D::Update()
    {
        if (!m_VertexContainer.m_lNumVertices)
        {
            return;
        }

        const SVertexWintelDP3* pVertices = ZPrimHandle{m_lVertices};
        auto* pBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
        if (pBuffer)
        {
            memcpy(
                pBuffer + m_VertexContainer.m_lVertexOffset,
                pVertices,
                m_VertexContainer.m_lNumVertices * sizeof(SVertexWintelDP3));
            m_bVerticesUploaded = true;
        }
    }
}
