#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimLightOmni.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimLightSpot : SPrimLightOmni
    {
        float fFallOff;
        float fHotSpot;
        float fCosFallOff;
        float fCosHotSpot;
        float fLConst1;       
    };
    RE_VERIFY_SIZE(SPrimLightSpot, 0x40);
}