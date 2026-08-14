#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimStrips.h>
#include <cstring>


namespace Glacier
{
    struct SPrimStripsWintel : SPrimStrips
    {
        uint32_t lWireColor;
        uint32_t lIndices;
        uint32_t lIndicesCount;
        uint32_t lAlphaCutOff;
        uint32_t lFrameStart;
        uint32_t lFrameStep;
        uint32_t lNrKeys;
        float fBias;
        float fOsset;
        uint32_t lEnvMapType;
        uint32_t lEnvMapTexture;
        float fEnvMapAmount;
        uint32_t lBmpMapTexture;
        float fBmpMapAmount;
        float fBmpMapLuminanceScale;
        float fBmpMapLuminanceOffset;
        uint32_t lLengths;
    };
}