#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObjectOldMeshD3D.h>
#include <Glacier/Render/Prim/SD3DRenderVertex.h>
#include <Glacier/Render/Prim/SVertexWintel.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectOldMeshD3D::ZRenderObjectOldMeshD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
    {
        const SPrimMesh* pMesh = hPrim;
        ZASSERT(pMesh->lType == EPrimType::PTMESH);
        ZASSERT(pMesh->lSubMeshTable);

        const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
        const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
        const uint32_t lFirstMesh = pSubMeshTable[0];
        const SPrimSubMesh* pSubMesh = ZPrimHandle{lFirstMesh};
        const uint32_t lVertices = pSubMesh->lVertices;
        const uint32_t* pVertices = ZPrimHandle{lVertices};       

        RE_VERIFY_SIZE(SD3DRenderVertex, 0x20);
        const uint32_t lNumVertices = pSubMesh->lNumVertices * pMesh->lNumFrames;
        m_VertexContainer.Create(
            lNumVertices,
            sizeof(SD3DRenderVertex), 
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator, 
            0
        );

        if (m_VertexContainer.m_lNumVertices)
        {
            // Copy veritces with small transform (remove field 'c' (color) from final vertex due color computed in shaders)
            auto* pLockedData = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
            auto* pDstBuffer = reinterpret_cast<SD3DRenderVertex*>(pLockedData + m_VertexContainer.m_lVertexOffset);
            auto* pSrcVertices = reinterpret_cast<const SVertexWintel*>(pVertices);

            for (int i = 0; i < lNumVertices; ++i)
            {
                pDstBuffer[i].p = pSrcVertices[i].p;
                pDstBuffer[i].n = pSrcVertices[i].n;
                pDstBuffer[i].t = pSrcVertices[i].t;
                // Don't copy SVertexWintel::c field, it's ok
            }
        }

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

    ZRenderObjectOldMeshD3D::~ZRenderObjectOldMeshD3D()
    {
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectOldMeshD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceOldMeshD3D>(this, pBaseGeom);
    }
}