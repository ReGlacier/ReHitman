#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Vehicle/ZVehicle.h>


namespace Glacier
{
    class ZDriving : public ZVehicle
    {
        // No new vtbl & data members
    };
    RE_VERIFY_SIZE(ZDriving, 0x1D0); // Verified
}