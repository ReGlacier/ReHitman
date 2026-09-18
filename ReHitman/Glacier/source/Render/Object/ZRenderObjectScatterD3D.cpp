#include <Glacier/Render/Object/ZRenderObjectScatterD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceScatterD3D.h>
#include <Glacier/Render/Prim/SPrimObjectScatter.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        struct SScatterVertex
        {
            ZVector3 vPosition;
            uint32_t lColor;
            uint32_t lDistribution;
        };
        RE_VERIFY_SIZE(SScatterVertex, 0x14);

        uint32_t InterpolateByte(uint8_t a, uint8_t b, uint8_t c, float wa, float wb, float wc)
        {
            return static_cast<uint32_t>(wa * a + wb * b + wc * c);
        }
    }

    ZRenderObjectScatterD3D::ZRenderObjectScatterD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
        , m_lTotalIndexCount(0)
        , m_bIndexBufferLimited(false)
    {
        const auto* pScatterPayload = hPrim.Get<SPrimObjectScatterPayload>();
        const auto* pScatter = &pScatterPayload->sScatter;
        ZASSERT(pScatter->lMeshType == 1);

        const auto* pDistributionTypes = ZPrimHandle{pScatter->lMasterPrim}.Get<SPrimScatterDistributionType>();
        const auto* pDistributionBytes = ZPrimHandle{pScatter->lDistributionPrim}.Get<uint8_t>();
        const auto* pDistributionItems = reinterpret_cast<const SPrimScatterDistributionItem*>(
            pDistributionTypes + 8);

        uint32_t lMaxItemsPerTile = 0;
        for (uint32_t i = 0; i < 8; ++i)
        {
            const uint32_t lItems = pDistributionTypes[i].lCount[0] + pDistributionTypes[i].lCount[1];
            if (lMaxItemsPerTile < lItems)
            {
                lMaxItemsPerTile = lItems;
            }
        }

        const uint64_t lMaxItems = static_cast<uint64_t>(lMaxItemsPerTile)
            * pScatter->lTileX * pScatter->lTileZ;
        const uint64_t lVertices = lMaxItems * 4u;
        const uint64_t lIndices = lMaxItems * 6u;

        if (lVertices > 0xFFFFFFFFu || lIndices > 0xFFFFFFFFu || lVertices > 0xFFFFu || lIndices > 0xFFFFu)
        {
            m_bIndexBufferLimited = true;
            return;
        }

        m_VertexContainer.Create(
            static_cast<uint32_t>(lVertices),
            0x14u,
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator,
            0u);

        auto* pVertexBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
        auto* pVertex = pVertexBuffer
            ? reinterpret_cast<SScatterVertex*>(pVertexBuffer + m_VertexContainer.m_lVertexOffset)
            : nullptr;
        uint32_t lDistributionItemCount = 0;
        const SPrimScatterTile* pTile = pScatterPayload->aTiles;
        for (uint32_t z = 0; z < pScatter->lTileZ; ++z)
        {
            for (uint32_t x = 0; x < pScatter->lTileX; ++x, ++pTile)
            {
                const uint32_t lType = pTile->lDistributionType;
                if ((lType & 8u) != 0)
                {
                    continue;
                }

                const auto& rType = pDistributionTypes[lType & 7u];
                const auto* pItem = pDistributionItems + rType.lPayloadOffset;
                const uint8_t* pPackedDistribution = pDistributionBytes + rType.lPayloadOffset;
                for (uint32_t edge = 0; edge < 2; ++edge)
                {
                    const uint32_t lRowStride = pScatter->lTileX + 1u;
                    const SPrimScatterTile& a = pTile[edge ? lRowStride : 0u];
                    const SPrimScatterTile& b = pTile[lRowStride + (edge ? 1u : 0u)];
                    const SPrimScatterTile& c = pTile[1u];
                    for (uint32_t i = 0; i < rType.lCount[edge]; ++i, ++pItem, ++pPackedDistribution)
                    {
                        const float wc = pItem->fBarycentricX;
                        const float wb = pItem->fBarycentricY;
                        const float wa = 1.0f - wb - wc;
                        const float fRandom = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__);
                        const float fRed = pScatter->vColA.x + (pScatter->vColB.x - pScatter->vColA.x) * fRandom;
                        const float fGreen = pScatter->vColA.y + (pScatter->vColB.y - pScatter->vColA.y) * fRandom;
                        const float fBlue = pScatter->vColA.z + (pScatter->vColB.z - pScatter->vColA.z) * fRandom;
                        const uint32_t lAlpha = InterpolateByte(a.lColor[0], b.lColor[0], c.lColor[0], wa, wb, wc);
                        const uint32_t lRed = static_cast<uint32_t>(
                            InterpolateByte(a.lColor[1], b.lColor[1], c.lColor[1], wa, wb, wc) * fRed);
                        const uint32_t lGreen = static_cast<uint32_t>(
                            InterpolateByte(a.lColor[2], b.lColor[2], c.lColor[2], wa, wb, wc) * fGreen);
                        const uint32_t lBlue = static_cast<uint32_t>(
                            InterpolateByte(a.lColor[3], b.lColor[3], c.lColor[3], wa, wb, wc) * fBlue);
                        const uint32_t lColor = (lAlpha << 24) | (lRed << 16) | (lGreen << 8) | lBlue;
                        const uint32_t lDistribution =
                            (static_cast<uint32_t>(pItem->fAlpha * 255.0f)
                                | static_cast<uint32_t>(*pPackedDistribution & 0xFCu) << 6
                                | static_cast<uint32_t>(*pPackedDistribution & 3u) << 16)
                            << 8;

                        const ZVector3 vPosition =
                        {
                            wa * a.vPosition.x + wb * b.vPosition.x + wc * c.vPosition.x,
                            wa * a.vPosition.y + wb * b.vPosition.y + wc * c.vPosition.y,
                            wa * a.vPosition.z + wb * b.vPosition.z + wc * c.vPosition.z,
                        };
                        if (pVertex)
                        {
                            for (uint32_t corner = 0; corner < 4; ++corner, ++pVertex)
                            {
                                pVertex->vPosition = vPosition;
                                pVertex->lColor = lColor;
                                pVertex->lDistribution = lDistribution | corner;
                            }
                        }
                        ++lDistributionItemCount;
                    }
                }
            }
            ++pTile;
        }

        m_lTotalIndexCount = lDistributionItemCount * 6u;

        if (m_lTotalIndexCount == 0)
        {
            return;
        }

        const uint32_t lPackedWords = 1u + 1u + m_lTotalIndexCount;
        auto* pPackedIndices = static_cast<uint16_t*>(
            ZUniMemory::Allocate(sizeof(uint16_t) * lPackedWords));
        pPackedIndices[0] = 1;
        pPackedIndices[1] = static_cast<uint16_t>(m_lTotalIndexCount);
        for (uint32_t i = 0; i < lDistributionItemCount; ++i)
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
