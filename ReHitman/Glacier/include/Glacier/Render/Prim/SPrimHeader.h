#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimHeader 
    {
        uint8_t lDrawDestination;
        uint8_t lPackType;
        uint16_t lType;
    };

    RE_VERIFY_OFFSET(SPrimHeader, lPackType, 0x1);
}