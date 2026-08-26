#include <Glacier/Render/Light/ZRenderSoftwareLight.h>
#include <Glacier/Render/Light/SUpdateLightData.h>

#include <algorithm>
#include <cmath>


namespace Glacier
{
    ZRenderSoftwareLight::SSEArrays ZRenderSoftwareLight::m_SSEArrays;

    namespace
    {
        // _mm_min_ps(_mm_max_ps(x, 0.0f), 1.0f) in the original code
        inline float Saturate(float x)
        {
            return std::min(std::max(x, 0.0f), 1.0f);
        }

        // Smoothstep polynomial used by the original code: t * t * (3.0f - t * 2.0f)
        inline float SmoothStep(float t)
        {
            return t * t * (3.0f - t * 2.0f);
        }
    }

    void ZRenderSoftwareLight::CalcLight(uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode, bool bIncludeHQLights)
    {
        // lDrawMode is passed by callers but unused in the PC implementation.
        // The original code processes m_SSEArrays four vertices per SSE pass and uses
        // the rcp_ps/rsqrt_ps approximations; exact scalar math is used here instead.
        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            m_SSEArrays.aDstCR[i] = 0.0f;
            m_SSEArrays.aDstCG[i] = 0.0f;
            m_SSEArrays.aDstCB[i] = 0.0f;
        }

