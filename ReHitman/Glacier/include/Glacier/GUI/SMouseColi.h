#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    struct SMouseColi
    {
        ZREF rGeom;
        ZVector3 vColi;
        ZCAMERA* pCamera;
    };
    RE_VERIFY_SIZE(SMouseColi, 0x14);
}
