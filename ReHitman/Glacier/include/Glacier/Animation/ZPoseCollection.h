#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Animation/Fwd.h>
#include <cstdint>


namespace Glacier::Animation
{
    struct ZPoseCollection
    {
        // methods
        ZPoseCollection();

        ZPoseBone* poseData();
        int32_t poseCount();
        int32_t* indexToOffsetLookup();
        int16_t* idToPosLookup();

        // members
        uint32_t m_PoseIdx { 0 };
    };
    RE_VERIFY_SIZE(ZPoseCollection, 0x4);
} // namespace Glacier::Animation
