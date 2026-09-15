#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    struct SIconBase 
    {
        Glacier::ZREF rGeomObj;
        bool bVisible;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(SIconBase, 0x8); // Verified
}