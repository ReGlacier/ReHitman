#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct ZPoseBone
    {
        ZQuat m_Quat;
        ZVector3 m_Pos;
        ZVector3 m_Scale;
        int32_t m_BoneIndex;
    };
    RE_VERIFY_SIZE(ZPoseBone, 0x2C);
}
