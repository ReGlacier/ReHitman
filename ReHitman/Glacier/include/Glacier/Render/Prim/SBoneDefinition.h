#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SBoneDefinition
    {
        ZVector3 Center;
        uint32_t lPrevBoneNr;
        ZVector3 Size;
        char Name[34];
        uint8_t Id;
        uint8_t BodyPart;
    };
    RE_VERIFY_SIZE(SBoneDefinition, 0x40);
}