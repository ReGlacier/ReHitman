#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimLightSpotSquare : SPrimLightSpot
    {
        float fAspect;
    };
    RE_VERIFY_SIZE(SPrimLightSpotSquare, 0x44);
}