#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <cstdint>

namespace Glacier
{
    struct ZCarMoveModelIF 
    {
        uint16_t BODY;
        uint16_t WHEEL_FRONT_LEFT;
        uint16_t WHEEL_FRONT_RIGHT;
        uint16_t WHEEL_BACK_LEFT;
        uint16_t WHEEL_BACK_RIGHT;
        uint16_t WHEEL_BACK_LEFT_2;
        uint16_t WHEEL_BACK_RIGHT_;
        uint16_t SEAT_FRONT_LEFT;
        uint16_t SEAT_FRONT_RIGHT;
        uint16_t SEAT_BACK_LEFT;
        uint16_t SEAT_BACK_RIGHT;
        uint16_t DOOR_FRONT_LEFT;
        uint16_t DOOR_FRONT_RIGHT;
        uint16_t DOOR_BACK_LEFT;
        uint16_t DOOR_BACK_RIGHT;
        uint16_t WHEELS_BEGIN_INDE;
        uint16_t WHEELS_END_INDEX;
        ZStaticVector<uint16_t, 8> AllWheels;
    };
    RE_VERIFY_SIZE(ZCarMoveModelIF, 0x38); // Verified
}