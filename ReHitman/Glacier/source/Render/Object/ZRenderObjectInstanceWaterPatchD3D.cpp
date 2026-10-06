#include <Glacier/Render/Object/ZRenderObjectInstanceWaterPatchD3D.h>
#include <Glacier/Render/Object/ZRenderObjectWaterPatchD3D.h>
#include <Glacier/Render/Prim/SPrimHeaderStripWaterPatch.h>
#include <Glacier/Render/Prim/SVertexColorD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRD3DDynamicVB.h>
#include <Glacier/Render/ZWaterManager.h>
#include <Glacier/ZUniAssert.h>
#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        ZWaterManager s_WaterManager;

        struct SVertexWaterPatchXenon
        {
            float Pos[3];
            uint32_t Col;
            float Tex[2];
            float Tan[3];
            float Bin[3];
        };
        RE_VERIFY_SIZE(SVertexWaterPatchXenon, 0x30);

        void ApplyWaterPatchSimulation(
            SVertexWaterPatchXenon& rVertex,
            const SWaterPatchInst* pWaterPatch,
            const SPrimHeaderStripWaterPatch* pHeader)
        {
            if (!pWaterPatch || !pHeader)
                return;

            constexpr float kTwoPi = 6.28318530717958647692f;
            const float x = rVertex.Pos[0] - pWaterPatch->fXPos;
            const float z = rVertex.Pos[2] - pWaterPatch->fZPos;
            float dx = 0.0f;
            float dz = 0.0f;
            float dy = 0.0f;

            for (uint32_t i = 0; i < 4; ++i)
            {
                const SWaterPatchSineWaveDesc& wave = pHeader->m_pWavesDesc[i];
                const float directionLength = std::sqrt(
                    wave.m_fWaveDir.x * wave.m_fWaveDir.x + wave.m_fWaveDir.y * wave.m_fWaveDir.y);
                if (directionLength <= 0.000001f || wave.m_fWaveLength <= 0.000001f)
                    continue;

                const float directionX = wave.m_fWaveDir.x / directionLength;
                const float directionZ = wave.m_fWaveDir.y / directionLength;
                const float waveNumber = kTwoPi / wave.m_fWaveLength;
                const float phase = waveNumber * (directionX * x + directionZ * z);
                const float sine = std::sin(phase);
                const float cosine = std::cos(phase);
                const float slope = wave.m_fWaveAmplitude * waveNumber * cosine;

                dy += wave.m_fWaveAmplitude * sine;
                dx += directionX * slope;
                dz += directionZ * slope;
            }

            for (int i = 0; i < pWaterPatch->iRipples; ++i)
            {
                const SWaterRippleInst* pRipple = pWaterPatch->ripples[i];
                if (!pRipple || pRipple->fWaveLength <= 0.000001f)
                    continue;

                const float offsetX = rVertex.Pos[0] - pRipple->fXCenter;
                const float offsetZ = rVertex.Pos[2] - pRipple->fZCenter;
                const float distance = std::sqrt(offsetX * offsetX + offsetZ * offsetZ);
                if (distance <= 0.000001f || distance > pRipple->fRadius)
                    continue;

                const float waveNumber = kTwoPi / pRipple->fWaveLength;
                const float phase = waveNumber * distance - pRipple->fPhase;
                const float falloff = 1.0f - distance * pRipple->fOneOverRadius;
                const float amplitude = pRipple->fAmplitude * falloff;
                const float sine = std::sin(phase);
                const float cosine = std::cos(phase);
                const float slope = amplitude * waveNumber * cosine
                    - pRipple->fAmplitude * pRipple->fOneOverRadius * sine;

                dy += amplitude * sine;
                dx += slope * offsetX / distance;
                dz += slope * offsetZ / distance;
            }

            rVertex.Pos[1] = pWaterPatch->fYPos + dy;
            rVertex.Tan[0] = 1.0f;
            rVertex.Tan[1] = dx;
            rVertex.Tan[2] = 0.0f;
            rVertex.Bin[0] = 0.0f;
            rVertex.Bin[1] = dz;
            rVertex.Bin[2] = 1.0f;
        }
    }

    ZRenderObjectInstanceWaterPatchD3D::ZRenderObjectInstanceWaterPatchD3D(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom, uint8_t lMode)
        : ZRenderObjectInstanceD3D(pRenderObject, pBaseGeom)
        , m_pWaterPatch(nullptr)
        , m_lMode(lMode)
    {
        const auto* pHeader = static_cast<const SPrimHeaderStripWaterPatch*>(pRenderObject->m_hPrim);
        m_VertexContainer.Create(
            static_cast<uint32_t>((pHeader->m_iXSegs + 1) * (pHeader->m_iZSegs + 1)),
            sizeof(SVertexColorD3D),
            ZSharedResourcesD3D::g_pInstance->m_pVertexAllocator,
            0u);
        m_pWaterPatch = s_WaterManager.AllocWaterPatch(pRenderObject);
        if (m_pWaterPatch)
        {
            m_pWaterPatch->iXsegs = pHeader->m_iXSegs;
            m_pWaterPatch->iZsegs = pHeader->m_iZSegs;
            m_pWaterPatch->fSegSizeX = pHeader->m_fXSize / pHeader->m_iXSegs;
            m_pWaterPatch->fSegSizeZ = pHeader->m_fZSize / pHeader->m_iZSegs;
            m_pWaterPatch->fXPos = pHeader->m_vPos.x;
            m_pWaterPatch->fYPos = pHeader->m_vPos.y;
            m_pWaterPatch->fZPos = pHeader->m_vPos.z;
            m_pWaterPatch->lGroupID = pHeader->m_lGroupID;
            for (uint32_t i = 0; i < 4; ++i)
            {
                m_pWaterPatch->pSineWaves[i].fP = pHeader->m_pWavesDesc[i].m_fWaveAmplitude;
                m_pWaterPatch->pSineWaves[i].fAmplitude = pHeader->m_pWavesDesc[i].m_fWaveAmplitude;
                m_pWaterPatch->pSineWaves[i].fQ = pHeader->m_pWavesDesc[i].m_fWaveSpeed;
                m_pWaterPatch->pSineWaves[i].fR = pHeader->m_pWavesDesc[i].m_fWaveLength;
                m_pWaterPatch->pSineWavesSpeed[i] = pHeader->m_pWavesDesc[i].m_fWaveSpeed;
                m_pWaterPatch->pSineWavesFreq[i] = pHeader->m_pWavesDesc[i].m_fWaveLength;
            }
        }
    }

    ZRenderObjectInstanceWaterPatchD3D::~ZRenderObjectInstanceWaterPatchD3D()
    {
        if (m_pWaterPatch)
            s_WaterManager.FreeWaterPatch(m_pRenderObject);
        m_VertexContainer.Release();
    }

    void ZRenderObjectInstanceWaterPatchD3D::Draw(ZRenderContext* pContext)
    {
        auto* pObject = RenderObject<ZRenderObjectWaterPatchD3D>();
        const auto* pHeader = static_cast<const SPrimHeaderStripWaterPatch*>(pObject->m_hPrim);
        if (!pObject->m_VertexContainer.m_lNumVertices || !pObject->m_IndexContainer.m_pSubRanges)
            return;
        auto* pResources = ZSharedResourcesD3D::g_pInstance;
        auto* pDynamicVB = pResources->m_pDVB;
        auto* pStaticVertexBuffer = static_cast<const SVertexWaterPatchXenon*>(pResources->m_pSVB->LockedData());
        auto* pColorBuffer = pResources->m_pSVB->Interface();
        if (!pDynamicVB || !pStaticVertexBuffer || !pColorBuffer)
            return;

        uint32_t lVertexStart = 0;
        auto* pDynamicVertices = static_cast<SVertexWaterPatchXenon*>(pDynamicVB->Lock(
            pObject->m_VertexContainer.m_lNumVertices, sizeof(SVertexWaterPatchXenon), lVertexStart));
        if (!pDynamicVertices)
            return;

        memcpy(pDynamicVertices, pStaticVertexBuffer + pObject->m_VertexContainer.m_lVertexOffset / sizeof(SVertexWaterPatchXenon),
            sizeof(SVertexWaterPatchXenon) * pObject->m_VertexContainer.m_lNumVertices);
        for (uint32_t i = 0; i < pObject->m_VertexContainer.m_lNumVertices; ++i)
            ApplyWaterPatchSimulation(pDynamicVertices[i], m_pWaterPatch, pHeader);
        pDynamicVB->Unlock();

        const uint32_t lDynamicVertexOffset = lVertexStart * sizeof(SVertexWaterPatchXenon);
        g_pd3dDevice->SetStreamSource(0u, pDynamicVB->Interface(), lDynamicVertexOffset, sizeof(SVertexWaterPatchXenon));
        g_pd3dDevice->SetStreamSource(1u, pColorBuffer, m_VertexContainer.m_lVertexOffset, sizeof(SVertexColorD3D));
        g_pd3dDevice->SetStreamSource(2u, pDynamicVB->Interface(), lDynamicVertexOffset, sizeof(SVertexWaterPatchXenon));
        DrawIndexedTriangles(&pObject->m_IndexContainer, pObject->m_IndexContainer.m_lTotalIndexCount, reinterpret_cast<ZRenderWintelD3D*>(pContext->m_pRender), 0u);
    }

    void ZRenderObjectInstanceWaterPatchD3D::UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights)
    {
        UpdateVertexLight(pUpdateLightData, lNumLights);
    }

    void ZRenderObjectInstanceWaterPatchD3D::UpdateVertices()
    {
        if (m_pWaterPatch)
            s_WaterManager.UpdateRippleInfo(m_pWaterPatch);
    }

    void ZRenderObjectInstanceWaterPatchD3D::UpdateVertexLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights)
    {
        if (!m_VertexContainer.m_lNumVertices)
            return;
        auto* pBuffer = static_cast<uint8_t*>(ZSharedResourcesD3D::g_pInstance->m_pSVB->LockedData());
        if (!pBuffer)
            return;
        auto* pDst = reinterpret_cast<SVertexColorD3D*>(pBuffer + m_VertexContainer.m_lVertexOffset);
        const uint32_t color = lNumLights && pUpdateLightData ? 0xFFFFFFFFu : 0x7F7F7F7Fu;
        for (uint32_t i = 0; i < m_VertexContainer.m_lNumVertices; ++i)
            pDst[i].c = color;
    }
}
