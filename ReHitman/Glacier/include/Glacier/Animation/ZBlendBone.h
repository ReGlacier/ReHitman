#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::Animation
{
    struct ZBlendBone
    {
        ZQuat m_Quat;
        ZVector3 m_Pos;

        union {
            float    m_Blend;
            int32_t  m_lBlend;
        };

        int32_t  m_MagicNr;
        uint32_t m_Align[3];
    };
    RE_VERIFY_SIZE(ZBlendBone, 48);
}