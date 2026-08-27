#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZBaseGeom;
    struct SUpdateLightData;

    // Confirmed by Mini Ninjas XBOX PDB (empty struct, all members are static).
    struct ZRenderSoftwareLight
    {
        // SoA scratch buffers for a chunk of up to 32 vertices.
        // Layout confirmed by Mini Ninjas XBOX PDB (ZRenderSoftwareLight::SSEArrays).
        // aDirX/aDirY/aDirZ and aAmbCR/aAmbCG/aAmbCB are used by CalcLightHQ only.
        struct alignas(16) SSEArrays
        {
            float aSrcPX[32];
            float aSrcPY[32];
            float aSrcPZ[32];
            float aSrcNX[32];
            float aSrcNY[32];
            float aSrcNZ[32];
            float aSrcCA[32];
            float aSrcCR[32];
            float aSrcCG[32];
            float aSrcCB[32];
            float aDstCR[32];
            float aDstCG[32];
            float aDstCB[32];
            float aDirX[32];
            float aDirY[32];
            float aDirZ[32];
            float aAmbCR[32];
            float aAmbCG[32];
            float aAmbCB[32];
        };
        RE_VERIFY_SIZE(SSEArrays, 0x980);

        // Protected static in XBOX PDB; public here so render object instances can fill the arrays.
        static SSEArrays m_SSEArrays;

        // PC (Blood Money) version takes 5 arguments; the Mini Ninjas XBOX version has an extra
        // trailing lAlwaysInShadowColor argument. lDrawMode is passed but unused by the PC implementation.
        static void CalcLight(uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode, bool bIncludeHQLights);

        static void CalcLightHQ(uint32_t lNumVertices, const SUpdateLightData* pLights, uint32_t lNumLights, uint32_t lDrawMode, bool bIncludeHQLights);

        static uint32_t GetUpdateLightData(SUpdateLightData* pUpdateLightData, const ZBaseGeom* pBaseGeom, const ZBaseGeom* pEnvironment);
    };
}
