#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // Bones property blob referenced by SPrimObjectHeader::lPropertyData
    // when HAS_BONES is set. Layout taken from the PC build.
    struct SPropertyBones
    {
        uint32_t lNumBones;
        uint32_t lBoneDefinitions;
        uint32_t lGlobalBones;
        uint32_t lLocalBones;
        uint32_t lConvBones;
        uint32_t lBoneIdToIndexLookup;
        uint32_t lLocalBonesQuats;
        uint32_t lBoneConstraintsHeader;
        uint32_t lBoneIdToIndexLookup2;
        uint32_t lBoneIndexToIdLookup2;
        uint32_t lBoneIdToPosLookup;
        uint32_t lBonePoseHeader;
        uint32_t stateModel;
        uint32_t lParentLookup;
        uint16_t lNumBonesUsedLOD[8];
        uint32_t lInvertGlobalBones;
        uint32_t lParentBones;
    };
    RE_VERIFY_SIZE(SPropertyBones, 0x50);
}
