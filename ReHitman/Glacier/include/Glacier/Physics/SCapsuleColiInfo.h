#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SCapsuleColiInfo
    {
        ZVector3 vDir;
        float t0;
        float fScaledDist;
    };
    RE_VERIFY_SIZE(SCapsuleColiInfo, 0x14);
}
