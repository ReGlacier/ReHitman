#include <Glacier/Render/Object/ZRenderObjectScatterD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceScatterD3D.h>
#include <Glacier/Render/Prim/SPrimObjectScatter.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    ZRenderObjectScatterD3D::ZRenderObjectScatterD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
        , m_lTotalIndexCount(0)
        , m_bIndexBufferLimited(false)
    {
        const auto* pScatter = static_cast<const SPrimObjectScatter*>(hPrim);
        const uint32_t lDistributionItems = pScatter->lDistributionSize;
        const uint64_t lVertices = static_cast<uint64_t>(lDistributionItems) * 4u;
        const uint64_t lIndices = static_cast<uint64_t>(lDistributionItems) * 6u;

        if (lVertices > 0xFFFFFFFFu || lIndices > 0xFFFFFFFFu || lVertices > 0xFFFFu || lIndices > 0xFFFFu)
        {
            m_bIndexBufferLimited = true;
            return;
        }

        m_lTotalIndexCount = static_cast<uint32_t>(lIndices);
        m_VertexContainer.Create(
            static_cast<uint32_t>(lVertices),
            0x14u,
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator,
            0u);

        // The distribution primitive payload is not represented by a local type or accessor.
        // Keep the generated stream deterministic until that payload can be resolved.
        auto* pVertexBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
        if (pVertexBuffer && lVertices)
        {
            memset(pVertexBuffer + m_VertexContainer.m_lVertexOffset, 0, sizeof(float) * 5u * lVertices);
        }

        if (lIndices == 0)
        {
            return;
        }

        const uint32_t lPackedWords = 1u + 1u + m_lTotalIndexCount;
        auto* pPackedIndices = static_cast<uint16_t*>(
            ZUniMemory::Allocate(sizeof(uint16_t) * lPackedWords));
        pPackedIndices[0] = 1;
        pPackedIndices[1] = static_cast<uint16_t>(m_lTotalIndexCount);
        for (uint32_t i = 0; i < lDistributionItems; ++i)
        {
            const uint32_t lVertex = i * 4u;
            const uint32_t lIndex = i * 6u;
            pPackedIndices[2u + lIndex + 0u] = static_cast<uint16_t>(lVertex + 0u);
            pPackedIndices[2u + lIndex + 1u] = static_cast<uint16_t>(lVertex + 1u);
            pPackedIndices[2u + lIndex + 2u] = static_cast<uint16_t>(lVertex + 2u);
            pPackedIndices[2u + lIndex + 3u] = static_cast<uint16_t>(lVertex + 0u);
            pPackedIndices[2u + lIndex + 4u] = static_cast<uint16_t>(lVertex + 2u);
            pPackedIndices[2u + lIndex + 5u] = static_cast<uint16_t>(lVertex + 3u);
        }

        m_IndexContainer.Create(
            pPackedIndices,
            ZSharedResourcesD3D::g_pInstance->m_pIndexAllocator,
            0u);
        if (!m_IndexContainer.m_pSubRanges)
        {
            m_bIndexBufferLimited = true;
        }
        else
        {
            auto* pIndexBuffer = static_cast<uint16_t*>(ZSharedResourcesD3D::g_pInstance->m_pSIB->LockedData());
            if (pIndexBuffer)
            {
                memcpy(
                    pIndexBuffer + m_IndexContainer.m_pSubRanges[0].m_lIndexOffset,
                    pPackedIndices + 2,
                    sizeof(uint16_t) * m_lTotalIndexCount);
            }
        }
        ZUniMemory::Free(pPackedIndices);

    }

    ZRenderObjectScatterD3D::~ZRenderObjectScatterD3D()
    {
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectScatterD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceScatterD3D>(this, pBaseGeom);
    }

    void ZRenderObjectScatterD3D::Update()
    {
    }
}
