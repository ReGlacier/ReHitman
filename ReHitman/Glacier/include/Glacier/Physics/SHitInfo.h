#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    struct COLI;

    struct SHitInfo
    {
        COLI* pColi;
        ZREF rShot;
        ZREF rWeapon;
        ZREF rWeaponOwner;
        ZREF rWeaponTemplate;
        float fRange;
    };

    using THitInfo = SHitInfo;
}