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
        // methods
        ZBone();
        ZBone(const ZBone& copy);
        ZBone& operator=(const ZBone& copy);

        // members
        union
        {
            ZMat3x3 _Mat;
            ZQuat   _Quat;
        };
        ZVector3 _Pos;
    };
    RE_VERIFY_SIZE(ZBone, 0x30);
}
