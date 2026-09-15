#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZBoatMoveModelIF
    {
        // uint16_t BODY;
        // uint16_t SEAT_FRONT_LEFT;
        // uint16_t SEAT_FRONT_RIGHT;
        // uint16_t SEAT_MID_LEFT;
        // uint16_t SEAT_MID_RIGHT;
        // uint16_t SEAT_BACK_LEFT;
        // uint16_t SEAT_BACK_RIGHT;
        RE_ADD_PADDING(0xC);
    };
    RE_VERIFY_SIZE(ZBoatMoveModelIF, 0xC);
}