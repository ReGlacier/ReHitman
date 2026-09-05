#pragma once

#include <cstdint>

namespace Glacier
{
    enum EQuatIds : uint32_t 
    {
        ePelvisQuat = 0,
        eSpineQuat = 1,
        eNeckQuat = 2,
        eHeadQuat = 3,
        eLeftFootQuat = 4,
        eRightFootQuat = 5,
        eLeftHandQuat  = 6,
        eLeftClavicleQuat = 7,
        eRightHandQuat = 8,
        eRightClavicleQuat = 9,
        eQuats = 10
    };
}