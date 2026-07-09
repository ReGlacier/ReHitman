#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    namespace Animation
    {
        struct ZState {};
    }

    struct ZHumanState : Animation::ZState
    {
        ZQuat m_Quats[10];   
        float m_Floats[64];
    };
    RE_VERIFY_SIZE(ZHumanState, 0x1A0);
}