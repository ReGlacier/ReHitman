#include <Glacier/Render/Light/ZRenderSoftwareLight.h>
#include <Glacier/Render/Light/SUpdateLightData.h>

#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <Glacier/Render/Prim/SPrimLightEnvironment.h>
#include <Glacier/Render/Prim/SPrimLightOmni.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSTL/CListUser.h>
#include <Glacier/ZSTL/ZMath.h>

#include <cmath>
#include <xmmintrin.h>


namespace Glacier
{
    ZRenderSoftwareLight::SSEArrays ZRenderSoftwareLight::m_SSEArrays;

    namespace
    {
        constexpr uint32_t kMaxLights = 12;

        inline __m128 Saturate(const __m128 value)
        {
            return _mm_min_ps(_mm_max_ps(value, _mm_setzero_ps()), _mm_set1_ps(1.0f));
        }

        inline __m128 SmoothStep(const __m128 value)
        {
            return _mm_mul_ps(_mm_mul_ps(value, value), _mm_sub_ps(_mm_set1_ps(3.0f), _mm_add_ps(value, value)));
        }

        inline __m128 Abs(const __m128 value)
        {
            return _mm_and_ps(value, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
        }

        inline void AddColor(__m128& red, __m128& green, __m128& blue, const SUpdateLightDataSSE& light, const __m128 intensity)
        {
            red = _mm_add_ps(red, _mm_mul_ps(_mm_load_ps(light.cRed1), intensity));
            green = _mm_add_ps(green, _mm_mul_ps(_mm_load_ps(light.cGreen1), intensity));
            blue = _mm_add_ps(blue, _mm_mul_ps(_mm_load_ps(light.cBlue1), intensity));
        }

        inline void ConvertToSSE(SUpdateLightData& light)
        {
            const float* source[] = {
                &light.vPos[0], &light.vPos[1], &light.vPos[2],
                &light.mDir[0], &light.mDir[1], &light.mDir[2],
                &light.mDir[3], &light.mDir[4], &light.mDir[5],
                &light.mDir[6], &light.mDir[7], &light.mDir[8],
                &light.vColor1[0], &light.vColor1[1], &light.vColor1[2],
                &light.vColor2[0], &light.vColor2[1], &light.vColor2[2],
                &light.fScaleX, &light.fScaleY, &light.hotpct, &light.ihotpct,
                &light.fFarRange, &light.fInvFarNear, &light.fCosFallOff, &light.fLConst1
            };
            float* destination = reinterpret_cast<float*>(&light.SSE);
            for (uint32_t i = 0; i < 26; ++i)
            {
                _mm_store_ps(destination + i * 4, _mm_set1_ps(*source[i]));
            }
        }

        inline void DecodeColor(float (&color)[3], uint32_t packedColor, float multiplier)
        {
            constexpr float kByteToFloat = 1.0f / 255.0f;
            color[0] = static_cast<float>((packedColor >> 16) & 0xFFu) * multiplier * kByteToFloat;
            color[1] = static_cast<float>((packedColor >> 8) & 0xFFu) * multiplier * kByteToFloat;
            color[2] = static_cast<float>(packedColor & 0xFFu) * multiplier * kByteToFloat;
        }
    }