        for (uint32_t l = 0; l < lNumLights; ++l)
        {
            const SUpdateLightData* pLight = &pLights[l];

            if ((pLight->lLightControl & 0x2u) && !bIncludeHQLights)
            {
                continue;
            }

            const SUpdateLightDataSSE& sse = pLight->SSE;

            switch (pLight->lType)
            {
            case 0: // Spot light
                for (uint32_t i = 0; i < lNumVertices; ++i)
                {
                    const float vToLightX = sse.vPosX[0] - m_SSEArrays.aSrcPX[i];
                    const float vToLightY = sse.vPosY[0] - m_SSEArrays.aSrcPY[i];
                    const float vToLightZ = sse.vPosZ[0] - m_SSEArrays.aSrcPZ[i];

                    const float fNdotL = std::max(vToLightX * m_SSEArrays.aSrcNX[i] + vToLightY * m_SSEArrays.aSrcNY[i] + vToLightZ * m_SSEArrays.aSrcNZ[i], 0.0f);
                    const float fInvLength = 1.0f / std::sqrt(vToLightX * vToLightX + vToLightY * vToLightY + vToLightZ * vToLightZ);
                    const float fDistance = 1.0f / fInvLength;

                    const float fCosAngle = (vToLightX * sse.vDirZAX[0] + vToLightY * sse.vDirZAY[0] + vToLightZ * sse.vDirZAZ[0]) * fInvLength;
                    const float fSpot = Saturate((-fCosAngle - sse.fCosFallOff[0]) * sse.fLConst1[0]);

                    const float fAttenuation = Saturate((sse.fFarRange[0] - fDistance) * sse.fInvFarNear[0]);
                    const float fIntensity = fAttenuation * fInvLength * fNdotL * SmoothStep(fSpot);

                    m_SSEArrays.aDstCR[i] += m_SSEArrays.aSrcCR[i] * fIntensity * sse.cRed1[0];
                    m_SSEArrays.aDstCG[i] += m_SSEArrays.aSrcCG[i] * fIntensity * sse.cGreen1[0];
                    m_SSEArrays.aDstCB[i] += m_SSEArrays.aSrcCB[i] * fIntensity * sse.cBlue1[0];
                }
                break;

            case 1: // Point light
                for (uint32_t i = 0; i < lNumVertices; ++i)
                {
                    const float vToLightX = sse.vPosX[0] - m_SSEArrays.aSrcPX[i];
                    const float vToLightY = sse.vPosY[0] - m_SSEArrays.aSrcPY[i];
                    const float vToLightZ = sse.vPosZ[0] - m_SSEArrays.aSrcPZ[i];

                    const float fNdotL = std::max(vToLightX * m_SSEArrays.aSrcNX[i] + vToLightY * m_SSEArrays.aSrcNY[i] + vToLightZ * m_SSEArrays.aSrcNZ[i], 0.0f);
                    const float fInvLength = 1.0f / std::sqrt(vToLightX * vToLightX + vToLightY * vToLightY + vToLightZ * vToLightZ);
                    const float fDistance = 1.0f / fInvLength;

                    const float fAttenuation = Saturate((sse.fFarRange[0] - fDistance) * sse.fInvFarNear[0]);
                    const float fIntensity = fAttenuation * fInvLength * fNdotL;

                    m_SSEArrays.aDstCR[i] += m_SSEArrays.aSrcCR[i] * fIntensity * sse.cRed1[0];
                    m_SSEArrays.aDstCG[i] += m_SSEArrays.aSrcCG[i] * fIntensity * sse.cGreen1[0];
                    m_SSEArrays.aDstCB[i] += m_SSEArrays.aSrcCB[i] * fIntensity * sse.cBlue1[0];
                }
                break;

            case 2: // Projector light (square falloff)
                for (uint32_t i = 0; i < lNumVertices; ++i)
                {
                    const float vToLightX = sse.vPosX[0] - m_SSEArrays.aSrcPX[i];
                    const float vToLightY = sse.vPosY[0] - m_SSEArrays.aSrcPY[i];
                    const float vToLightZ = sse.vPosZ[0] - m_SSEArrays.aSrcPZ[i];

                    const float fNdotL = std::max(vToLightX * m_SSEArrays.aSrcNX[i] + vToLightY * m_SSEArrays.aSrcNY[i] + vToLightZ * m_SSEArrays.aSrcNZ[i], 0.0f);

                    // To light space
                    const float fLightX = vToLightX * sse.vDirXAX[0] + vToLightY * sse.vDirXAY[0] + vToLightZ * sse.vDirXAZ[0];
                    const float fLightY = vToLightX * sse.vDirYAX[0] + vToLightY * sse.vDirYAY[0] + vToLightZ * sse.vDirYAZ[0];
                    const float fLightZ = vToLightX * sse.vDirZAX[0] + vToLightY * sse.vDirZAY[0] + vToLightZ * sse.vDirZAZ[0];

                    const float fInvLength = 1.0f / std::sqrt(fLightX * fLightX + fLightY * fLightY + fLightZ * fLightZ);
                    const float fDistance = 1.0f / fInvLength;

                    const float fAttenuation = Saturate((sse.fFarRange[0] - fDistance) * sse.fInvFarNear[0]);

                    const float fInvZ = 1.0f / fLightZ;
                    const float u = std::fabs(fLightX * sse.fScaleX[0] * fInvZ);
                    const float v = std::fabs(fLightY * sse.fScaleY[0] * fInvZ);
                    const float fEdge = (1.0f - Saturate((u - sse.hotpct[0]) * sse.ihotpct[0])) * (1.0f - Saturate((v - sse.hotpct[0]) * sse.ihotpct[0]));

                    const float fIntensity = fAttenuation * fInvLength * fNdotL * SmoothStep(fEdge);

                    m_SSEArrays.aDstCR[i] += m_SSEArrays.aSrcCR[i] * fIntensity * sse.cRed1[0];
                    m_SSEArrays.aDstCG[i] += m_SSEArrays.aSrcCG[i] * fIntensity * sse.cGreen1[0];
                    m_SSEArrays.aDstCB[i] += m_SSEArrays.aSrcCB[i] * fIntensity * sse.cBlue1[0];
                }
                break;

            case 3: // Directional (hemisphere) light
            case 4:
                for (uint32_t i = 0; i < lNumVertices; ++i)
                {
                    const float fFactor = (1.0f - (m_SSEArrays.aSrcNX[i] * sse.vDirZAX[0] + m_SSEArrays.aSrcNY[i] * sse.vDirZAY[0] + m_SSEArrays.aSrcNZ[i] * sse.vDirZAZ[0])) * 0.5f;

                    m_SSEArrays.aDstCR[i] += (sse.cRed2[0] * fFactor + sse.cRed1[0]) * m_SSEArrays.aSrcCR[i];
                    m_SSEArrays.aDstCG[i] += (sse.cGreen2[0] * fFactor + sse.cGreen1[0]) * m_SSEArrays.aSrcCG[i];
                    m_SSEArrays.aDstCB[i] += (sse.cBlue2[0] * fFactor + sse.cBlue1[0]) * m_SSEArrays.aSrcCB[i];
                }
                break;

            default:
                break;
            }
        }

        for (uint32_t i = 0; i < lNumVertices; ++i)
        {
            m_SSEArrays.aDstCR[i] = Saturate(m_SSEArrays.aDstCR[i]);
            m_SSEArrays.aDstCG[i] = Saturate(m_SSEArrays.aDstCG[i]);
            m_SSEArrays.aDstCB[i] = Saturate(m_SSEArrays.aDstCB[i]);
        }
    }

    uint32_t ZRenderSoftwareLight::GetUpdateLightData(SUpdateLightData* pUpdateLightData, const ZBaseGeom* pBaseGeom, const ZBaseGeom* pEnvironment)
    {
        // TODO: Finish me
        return 0u;
    }

    void ZRenderSoftwareLight::CalcLightHQ(uint32_t lNumVertices, const SUpdateLightData *pLights, int32_t lNumLights, int32_t lDrawMode, bool bIncludeHQLights)
    {
        // TODO: Finish me
    }
}
