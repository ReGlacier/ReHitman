#include <Glacier/Render/Object/ZRenderObjectInstanceStandardMeshD3D.h>
#include <Glacier/Render/Object/ZRenderObjectStandardMeshD3D.h>
#include <Glacier/Render/Light/ZRenderSoftwareLight.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/SPrimSubMesh.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SVertexColorD3D.h>
#include <Glacier/Render/Prim/SVertexWintelDP3.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        float UnpackNormalComponent(uint32_t lComponent)
        {
            return static_cast<float>(lComponent) * (1.0f / 127.5f) - 1.0f;
        }
    }

    ZRenderObjectInstanceStandardMeshD3D::ZRenderObjectInstanceStandardMeshD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom)
        : ZRenderObjectInstanceD3D(pRenderObject, pBaseGeom),
          m_bLightUpdated(false)
    {
        const SPrimMesh* pMesh = pRenderObject->m_hPrim;
        const uint32_t* pSubMeshTable = ZPrimHandle{pMesh->lSubMeshTable};
        ZASSERT(pSubMeshTable[0]);
        const SPrimSubMesh* pSubMesh = ZPrimHandle{pSubMeshTable[0]};

        m_VertexContainer.Create(
            pSubMesh->lNumVertices * pMesh->lNumFrames,
            sizeof(SVertexColorD3D),
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator,
            0u);
    }

    ZRenderObjectInstanceStandardMeshD3D::~ZRenderObjectInstanceStandardMeshD3D()
    {
        m_VertexContainer.Release();
    }

    void ZRenderObjectInstanceStandardMeshD3D::Draw(ZRenderContext* pCtx)
    {
        auto* pObject = RenderObject<ZRenderObjectStandardMeshD3D>();
        if (!pObject->m_VertexContainer.m_lNumVertices || !pObject->m_IndexContainer.m_pSubRanges)
        {
            return;
        }

        auto* pVertexBuffer = ZSharedResourcesD3D::g_pInstance->m_pSVB->Interface();
        g_pd3dDevice->SetStreamSource(0u, pVertexBuffer, pObject->m_VertexContainer.m_lVertexOffset, sizeof(SVertexWintelDP3));
        g_pd3dDevice->SetStreamSource(1u, pVertexBuffer, m_VertexContainer.m_lVertexOffset, sizeof(SVertexColorD3D));
        DrawIndexedTriangles(
            &pObject->m_IndexContainer,
            pObject->m_VertexContainer.m_lNumVertices,
            reinterpret_cast<ZRenderWintelD3D*>(pCtx->m_pRender),
            SPrimObject::SUBTYPE_STANDARD);
    }

    void ZRenderObjectInstanceStandardMeshD3D::UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights)
    {
        if (!m_VertexContainer.m_lNumVertices)
        {
            return;
        }

        auto* pSharedResources = ZSharedResourcesD3D::g_pInstance;
        auto* pBuffer = static_cast<uint8_t*>(pSharedResources->m_pSVB->LockedData());
        if (!pBuffer)
        {
            return;
        }

        const SPrimMesh* pMesh = m_pRenderObject->m_hPrim;
        const uint32_t* pSubMeshTable = ZPrimHandle{pMesh->lSubMeshTable};
        ZASSERT(pSubMeshTable[0]);
        const SPrimSubMesh* pSubMesh = ZPrimHandle{pSubMeshTable[0]};
        const SVertexWintelDP3* pVertices = ZPrimHandle{pSubMesh->lVertices};

        CalcLightVertices(
            reinterpret_cast<SVertexColorD3D*>(pBuffer + m_VertexContainer.m_lVertexOffset),
            pVertices,
            m_VertexContainer.m_lNumVertices,
            pUpdateLightData,
            lNumLights,
            pMesh->lDrawMode);
        m_bLightUpdated = true;
    }

    void ZRenderObjectInstanceStandardMeshD3D::CalcLightVertices(SVertexColorD3D* pDstVertices, const SVertexWintelDP3* pSrcVertices, uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode)
    {
        const bool bLit = (lDrawMode & 0x80u) != 0u;
        while (lNumVertices)
        {
            const uint32_t lCount = lNumVertices < 32u ? lNumVertices : 32u;
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
    }

    void ZRenderObjectInstanceStandardMeshD3D::UnpackVertices(const SVertexWintelDP3* pSrcVertices, uint32_t lNumVertices, uint32_t lDrawMode)
    {
        (void)lDrawMode;

        auto& arrays = ZRenderSoftwareLight::m_SSEArrays;
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            const SVertexWintelDP3& vertex = pSrcVertices[i];
            arrays.aSrcCA[i] = static_cast<float>((vertex.c >> 24) & 0xFFu) * (1.0f / 510.0f);
            arrays.aSrcCR[i] = static_cast<float>((vertex.c >> 16) & 0xFFu) * (1.0f / 510.0f);
            arrays.aSrcCG[i] = static_cast<float>((vertex.c >> 8) & 0xFFu) * (1.0f / 510.0f);
            arrays.aSrcCB[i] = static_cast<float>(vertex.c & 0xFFu) * (1.0f / 510.0f);

            arrays.aSrcPX[i] = vertex.p.x;
            arrays.aSrcPY[i] = vertex.p.y;
            arrays.aSrcPZ[i] = vertex.p.z;
            arrays.aSrcNX[i] = UnpackNormalComponent((vertex.n >> 16) & 0xFFu);
            arrays.aSrcNY[i] = UnpackNormalComponent((vertex.n >> 8) & 0xFFu);
            arrays.aSrcNZ[i] = UnpackNormalComponent(vertex.n & 0xFFu);
        }
    }

    void ZRenderObjectInstanceStandardMeshD3D::PackVertices(SVertexColorD3D* pDstVertices, const SVertexWintelDP3* pSrcVertices, uint32_t lNumVertices)
    {
        (void)pSrcVertices;

        const auto& arrays = ZRenderSoftwareLight::m_SSEArrays;
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            pDstVertices[i].c = static_cast<uint32_t>(arrays.aDstCB[i] * 255.0f + 0.5f)
                | (static_cast<uint32_t>(arrays.aDstCG[i] * 255.0f + 0.5f) << 8)
                | (static_cast<uint32_t>(arrays.aDstCR[i] * 255.0f + 0.5f) << 16)
                | (static_cast<uint32_t>(arrays.aSrcCA[i] * 255.0f + 0.5f) << 24);
        }
    }
}
