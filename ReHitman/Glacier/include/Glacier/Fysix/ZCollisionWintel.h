#pragma once

#include <Glacier/Fysix/ZCollisionBase.h>

namespace Glacier
{
    class ZCollisionWintel : public ZCollisionBase
    {
    };
    RE_VERIFY_SIZE(ZCollisionWintel, 0x20B4); // must be same
}