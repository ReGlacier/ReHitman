#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimLightOmni : SPrimLight
    {
        float fNearRange;
        float fFarRange;
        float fInverseFarMinusNear;
    };
    RE_VERIFY_SIZE(SPrimLightOmni, 0x2C);
}