    void ZRenderSoftwareLight::CalcLight(uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode, bool bIncludeHQLights)
    {
        (void)lDrawMode;
        if (lNumVertices == 0)
        {
            return;
        }

        const uint32_t lNumVectors = (lNumVertices + 3u) >> 2u;
        const __m128 zero = _mm_setzero_ps();
        const __m128 one = _mm_set1_ps(1.0f);

        for (uint32_t i = 0; i < lNumVectors; ++i)
        {
            _mm_storeu_ps(m_SSEArrays.aDstCR + i * 4, zero);
            _mm_storeu_ps(m_SSEArrays.aDstCG + i * 4, zero);
            _mm_storeu_ps(m_SSEArrays.aDstCB + i * 4, zero);
        }

        for (uint32_t l = 0; l < lNumLights; ++l)
        {
            const SUpdateLightData& lightData = pLights[l];
            if ((lightData.lLightControl & 2u) != 0 && !bIncludeHQLights)
            {
                continue;
            }

            const SUpdateLightDataSSE& light = lightData.SSE;
            for (uint32_t i = 0; i < lNumVectors; ++i)
            {
                const uint32_t offset = i * 4;
                __m128 red = _mm_loadu_ps(m_SSEArrays.aDstCR + offset);
                __m128 green = _mm_loadu_ps(m_SSEArrays.aDstCG + offset);
                __m128 blue = _mm_loadu_ps(m_SSEArrays.aDstCB + offset);
                const __m128 nx = _mm_loadu_ps(m_SSEArrays.aSrcNX + offset);
                const __m128 ny = _mm_loadu_ps(m_SSEArrays.aSrcNY + offset);
                const __m128 nz = _mm_loadu_ps(m_SSEArrays.aSrcNZ + offset);

                if (lightData.lType <= 2u)
                {
                    const __m128 x = _mm_sub_ps(_mm_load_ps(light.vPosX), _mm_loadu_ps(m_SSEArrays.aSrcPX + offset));
                    const __m128 y = _mm_sub_ps(_mm_load_ps(light.vPosY), _mm_loadu_ps(m_SSEArrays.aSrcPY + offset));
                    const __m128 z = _mm_sub_ps(_mm_load_ps(light.vPosZ), _mm_loadu_ps(m_SSEArrays.aSrcPZ + offset));
                    const __m128 ndotl = _mm_max_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, nx), _mm_mul_ps(y, ny)), _mm_mul_ps(z, nz)), zero);
                    __m128 lx = x;
                    __m128 ly = y;
                    __m128 lz = z;

