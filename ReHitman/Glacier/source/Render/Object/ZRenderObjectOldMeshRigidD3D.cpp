#include <Glacier/Render/Object/ZRenderObjectOldMeshRigidD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshRigidD3D.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SVertexWintel.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectOldMeshRigidD3D::ZRenderObjectOldMeshRigidD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
    {
        const SPrimMesh* pMesh = hPrim;
        const uint32_t* pSubMeshTable = ZPrimHandle{pMesh->lSubMeshTable};
        ZASSERT(pSubMeshTable[0]);
        const SPrimSubMesh* pSubMesh = ZPrimHandle{pSubMeshTable[0]};
        const SVertexWintel* pVertices = ZPrimHandle{pSubMesh->lVertices};

        m_VertexContainer.Create(
            pSubMesh->lNumVertices,
            sizeof(SVertexWintel),
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator,
            0u);
        if (m_VertexContainer.m_lNumVertices)
        {
            auto* pBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
            memcpy(pBuffer + m_VertexContainer.m_lVertexOffset, pVertices, sizeof(SVertexWintel) * pSubMesh->lNumVertices);
        }

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
                memcpy(
                    reinterpret_cast<uint16_t*>(pBuffer) + subRange.m_lIndexOffset,
                    pStream + 1,
                    sizeof(uint16_t) * lIndexCount);
                pStream += lIndexCount + 1;
            }
        }
    }

    ZRenderObjectOldMeshRigidD3D::~ZRenderObjectOldMeshRigidD3D()
    {
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectOldMeshRigidD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceOldMeshRigidD3D>(this, pBaseGeom);
    }
}
