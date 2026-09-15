#include <Glacier/Render/Object/ZRenderObjectFurD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceFurD3D.h>
#include <Glacier/Render/Prim/SPrimMeshWeightedFur.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SVertexW4Wintel.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectFurD3D::ZRenderObjectFurD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
        , m_Unk10(0u)
        , m_Unk48(0u)
        , m_Unk80(0u)
        , m_Unk84(0u)
    {
        const SPrimMeshWeightedFur* pMesh = hPrim;
        const uint32_t* pSubMeshTable = ZPrimHandle{pMesh->lSubMeshTable};

        auto upload = [](const SPrimSubMesh* pSubMesh, ZRIndexContainer& indexContainer, ZRVertexContainer& vertexContainer)
        {
            vertexContainer.Create(
                pSubMesh->lNumVertices,
                sizeof(SVertexW4Wintel),
                ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator,
                0u
            );

            const void* pVertices = ZPrimHandle{pSubMesh->lVertices}.Get<void>();
            if (pSubMesh->lNumVertices)
            {
                auto* pVertexBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
                memcpy(
                    pVertexBuffer + vertexContainer.m_lVertexOffset,
                    pVertices,
                    sizeof(SVertexW4Wintel) * pSubMesh->lNumVertices
                );
            }

            const uint16_t* pIndices = ZPrimHandle{pSubMesh->lIndices}.Get<uint16_t>();
            indexContainer.Create(
                pIndices,
                ZSharedResourcesD3D::g_pInstance->m_pIndexAllocator,
                pSubMesh->lNumIndices
            );
            if (!indexContainer.m_pSubRanges)
            {
                return;
            }

            auto* pIndexBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSIB->LockedData());
            const uint16_t* pStream = pIndices + 1;
            for (uint32_t i = 0; i < indexContainer.m_lSubRangeCount; ++i)
            {
                const uint32_t lIndexCount = *pStream;
                const ZRIndexSubRange& subRange = indexContainer.m_pSubRanges[i];
                ZASSERT(subRange.m_lIndexCount == lIndexCount);
                memcpy(
                    reinterpret_cast<uint16_t*>(pIndexBuffer) + subRange.m_lIndexOffset,
                    pStream + 1,
                    sizeof(uint16_t) * lIndexCount
                );
                pStream += lIndexCount + 1;
            }
        };

        upload(ZPrimHandle{pSubMeshTable[0]}.Get<SPrimSubMesh>(), m_MainIndexContainer, m_MainVertexContainer);
        if (pMesh->bRenderFins)
        {
            upload(ZPrimHandle{pSubMeshTable[1]}.Get<SPrimSubMesh>(), m_FinIndexContainer, m_FinVertexContainer);
        }
        if (pMesh->bRenderShells)
        {
            upload(ZPrimHandle{pSubMeshTable[2]}.Get<SPrimSubMesh>(), m_ShellIndexContainer, m_ShellVertexContainer);
        }
    }

    ZRenderObjectFurD3D::~ZRenderObjectFurD3D()
    {
        m_ShellVertexContainer.Release();
        m_ShellIndexContainer.Release();
        m_FinVertexContainer.Release();
        m_FinIndexContainer.Release();
        m_MainVertexContainer.Release();
        m_MainIndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectFurD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceFurD3D>(this, pBaseGeom);
    }
}
