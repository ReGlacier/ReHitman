#include <Glacier/Render/Object/ZRenderObjectOldMeshWeightedD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshWeightedD3D.h>
#include <Glacier/Render/Prim/SVertexW4Wintel.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectOldMeshWeightedD3D::ZRenderObjectOldMeshWeightedD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
    {
        const SPrimMesh* pMesh = hPrim;
        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstMesh = pSubMeshTable[0];
        ZASSERT(lFirstMesh);
        
        const SPrimSubMesh* pSubMesh = ZPrimHandle{lFirstMesh};

        // Init vertices storage
        const uint32_t lVertices = pSubMesh->lVertices;
        const uint32_t* pVertices = ZPrimHandle{lVertices};

        RE_VERIFY_SIZE(SVertexW4Wintel, 0x34); // Twice

        m_VertexContainer.Create(
            pSubMesh->lNumVertices, 
            sizeof(SVertexW4Wintel), // 0x34
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator, 
            0);

        if (m_VertexContainer.m_lNumVertices)
        {
            memcpy(
                reinterpret_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData()) + m_VertexContainer.m_lVertexOffset,
                pVertices,
                sizeof(SVertexW4Wintel) * pSubMesh->lNumVertices
            );
        }

        // Init indices storage
        const uint32_t lIndices = pSubMesh->lIndices;
        const uint16_t* pIndices = ZPrimHandle{lIndices};

        m_IndexContainer.Create(pIndices, ZSharedResourcesD3D::g_pInstance->m_pIndexAllocator, pSubMesh->lNumIndices);
        if (m_IndexContainer.m_pSubRanges)
        {
            // Copy indices by subranges
            auto* pBuffer = reinterpret_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSIB->LockedData());
            const uint16_t* pStream = pIndices + 1;

            for (uint32_t i = 0; i < m_IndexContainer.m_lSubRangeCount; ++i)
            {
                const uint32_t lIndexCount = *pStream;
                const ZRIndexSubRange* pSubRange = &m_IndexContainer.m_pSubRanges[i];
                ZASSERT(pSubRange->m_lIndexCount == lIndexCount);

                const uint16_t* pSrcIndices = pStream + 1;
                auto* pDstIndices = reinterpret_cast<uint16_t*>(pBuffer + sizeof(uint16_t) * pSubRange->m_lIndexOffset);

                if (lIndexCount)
                {
                    memcpy(pDstIndices, pSrcIndices, sizeof(uint16_t) * lIndexCount);
                }

                pStream = pSrcIndices + lIndexCount;
            }
        }
    }

    ZRenderObjectOldMeshWeightedD3D::~ZRenderObjectOldMeshWeightedD3D()
    {
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectOldMeshWeightedD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceOldMeshWeightedD3D>(this, pBaseGeom);
    }
}