#include <Glacier/Render/Object/ZRenderObjectInstanceOldMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObjectOldMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/SVertexWintel.h>
#include <Glacier/Render/Prim/SVertexColorD3D.h>
#include <Glacier/Render/Prim/SD3DRenderVertex.h>
#include <Glacier/Render/Light/ZRenderSoftwareLight.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
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
        auto* pRender = reinterpret_cast<ZRenderWintelD3D*>(pContext->m_pRender);
        auto* pObject = RenderObject<ZRenderObjectOldMeshD3D>();
        if (!(pObject->m_VertexContainer.m_lNumVertices && pObject->m_IndexContainer.m_pSubRanges))
        {
            return;
        }

        const SPrimMesh* pMesh = pObject->m_hPrim;
        if (pMesh->lType != EPrimType::PTSTRIPBONES)
        {
            // Not PTSTRIPBONES here
            const uint32_t lSubMeshTable = pMesh->lSubMeshTable;
            const uint32_t* pSubMeshTable = ZPrimHandle{lSubMeshTable};
            const uint32_t lSubMesh = pSubMeshTable[0];
            const SPrimSubMesh* pSubMesh = ZPrimHandle{lSubMesh};

            // TODO: Finish me
            // Field +0x84 of ZRenderEntry as float? Or... idk
        }
        else
        {
            // PTSTRIPBONES here
            auto* pVB0 = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
            g_pd3dDevice->SetStreamSource(0u, pVB0, pObject->m_VertexContainer.m_lVertexOffset, sizeof(SD3DRenderVertex));

            auto* pVB1 = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
            g_pd3dDevice->SetStreamSource(1u, pVB0, m_VertexContrainer.m_lVertexOffset, 0x4); // ???
        }

        ZRenderObjectInstanceD3D::DrawIndexedTriangles(&pObject->m_IndexContainer, pObject->m_VertexContainer.m_lNumVertices, pRender, 0);
    }

    void ZRenderObjectInstanceOldMeshD3D::UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights)
    {
        auto* pObject = RenderObject<ZRenderObjectOldMeshD3D>();

        if (m_VertexContrainer.m_lNumVertices && pObject->m_IndexContainer.m_pSubRanges)
        {
            auto* pBuffer = reinterpret_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
            const SPrimMesh* pMesh = pObject->m_hPrim;
            const uint32_t lSubTable = pMesh->lSubMeshTable;
            const uint32_t* pSubTable = ZPrimHandle{lSubTable};
            const uint32_t lSubMesh = pSubTable[0];
            const SPrimSubMesh* pSubMesh = ZPrimHandle{lSubMesh};
            const uint32_t lVertices = pSubMesh->lVertices;
            const SVertexWintel* pVertices = ZPrimHandle{lVertices};

            CalcLightVertices(
                reinterpret_cast<SVertexColorD3D*>(&pBuffer[m_VertexContrainer.m_lVertexOffset]),
                pVertices,
                m_VertexContrainer.m_lNumVertices,
                pUpdateLightData,
                lNumLights,
                pMesh->lDrawMode
            );
        }
    }

    void ZRenderObjectInstanceOldMeshD3D::CalcLightVertices(SVertexColorD3D* pDstVertices, const SVertexWintel* pSrcVertices, uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode)
    {
        if (lNumVertices)
        {
            const bool bLit = (lDrawMode & 0x80u) != 0u;
            do
            {
                uint32_t lCount = 32;
                if (lNumVertices <= 32)
                {
                    lCount = lNumVertices;
                }
                lNumVertices -= lCount;

                if (bLit)
                {
                    UnpackVertices(pSrcVertices, lCount, lDrawMode);
                    ZRenderSoftwareLight::CalcLight(lCount, pLights, lNumLights, lDrawMode, true);
                    PackVertices(pDstVertices, pSrcVertices, lCount);
                }
                else
                {
                    for (uint32_t i = 0; i < lCount; ++i)
                    {
                        pDstVertices[i].c = (pSrcVertices[i].c >> 1) & 0x7F7F7F7Fu;
                    }
                }

                pSrcVertices += lCount;
                pDstVertices += lCount;
            }
            while (lNumVertices);
        }
    }

    void ZRenderObjectInstanceOldMeshD3D::UnpackVertices(const SVertexWintel* pSrcVertices, uint32_t lNumVertices, uint32_t lDrawMode)
    {
        // lDrawMode is unused in the original code
        auto& arrays = ZRenderSoftwareLight::m_SSEArrays;
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            const SVertexWintel& vertex = pSrcVertices[i];

            // Unpack D3DCOLOR, all channels are halved (matches the unlit path shift)
            arrays.aSrcCA[i] = static_cast<float>((vertex.c >> 24) & 0xFFu) * (1.0f / 255.0f) * 0.5f;
            arrays.aSrcCR[i] = static_cast<float>((vertex.c >> 16) & 0xFFu) * (1.0f / 255.0f) * 0.5f;
            arrays.aSrcCG[i] = static_cast<float>((vertex.c >> 8) & 0xFFu) * (1.0f / 255.0f) * 0.5f;
            arrays.aSrcCB[i] = static_cast<float>(vertex.c & 0xFFu) * (1.0f / 255.0f) * 0.5f;

            arrays.aSrcPX[i] = vertex.p.x;
            arrays.aSrcPY[i] = vertex.p.y;
            arrays.aSrcPZ[i] = vertex.p.z;

            arrays.aSrcNX[i] = vertex.n.x;
            arrays.aSrcNY[i] = vertex.n.y;
            arrays.aSrcNZ[i] = vertex.n.z;
        }
    }

    void ZRenderObjectInstanceOldMeshD3D::PackVertices(SVertexColorD3D* pDstVertices, const SVertexWintel* pSrcVertices, uint32_t lNumVertices)
    {
        // pSrcVertices is unused in the original code (alpha comes from the halved source color in m_SSEArrays)
        const auto& arrays = ZRenderSoftwareLight::m_SSEArrays;
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            pDstVertices[i].c = static_cast<uint32_t>(arrays.aDstCB[i] * 255.0 + 0.5)
                | (static_cast<uint32_t>(arrays.aDstCG[i] * 255.0 + 0.5) << 8)
                | (static_cast<uint32_t>(arrays.aDstCR[i] * 255.0 + 0.5) << 16)
                | (static_cast<uint32_t>(arrays.aSrcCA[i] * 255.0 + 0.5) << 24);
        }
    }
}