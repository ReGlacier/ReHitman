#pragma once

#include <cstdint>


namespace Glacier
{
    using PoseID = uint8_t;

    enum EPoseID : PoseID {
        MaxPoseIDs = 126,
        ePoseIDNA = 127
    };
}