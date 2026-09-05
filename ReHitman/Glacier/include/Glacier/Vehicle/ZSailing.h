#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Vehicle/ZVehicle.h>


namespace Glacier
{
    class ZSailing : public ZVehicle
    {
        // No new vtbl & data stuff 
    };
    RE_VERIFY_SIZE(ZSailing, 0x1D0); // Verified
}