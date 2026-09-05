#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimStripsWintel.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimBoneStripsWintel : SPrimStripsWintel
    {
        uint32_t lNrBones;               // 0x7C
        uint32_t lLocalBones;            // 0x80
        uint32_t lLocalBonesQuats;       // 0x84
        uint32_t lGlobalBones;           // 0x88
        uint32_t lConvBones;             // 0x8C
        uint32_t lBoneDefinitions;       // 0x90
        uint32_t lBoneIdToIndexLookup;   // 0x94
        uint32_t lNrCopyBones;           // 0x98
        uint32_t lCopyBones;             // 0x9C
        uint32_t lBoneConstraintsHeader; // 0xA0
    };
    RE_VERIFY_SIZE(SPrimBoneStripsWintel, 0xA4);
}
