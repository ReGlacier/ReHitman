#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimLight : SPrims
    {
        uint8_t lLightType;
        RE_ADD_PADDING(1);
        uint16_t lLightControl;
        uint32_t lDiffuseColor;
        float fMultiplier;
        uint32_t lProjectorMap;
        uint32_t lStaticShadowColor;
    };
    RE_VERIFY_SIZE(SPrimLight, 0x20);
}