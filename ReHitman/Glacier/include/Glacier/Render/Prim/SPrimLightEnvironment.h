#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimLightEnvironment : SPrimLight
    {
        uint32_t lDiffuseColorBack;
        bool bIsDirectional;
    };
    RE_VERIFY_SIZE(SPrimLightEnvironment, 0x28);
}