                    if (lightData.lType == 2u)
                    {
                        lx = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirXAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirXAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirXAZ)));
                        ly = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirYAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirYAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirYAZ)));
                        lz = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirZAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirZAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirZAZ)));
                    }

                    const __m128 invLength = _mm_rsqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(lx, lx), _mm_mul_ps(ly, ly)), _mm_mul_ps(lz, lz)));
                    const __m128 attenuation = Saturate(_mm_mul_ps(_mm_sub_ps(_mm_load_ps(light.fFarRange), _mm_rcp_ps(invLength)), _mm_load_ps(light.fInvFarNear)));
                    __m128 intensity = _mm_mul_ps(_mm_mul_ps(attenuation, invLength), ndotl);

                    if (lightData.lType == 0u)
                    {
                        const __m128 angle = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirZAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirZAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirZAZ))), invLength);
                        const __m128 spot = Saturate(_mm_mul_ps(_mm_sub_ps(_mm_sub_ps(zero, angle), _mm_load_ps(light.fCosFallOff)), _mm_load_ps(light.fLConst1)));
                        intensity = _mm_mul_ps(intensity, SmoothStep(spot));
                    }
                    else if (lightData.lType == 2u)
                    {
                        const __m128 invZ = _mm_rcp_ps(lz);
                        const __m128 u = Saturate(_mm_mul_ps(_mm_sub_ps(Abs(_mm_mul_ps(_mm_mul_ps(lx, _mm_load_ps(light.fScaleX)), invZ)), _mm_load_ps(light.hotpct)), _mm_load_ps(light.ihotpct)));
                        const __m128 v = Saturate(_mm_mul_ps(_mm_sub_ps(Abs(_mm_mul_ps(_mm_mul_ps(ly, _mm_load_ps(light.fScaleY)), invZ)), _mm_load_ps(light.hotpct)), _mm_load_ps(light.ihotpct)));
                        const __m128 edge = _mm_mul_ps(_mm_sub_ps(one, u), _mm_sub_ps(one, v));
                        intensity = _mm_mul_ps(intensity, SmoothStep(edge));
                    }

                    red = _mm_add_ps(red, _mm_mul_ps(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aSrcCR + offset), intensity), _mm_load_ps(light.cRed1)));
                    green = _mm_add_ps(green, _mm_mul_ps(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aSrcCG + offset), intensity), _mm_load_ps(light.cGreen1)));
                    blue = _mm_add_ps(blue, _mm_mul_ps(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aSrcCB + offset), intensity), _mm_load_ps(light.cBlue1)));
                }
                else if (lightData.lType == 3u || lightData.lType == 4u)
                {
                    const __m128 factor = _mm_mul_ps(_mm_sub_ps(one, _mm_add_ps(_mm_add_ps(_mm_mul_ps(nx, _mm_load_ps(light.vDirZAX)), _mm_mul_ps(ny, _mm_load_ps(light.vDirZAY))), _mm_mul_ps(nz, _mm_load_ps(light.vDirZAZ)))), _mm_set1_ps(0.5f));
                    red = _mm_add_ps(red, _mm_mul_ps(_mm_add_ps(_mm_load_ps(light.cRed1), _mm_mul_ps(_mm_load_ps(light.cRed2), factor)), _mm_loadu_ps(m_SSEArrays.aSrcCR + offset)));
                    green = _mm_add_ps(green, _mm_mul_ps(_mm_add_ps(_mm_load_ps(light.cGreen1), _mm_mul_ps(_mm_load_ps(light.cGreen2), factor)), _mm_loadu_ps(m_SSEArrays.aSrcCG + offset)));
                    blue = _mm_add_ps(blue, _mm_mul_ps(_mm_add_ps(_mm_load_ps(light.cBlue1), _mm_mul_ps(_mm_load_ps(light.cBlue2), factor)), _mm_loadu_ps(m_SSEArrays.aSrcCB + offset)));
                }

                _mm_storeu_ps(m_SSEArrays.aDstCR + offset, red);
                _mm_storeu_ps(m_SSEArrays.aDstCG + offset, green);
                _mm_storeu_ps(m_SSEArrays.aDstCB + offset, blue);
            }
        }

        for (uint32_t i = 0; i < lNumVectors; ++i)
        {
            const uint32_t offset = i * 4;
            _mm_storeu_ps(m_SSEArrays.aDstCR + offset, Saturate(_mm_loadu_ps(m_SSEArrays.aDstCR + offset)));
            _mm_storeu_ps(m_SSEArrays.aDstCG + offset, Saturate(_mm_loadu_ps(m_SSEArrays.aDstCG + offset)));
            _mm_storeu_ps(m_SSEArrays.aDstCB + offset, Saturate(_mm_loadu_ps(m_SSEArrays.aDstCB + offset)));
        }
    }

    uint32_t ZRenderSoftwareLight::GetUpdateLightData(SUpdateLightData* pUpdateLightData, const ZBaseGeom* pBaseGeom, const ZBaseGeom* pEnvironment)
    {
        CListUser* pListUser = g_pSysInterface->m_pEngineData->m_pListUser;
        if (!pListUser)
        {
            return 0;
        }

        uint32_t lListLength = 0;
        uint32_t* pList = pListUser->UnfoldList(&lListLength, pBaseGeom->ListId());
        const ZBaseGeom* lights[kMaxLights];
        uint32_t lCandidateCount = 0;
        if (pEnvironment)
        {
            lights[lCandidateCount++] = pEnvironment;
        }

        for (uint32_t i = 0; i < lListLength && lCandidateCount < kMaxLights; ++i)
        {
            const ZBaseGeom* pLightBase = reinterpret_cast<const ZBaseGeom*>(pList[i]);
            if ((pLightBase->m_lControl & 0x2400u) != 0)
            {
                continue;
            }

            ZGEOM* pGeom = pLightBase->GetGeom();
            ZASSERT(pGeom ? pGeom->IsDerivedFrom<ZLIGHT>() : pLightBase->IsDerivedFromStdObj(ZLIGHT::m_Id));
            const ZLIGHT* pLightGeom = static_cast<const ZLIGHT*>(pGeom);
            if (pLightGeom && pLightGeom->m_rMasterLight)
            {
                const ZGEOM* pMaster = ZGEOM::RefToPtr(pLightGeom->m_rMasterLight);
                bool bMasterInList = false;
                for (uint32_t j = 0; j < lListLength; ++j)
                {
                    if (pMaster && reinterpret_cast<const ZBaseGeom*>(pList[j]) == pMaster->BaseGeom())
                    {
                        bMasterInList = true;
                        break;
                    }
                }
                if (bMasterInList)
                {
                    continue;
                }
            }
            lights[lCandidateCount++] = pLightBase;
        }

        ZMat3x3 baseMatrix;
        ZVector3 basePosition;
        pBaseGeom->GetRootTM(baseMatrix, basePosition);
        uint32_t lResult = 0;
        for (uint32_t i = 0; i < lCandidateCount; ++i)
        {
            const ZBaseGeom* pLightBase = lights[i];
            const int32_t lPrim = static_cast<int32_t>(pLightBase->m_lPrim);
            const SPrimLight* pPrim = lPrim >= 0
                ? static_cast<const SPrimLight*>(g_apPrimHandleToPointerTable[lPrim])
                : reinterpret_cast<const SPrimLight*>(static_cast<uintptr_t>(lPrim) & 0x7FFFFFFFu);
            if (!pPrim)
            {
                continue;
            }

            SUpdateLightData& output = pUpdateLightData[lResult];
            ZMat3x3 lightMatrix;
            ZVector3 lightPosition;
            pLightBase->GetRootTM(lightMatrix, lightPosition);
            vsub(output.vPos, lightPosition.Get(), basePosition.Get());
            vmtmul(output.vPos, baseMatrix.Get());
            mmtmul(output.mDir, lightMatrix.Get(), baseMatrix.Get());

            output.lType = pPrim->lLightType;
            output.lGeomCon = pLightBase->GetGeom() ? pLightBase->GetGeom()->BaseGeom()->m_lControl : pLightBase->m_lControl;
            output.lLightControl = pPrim->lLightControl;
            output.lDrawEntryId = pLightBase->m_lDrawEntryId;
            DecodeColor(output.vShadowColor, pPrim->lStaticShadowColor, 1.0f);

            if (output.lType <= 2u)
            {
                DecodeColor(output.vColor1, pPrim->lDiffuseColor, pPrim->fMultiplier);
                const SPrimLightOmni* pOmni = static_cast<const SPrimLightOmni*>(pPrim);
                output.fNearRange = pOmni->fNearRange;
                output.fNearRange2 = pOmni->fNearRange * pOmni->fNearRange;
                output.fFarRange = pOmni->fFarRange;
                output.fFarRange2 = pOmni->fFarRange * pOmni->fFarRange;
                output.fInvFarNear = pOmni->fInverseFarMinusNear;
            }

            if (output.lType == 0u || output.lType == 2u)
            {
                const SPrimLightSpot* pSpot = static_cast<const SPrimLightSpot*>(pPrim);
                output.fCosFallOff = pSpot->fCosFallOff;
                output.fCosHotSpot = pSpot->fCosHotSpot;
                output.fFallOff = pSpot->fFallOff;
                output.fLConst1 = pSpot->fLConst1;
            }

            if (output.lType == 2u)
            {
                const SPrimLightSpotSquare* pSquare = static_cast<const SPrimLightSpotSquare*>(pPrim);
                if (output.fFarRange == output.fNearRange || pSquare->fFallOff == 0.0f)
                {
                    continue;
                }
                const float fTanFallOff = std::tan(pSquare->fFallOff);
                output.fScaleX = 1.0f / (std::sqrt(pSquare->fAspect) * fTanFallOff);
                output.fScaleY = output.fScaleX * pSquare->fAspect;
                output.hotpct = std::tan(pSquare->fHotSpot) / fTanFallOff;
                output.ihotpct = 1.0f / (1.0f - output.hotpct);
            }
            else if (output.lType == 3u || output.lType == 4u)
            {
                const SPrimLightEnvironment* pEnvironmentPrim = static_cast<const SPrimLightEnvironment*>(pPrim);
                DecodeColor(output.vColor1, pEnvironmentPrim->lDiffuseColorBack, pPrim->fMultiplier);
                float frontColor[3];
                DecodeColor(frontColor, pPrim->lDiffuseColor, pPrim->fMultiplier);
                for (uint32_t component = 0; component < 3; ++component)
                {
                    output.vColor2[component] = frontColor[component] - output.vColor1[component];
                }
            }

            ConvertToSSE(output);
            if (++lResult == kMaxLights)
            {
                break;
            }
        }
        return lResult;
    }

    void ZRenderSoftwareLight::CalcLightHQ(uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode, bool bIncludeHQLights)
    {
        if (lNumVertices == 0)
        {
            return;
        }

        const uint32_t lNumVectors = (lNumVertices + 3u) >> 2u;
        const __m128 zero = _mm_setzero_ps();
        const __m128 one = _mm_set1_ps(1.0f);
        const __m128 half = _mm_set1_ps(0.5f);

        if ((lDrawMode & 0x2000u) != 0)
        {
            for (uint32_t i = 0; i < lNumVectors; ++i)
            {
                const uint32_t offset = i * 4;
                __m128 red = zero;
                __m128 green = zero;
                __m128 blue = zero;
                for (uint32_t l = 0; l < lNumLights; ++l)
                {
                    if (pLights[l].lType == 3u || pLights[l].lType == 4u)
                    {
                        red = _mm_add_ps(red, _mm_load_ps(pLights[l].SSE.cRed1));
                        green = _mm_add_ps(green, _mm_load_ps(pLights[l].SSE.cGreen1));
                        blue = _mm_add_ps(blue, _mm_load_ps(pLights[l].SSE.cBlue1));
                    }
                }
                _mm_storeu_ps(m_SSEArrays.aDstCR + offset, zero);
                _mm_storeu_ps(m_SSEArrays.aDstCG + offset, zero);
                _mm_storeu_ps(m_SSEArrays.aDstCB + offset, zero);
                _mm_storeu_ps(m_SSEArrays.aDirX + offset, zero);
                _mm_storeu_ps(m_SSEArrays.aDirY + offset, zero);
                _mm_storeu_ps(m_SSEArrays.aDirZ + offset, one);
                _mm_storeu_ps(m_SSEArrays.aAmbCR + offset, _mm_mul_ps(_mm_sub_ps(one, _mm_min_ps(red, one)), half));
                _mm_storeu_ps(m_SSEArrays.aAmbCG + offset, _mm_mul_ps(_mm_sub_ps(one, _mm_min_ps(green, one)), half));
                _mm_storeu_ps(m_SSEArrays.aAmbCB + offset, _mm_mul_ps(_mm_sub_ps(one, _mm_min_ps(blue, one)), half));
            }
            return;
        }

        for (uint32_t i = 0; i < lNumVectors; ++i)
        {
            const uint32_t offset = i * 4;
            _mm_storeu_ps(m_SSEArrays.aDstCR + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aDstCG + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aDstCB + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aDirX + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aDirY + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aDirZ + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aAmbCR + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aAmbCG + offset, zero);
            _mm_storeu_ps(m_SSEArrays.aAmbCB + offset, zero);
        }

        for (uint32_t l = 0; l < lNumLights; ++l)
        {
            const SUpdateLightData& lightData = pLights[l];
            if ((lightData.lLightControl & 2u) != 0 && !bIncludeHQLights)
            {
                continue;
            }
            const bool bAmbient = (lightData.lLightControl & 4u) != 0;
            const SUpdateLightDataSSE& light = lightData.SSE;
            const __m128 averageColor = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_load_ps(light.cRed1), _mm_load_ps(light.cGreen1)), _mm_load_ps(light.cBlue1)), _mm_set1_ps(1.0f / 3.0f));

            for (uint32_t i = 0; i < lNumVectors; ++i)
            {
                const uint32_t offset = i * 4;
                const __m128 nx = _mm_loadu_ps(m_SSEArrays.aSrcNX + offset);
                const __m128 ny = _mm_loadu_ps(m_SSEArrays.aSrcNY + offset);
                const __m128 nz = _mm_loadu_ps(m_SSEArrays.aSrcNZ + offset);
                __m128 red = _mm_loadu_ps((bAmbient ? m_SSEArrays.aAmbCR : m_SSEArrays.aDstCR) + offset);
                __m128 green = _mm_loadu_ps((bAmbient ? m_SSEArrays.aAmbCG : m_SSEArrays.aDstCG) + offset);
                __m128 blue = _mm_loadu_ps((bAmbient ? m_SSEArrays.aAmbCB : m_SSEArrays.aDstCB) + offset);

                if (lightData.lType <= 2u)
                {
                    const __m128 x = _mm_sub_ps(_mm_load_ps(light.vPosX), _mm_loadu_ps(m_SSEArrays.aSrcPX + offset));
                    const __m128 y = _mm_sub_ps(_mm_load_ps(light.vPosY), _mm_loadu_ps(m_SSEArrays.aSrcPY + offset));
                    const __m128 z = _mm_sub_ps(_mm_load_ps(light.vPosZ), _mm_loadu_ps(m_SSEArrays.aSrcPZ + offset));
                    const __m128 ndotl = _mm_max_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, nx), _mm_mul_ps(y, ny)), _mm_mul_ps(z, nz)), zero);
                    __m128 lx = x;
                    __m128 ly = y;
                    __m128 lz = z;
                    if (lightData.lType == 2u)
                    {
                        lx = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirXAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirXAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirXAZ)));
                        ly = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirYAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirYAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirYAZ)));
                        lz = _mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirZAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirZAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirZAZ)));
                    }
                    const __m128 invLength = _mm_rsqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(lx, lx), _mm_mul_ps(ly, ly)), _mm_mul_ps(lz, lz)));
                    __m128 intensity = _mm_mul_ps(_mm_mul_ps(Saturate(_mm_mul_ps(_mm_sub_ps(_mm_load_ps(light.fFarRange), _mm_rcp_ps(invLength)), _mm_load_ps(light.fInvFarNear))), invLength), ndotl);
                    if (lightData.lType == 0u)
                    {
                        const __m128 angle = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, _mm_load_ps(light.vDirZAX)), _mm_mul_ps(y, _mm_load_ps(light.vDirZAY))), _mm_mul_ps(z, _mm_load_ps(light.vDirZAZ))), invLength);
                        intensity = _mm_mul_ps(intensity, SmoothStep(Saturate(_mm_mul_ps(_mm_sub_ps(_mm_sub_ps(zero, angle), _mm_load_ps(light.fCosFallOff)), _mm_load_ps(light.fLConst1)))));
                    }
                    else if (lightData.lType == 2u)
                    {
                        const __m128 invZ = _mm_rcp_ps(lz);
                        const __m128 u = Saturate(_mm_mul_ps(_mm_sub_ps(Abs(_mm_mul_ps(_mm_mul_ps(lx, _mm_load_ps(light.fScaleX)), invZ)), _mm_load_ps(light.hotpct)), _mm_load_ps(light.ihotpct)));
                        const __m128 v = Saturate(_mm_mul_ps(_mm_sub_ps(Abs(_mm_mul_ps(_mm_mul_ps(ly, _mm_load_ps(light.fScaleY)), invZ)), _mm_load_ps(light.hotpct)), _mm_load_ps(light.ihotpct)));
                        intensity = _mm_mul_ps(_mm_and_ps(intensity, _mm_cmple_ps(lz, zero)), SmoothStep(_mm_mul_ps(_mm_sub_ps(one, u), _mm_sub_ps(one, v))));
                    }
                    AddColor(red, green, blue, light, intensity);
                    if (!bAmbient)
                    {
                        const __m128 directionScale = _mm_mul_ps(_mm_mul_ps(invLength, averageColor), intensity);
                        _mm_storeu_ps(m_SSEArrays.aDirX + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirX + offset), _mm_mul_ps(x, directionScale)));
                        _mm_storeu_ps(m_SSEArrays.aDirY + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirY + offset), _mm_mul_ps(y, directionScale)));
                        _mm_storeu_ps(m_SSEArrays.aDirZ + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirZ + offset), _mm_mul_ps(z, directionScale)));
                    }
                }
                else if (lightData.lType == 3u || lightData.lType == 4u)
                {
                    const __m128 dot = _mm_add_ps(_mm_add_ps(_mm_mul_ps(nx, _mm_load_ps(light.vDirZAX)), _mm_mul_ps(ny, _mm_load_ps(light.vDirZAY))), _mm_mul_ps(nz, _mm_load_ps(light.vDirZAZ)));
                    const __m128 lightX = _mm_sub_ps(zero, _mm_load_ps(light.vDirZAX));
                    const __m128 lightY = _mm_sub_ps(zero, _mm_load_ps(light.vDirZAY));
                    const __m128 lightZ = _mm_sub_ps(zero, _mm_load_ps(light.vDirZAZ));

                    if (lightData.lType == 3u)
                    {
                        if (bAmbient)
                        {
                            const __m128 factor = _mm_mul_ps(_mm_sub_ps(one, dot), half);
                            red = _mm_add_ps(red, _mm_add_ps(_mm_load_ps(light.cRed1), _mm_mul_ps(_mm_load_ps(light.cRed2), factor)));
                            green = _mm_add_ps(green, _mm_add_ps(_mm_load_ps(light.cGreen1), _mm_mul_ps(_mm_load_ps(light.cGreen2), factor)));
                            blue = _mm_add_ps(blue, _mm_add_ps(_mm_load_ps(light.cBlue1), _mm_mul_ps(_mm_load_ps(light.cBlue2), factor)));
                        }
                        else
                        {
                            const __m128 lightDot = _mm_sub_ps(zero, dot);
                            const __m128 blend = _mm_max_ps(lightDot, zero);
                            __m128 bentX = _mm_add_ps(nx, _mm_mul_ps(_mm_sub_ps(lightX, nx), blend));
                            __m128 bentY = _mm_add_ps(ny, _mm_mul_ps(_mm_sub_ps(lightY, ny), blend));
                            __m128 bentZ = _mm_add_ps(nz, _mm_mul_ps(_mm_sub_ps(lightZ, nz), blend));
                            const __m128 bentLength2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(bentX, bentX), _mm_mul_ps(bentY, bentY)), _mm_mul_ps(bentZ, bentZ));
                            const __m128 bentInvLength = _mm_and_ps(_mm_rsqrt_ps(bentLength2), _mm_cmpneq_ps(bentLength2, zero));
                            bentX = _mm_mul_ps(bentX, bentInvLength);
                            bentY = _mm_mul_ps(bentY, bentInvLength);
                            bentZ = _mm_mul_ps(bentZ, bentInvLength);

                            const __m128 factor = _mm_add_ps(_mm_mul_ps(lightDot, half), half);
                            const __m128 colorRed = _mm_add_ps(_mm_load_ps(light.cRed1), _mm_mul_ps(_mm_load_ps(light.cRed2), factor));
                            const __m128 colorGreen = _mm_add_ps(_mm_load_ps(light.cGreen1), _mm_mul_ps(_mm_load_ps(light.cGreen2), factor));
                            const __m128 colorBlue = _mm_add_ps(_mm_load_ps(light.cBlue1), _mm_mul_ps(_mm_load_ps(light.cBlue2), factor));
                            const __m128 colorAverage = _mm_mul_ps(_mm_add_ps(_mm_add_ps(colorRed, colorGreen), colorBlue), _mm_set1_ps(1.0f / 3.0f));
                            const __m128 bentDot = _mm_add_ps(_mm_add_ps(_mm_mul_ps(bentX, nx), _mm_mul_ps(bentY, ny)), _mm_mul_ps(bentZ, nz));
                            red = _mm_add_ps(red, _mm_mul_ps(colorRed, bentDot));
                            green = _mm_add_ps(green, _mm_mul_ps(colorGreen, bentDot));
                            blue = _mm_add_ps(blue, _mm_mul_ps(colorBlue, bentDot));
                            _mm_storeu_ps(m_SSEArrays.aDirX + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirX + offset), _mm_mul_ps(bentX, colorAverage)));
                            _mm_storeu_ps(m_SSEArrays.aDirY + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirY + offset), _mm_mul_ps(bentY, colorAverage)));
                            _mm_storeu_ps(m_SSEArrays.aDirZ + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirZ + offset), _mm_mul_ps(bentZ, colorAverage)));
                        }
                    }
                    else
                    {
                        const __m128 lightDot = _mm_sub_ps(zero, dot);
                        const __m128 positiveDot = _mm_max_ps(lightDot, zero);
                        const __m128 colorSumRed = _mm_add_ps(_mm_load_ps(light.cRed1), _mm_load_ps(light.cRed2));
                        const __m128 colorSumGreen = _mm_add_ps(_mm_load_ps(light.cGreen1), _mm_load_ps(light.cGreen2));
                        const __m128 colorSumBlue = _mm_add_ps(_mm_load_ps(light.cBlue1), _mm_load_ps(light.cBlue2));
                        red = _mm_add_ps(red, _mm_add_ps(_mm_load_ps(light.cRed1), _mm_mul_ps(colorSumRed, positiveDot)));
                        green = _mm_add_ps(green, _mm_add_ps(_mm_load_ps(light.cGreen1), _mm_mul_ps(colorSumGreen, positiveDot)));
                        blue = _mm_add_ps(blue, _mm_add_ps(_mm_load_ps(light.cBlue1), _mm_mul_ps(colorSumBlue, positiveDot)));

                        if (!bAmbient)
                        {
                            const __m128 colorAverage = _mm_mul_ps(_mm_add_ps(_mm_add_ps(colorSumRed, colorSumGreen), colorSumBlue), _mm_set1_ps(1.0f / 3.0f));
                            const __m128 directionScale = _mm_or_ps(
                                _mm_and_ps(_mm_cmpgt_ps(lightDot, zero), _mm_sub_ps(zero, _mm_mul_ps(lightDot, colorAverage))),
                                _mm_andnot_ps(_mm_cmpgt_ps(lightDot, zero), _mm_set1_ps(-0.001f)));
                            _mm_storeu_ps(m_SSEArrays.aDirX + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirX + offset), _mm_mul_ps(_mm_load_ps(light.vDirZAX), directionScale)));
                            _mm_storeu_ps(m_SSEArrays.aDirY + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirY + offset), _mm_mul_ps(_mm_load_ps(light.vDirZAY), directionScale)));
                            _mm_storeu_ps(m_SSEArrays.aDirZ + offset, _mm_add_ps(_mm_loadu_ps(m_SSEArrays.aDirZ + offset), _mm_mul_ps(_mm_load_ps(light.vDirZAZ), directionScale)));
                        }
                    }
                }

                _mm_storeu_ps((bAmbient ? m_SSEArrays.aAmbCR : m_SSEArrays.aDstCR) + offset, red);
                _mm_storeu_ps((bAmbient ? m_SSEArrays.aAmbCG : m_SSEArrays.aDstCG) + offset, green);
                _mm_storeu_ps((bAmbient ? m_SSEArrays.aAmbCB : m_SSEArrays.aDstCB) + offset, blue);
            }
        }

        for (uint32_t i = 0; i < lNumVectors; ++i)
        {
            const uint32_t offset = i * 4;
            __m128 dx = _mm_loadu_ps(m_SSEArrays.aDirX + offset);
            __m128 dy = _mm_loadu_ps(m_SSEArrays.aDirY + offset);
            __m128 dz = _mm_loadu_ps(m_SSEArrays.aDirZ + offset);
            const __m128 length2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(dx, dx), _mm_mul_ps(dy, dy)), _mm_mul_ps(dz, dz));
            const __m128 nonZero = _mm_cmpneq_ps(length2, zero);
            const __m128 invLength = _mm_and_ps(_mm_rsqrt_ps(length2), nonZero);
            dx = _mm_mul_ps(dx, invLength);
            dy = _mm_mul_ps(dy, invLength);
            dz = _mm_mul_ps(dz, invLength);
            _mm_storeu_ps(m_SSEArrays.aDirX + offset, dx);
            _mm_storeu_ps(m_SSEArrays.aDirY + offset, dy);
            _mm_storeu_ps(m_SSEArrays.aDirZ + offset, dz);

            const __m128 dot = _mm_add_ps(_mm_add_ps(_mm_mul_ps(dx, _mm_loadu_ps(m_SSEArrays.aSrcNX + offset)), _mm_mul_ps(dy, _mm_loadu_ps(m_SSEArrays.aSrcNY + offset))), _mm_mul_ps(dz, _mm_loadu_ps(m_SSEArrays.aSrcNZ + offset)));
            const __m128 invDot = _mm_min_ps(_mm_and_ps(_mm_rcp_ps(dot), _mm_cmpneq_ps(dot, zero)), _mm_set1_ps(1000.0f));
            _mm_storeu_ps(m_SSEArrays.aDstCR + offset, Saturate(_mm_mul_ps(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aDstCR + offset), _mm_loadu_ps(m_SSEArrays.aSrcCR + offset)), invDot)));
            _mm_storeu_ps(m_SSEArrays.aDstCG + offset, Saturate(_mm_mul_ps(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aDstCG + offset), _mm_loadu_ps(m_SSEArrays.aSrcCG + offset)), invDot)));
            _mm_storeu_ps(m_SSEArrays.aDstCB + offset, Saturate(_mm_mul_ps(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aDstCB + offset), _mm_loadu_ps(m_SSEArrays.aSrcCB + offset)), invDot)));
            _mm_storeu_ps(m_SSEArrays.aAmbCR + offset, Saturate(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aAmbCR + offset), _mm_loadu_ps(m_SSEArrays.aSrcCR + offset))));
            _mm_storeu_ps(m_SSEArrays.aAmbCG + offset, Saturate(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aAmbCG + offset), _mm_loadu_ps(m_SSEArrays.aSrcCG + offset))));
            _mm_storeu_ps(m_SSEArrays.aAmbCB + offset, Saturate(_mm_mul_ps(_mm_loadu_ps(m_SSEArrays.aAmbCB + offset), _mm_loadu_ps(m_SSEArrays.aSrcCB + offset))));
        }
    }
}
