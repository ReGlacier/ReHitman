#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    using BoneIndex = uint8_t;
    enum EBoneIndex : BoneIndex { eBoneIndexNA = 0xFFu };
    using BoneID = uint16_t;

    struct ZBone
    {
        ZMat3x3 _Mat;
        ZVector3 _Pos;
    };
    RE_VERIFY_SIZE(ZBone, 0x30);
}