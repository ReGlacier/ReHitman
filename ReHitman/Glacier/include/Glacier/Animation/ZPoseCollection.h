#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier::Animation
{
    struct ZPoseCollection
    {
        uint32_t m_PoseIdx;
    };
    RE_VERIFY_SIZE(ZPoseCollection, 0x4);
} // namespace Glacier::Animation
