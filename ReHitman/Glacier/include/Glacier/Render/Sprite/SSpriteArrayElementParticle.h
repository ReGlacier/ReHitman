#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SSpriteArrayElementParticle
    {
        ZVector3 vC1;
        ZVector3 vC2;
        float fStartTime;
        float fEndTime;
        float fStartAngle;
        float fStartScale;
        float fDepthBias;
        float fAgeDepthBias;
        uint8_t u8Flags;
        uint8_t u8Phase;
        uint8_t u8OverdrawCtrl;
        uint8_t u8RoomIdx;
        uint8_t u8AttachIdx;
        uint8_t u8Pad01;
        uint8_t u8Pad02;
        uint8_t u8Pad03;
        void* piLightInfo;
    };
    RE_VERIFY_SIZE(SSpriteArrayElementParticle, 0x3C);
}
