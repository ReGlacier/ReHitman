#include <Glacier/Render/Object/ZRenderObjectStaticShadowMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceStaticShadowMeshD3D.h>
#include <Glacier/Render/Prim/SPrimStaticShadowMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SVertexStaticShadowXBox.h>
#include <Glacier/Render/Prim/SVertexStaticShadowWintel.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectStaticShadowMeshD3D::ZRenderObjectStaticShadowMeshD3D(
        const ZPrimHandle& hPrim,
        ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
        , m_Unk10(0)
    {
        const SPrimStaticShadowMesh* pMesh = hPrim;
        const uint32_t* pSubMeshTable = ZPrimHandle{pMesh->lSubMeshTable};
        ZASSERT(pSubMeshTable[0]);
        const SPrimSubMesh* pSubMesh = ZPrimHandle{pSubMeshTable[0]};

        m_VertexContainer.Create(
            pSubMesh->lNumVertices,
            sizeof(SVertexStaticShadowWintel),
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

        if (m_VertexContainer.m_lNumVertices)
        {
            const void* pVertices = ZPrimHandle{pSubMesh->lVertices}.Get<void>();
            auto* pBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
            if (pBuffer)
            {
                const auto* pSource = static_cast<const SVertexStaticShadowXBox*>(pVertices);
                auto* pDestination = reinterpret_cast<SVertexStaticShadowWintel*>(pBuffer + m_VertexContainer.m_lVertexOffset);
                for (uint32_t i = 0; i < m_VertexContainer.m_lNumVertices; ++i)
                {
                    const SVertexStaticShadowXBox& source = pSource[i];
                    SVertexStaticShadowWintel& destination = pDestination[i];
                    destination.p = ZVector3(
                        static_cast<float>(source.x) * (1.0f / 256.0f),
                        static_cast<float>(source.y) * (1.0f / 256.0f),
                        static_cast<float>(source.z) * (1.0f / 256.0f));
                    destination.c = 0xFF000000u
                        | (static_cast<uint32_t>(source.c) << 16)
                        | (static_cast<uint32_t>(source.c) << 8)
                        | source.c;
                }
            }
        }
    }

    ZRenderObjectStaticShadowMeshD3D::~ZRenderObjectStaticShadowMeshD3D()
    {
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectStaticShadowMeshD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceStaticShadowMeshD3D>(this, pBaseGeom);
    }
}
