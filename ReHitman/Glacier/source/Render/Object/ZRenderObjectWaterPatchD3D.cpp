#include <Glacier/Render/Object/ZRenderObjectWaterPatchD3D.h>
#include <Glacier/Render/Object/ZRenderObjectInstanceWaterPatchD3D.h>
#include <Glacier/Render/Prim/SPrimHeaderStripWaterPatch.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>

namespace Glacier
{
    namespace
    {
        struct SVertexWaterPatchXenon
        {
            float Pos[3];
            uint32_t Col;
            float Tex[2];
            float Tan[3];
            float Bin[3];
        };
        RE_VERIFY_SIZE(SVertexWaterPatchXenon, 0x30);
        RE_VERIFY_OFFSET(SVertexWaterPatchXenon, Col, 0x0C);
        RE_VERIFY_OFFSET(SVertexWaterPatchXenon, Tex, 0x10);
        RE_VERIFY_OFFSET(SVertexWaterPatchXenon, Tan, 0x18);
        RE_VERIFY_OFFSET(SVertexWaterPatchXenon, Bin, 0x24);
    }

    ZRenderObjectWaterPatchD3D::ZRenderObjectWaterPatchD3D(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : ZRenderObjectD3D(hPrim, pMaterialInstance)
        , m_pUnknown(nullptr)
        , m_pTempVertices(nullptr)
        , m_lUnknown(0)
        , m_lMode(0)
    {
        const auto* pHeader = static_cast<const SPrimHeaderStripWaterPatch*>(hPrim);
        const uint32_t lXVerts = static_cast<uint32_t>(pHeader->m_iXSegs + 1);
        const uint32_t lZVerts = static_cast<uint32_t>(pHeader->m_iZSegs + 1);
        const uint32_t lVertices = lXVerts * lZVerts;
        const uint32_t lIndices = static_cast<uint32_t>(pHeader->m_iXSegs * pHeader->m_iZSegs * 6);

        auto* pVertices = static_cast<SVertexWaterPatchXenon*>(ZUniMemory::Allocate(sizeof(SVertexWaterPatchXenon) * lVertices));
        m_pTempVertices = pVertices;
        for (uint32_t z = 0; z < lZVerts; ++z)
        {
            for (uint32_t x = 0; x < lXVerts; ++x)
            {
                SVertexWaterPatchXenon& vertex = pVertices[z * lXVerts + x] = {};
                vertex.Pos[0] = pHeader->m_vPos.x + pHeader->m_fXSize * x / pHeader->m_iXSegs;
                vertex.Pos[1] = pHeader->m_vPos.y;
                vertex.Pos[2] = pHeader->m_vPos.z + pHeader->m_fZSize * z / pHeader->m_iZSegs;
                vertex.Tex[0] = static_cast<float>(x) / pHeader->m_iXSegs;
                vertex.Tex[1] = static_cast<float>(z) / pHeader->m_iZSegs;
                vertex.Tan[0] = 1.0f;
                vertex.Bin[2] = 1.0f;
            }
        }

        m_VertexContainer.Create(lVertices, sizeof(SVertexWaterPatchXenon), ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator, 0u);
        auto* pPacked = static_cast<uint16_t*>(ZUniMemory::Allocate(sizeof(uint16_t) * (2u + lIndices)));
        pPacked[0] = 1;
        pPacked[1] = static_cast<uint16_t>(lIndices);
        uint32_t i = 0;
        for (int z = 0; z < pHeader->m_iZSegs; ++z)
        {
            for (int x = 0; x < pHeader->m_iXSegs; ++x)
            {
                const uint16_t a = static_cast<uint16_t>(z * lXVerts + x);
                const uint16_t b = static_cast<uint16_t>(a + 1);
                const uint16_t c = static_cast<uint16_t>(a + lXVerts);
                const uint16_t d = static_cast<uint16_t>(c + 1);
                pPacked[2 + i++] = a; pPacked[2 + i++] = c; pPacked[2 + i++] = b;
                pPacked[2 + i++] = b; pPacked[2 + i++] = c; pPacked[2 + i++] = d;
            }
        }
        m_IndexContainer.Create(pPacked, ZSharedResourcesD3D::g_pInstance->m_pIndexAllocator, 0u);
        auto* pIndexBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSIB->LockedData());
        if (pIndexBuffer && m_IndexContainer.m_pSubRanges)
        {
            memcpy(pIndexBuffer + m_IndexContainer.m_pSubRanges[0].m_lIndexOffset * sizeof(uint16_t), pPacked + 2, sizeof(uint16_t) * lIndices);
        }
        ZUniMemory::Free(pPacked);
        auto* pVertexBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
        if (pVertexBuffer && m_VertexContainer.m_lNumVertices)
        {
            memcpy(pVertexBuffer + m_VertexContainer.m_lVertexOffset, pVertices, sizeof(SVertexWaterPatchXenon) * lVertices);
        }
        ZUniMemory::Free(pVertices);
        m_pTempVertices = nullptr;
    }

    ZRenderObjectWaterPatchD3D::~ZRenderObjectWaterPatchD3D()
    {
        if (m_pTempVertices) ZUniMemory::Free(m_pTempVertices);
        m_VertexContainer.Release();
        m_IndexContainer.Release();
    }

    ZRenderObjectInstance* ZRenderObjectWaterPatchD3D::CreateInstance(ZBaseGeom* pBaseGeom)
    {
        return ZUniMemory::New<ZRenderObjectInstanceWaterPatchD3D>(this, pBaseGeom, m_lMode);
    }
}
