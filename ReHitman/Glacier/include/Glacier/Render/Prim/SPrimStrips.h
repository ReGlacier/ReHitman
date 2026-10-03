#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimDrawSetup.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimStrips : SPrimDrawSetup
    {
        uint16_t lNrStrips;
        uint16_t lNrEntries;
        uint32_t lVertices;
        uint32_t lColors;
        uint32_t lKeys;
        uint32_t lInsideCheckData;
        uint32_t lColiId;
        uint32_t lColiBits;       
    };
    RE_VERIFY_SIZE(SPrimStrips, 0x38);
}