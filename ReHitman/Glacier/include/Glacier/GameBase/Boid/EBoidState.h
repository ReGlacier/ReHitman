#pragma once

namespace Glacier
{
    enum EBoidState
    {
        eFollowPath = 0,
        eControlled = 1,
        eSoftObstacle = 2,
        eHardObstacle = 3,
        ePassivePushable = 4,
        eInActive = 5,
        eHero = 6,
    };
}
