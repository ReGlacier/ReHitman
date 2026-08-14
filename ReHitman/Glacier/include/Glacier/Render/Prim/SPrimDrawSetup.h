#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimDrawSetup : SPrims
    {
        uint8_t lLODControl;
        uint8_t lBias;
        uint16_t lVariantId;
        uint32_t lDrawMode;
        uint32_t lDiffuseColor;
        uint32_t lTempHeader;
    };
    RE_VERIFY_SIZE(SPrimDrawSetup, 0x1C);
}