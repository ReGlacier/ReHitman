#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::Animation
{
    struct ZStateBlending 
    {
        uint32_t m_MagicNumber;
        float  m_BlendTime;
    };

    RE_VERIFY_SIZE(ZStateBlending, 8);
}