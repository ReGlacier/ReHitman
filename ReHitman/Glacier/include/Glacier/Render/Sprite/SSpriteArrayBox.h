#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Sprite/SSpriteArray.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>

namespace Glacier
{
    struct SSpriteClipPlanes
    {
        uint32_t lNrClipPlanes;
        float ClipPlanes[24];
    };
    RE_VERIFY_SIZE(SSpriteClipPlanes, 0x64);

    struct SSpriteBox
    {
        ZVector3 vCornerBot0;
        ZVector3 vCornerBot1;
        ZVector3 vCornerTop0;
        ZVector3 vCornerTop1;
        ZVector3 vDirectionBot0;
        ZVector3 vDirectionBot1;
        ZVector3 vDirectionTop0;
        ZVector3 vDirectionTop1;
        uint32_t lMask;
        uint32_t Pad[3];
    };
    RE_VERIFY_SIZE(SSpriteBox, 0x70);

    struct SSpriteArrayBox : SSpriteArray
    {
        float fNear;
        float fFar;
        SSpriteClipPlanes* pSpriteClipPlanes;
        uint32_t lColor;
        float fDirectionScale;
        uint32_t lNrBoxes;
        SSpriteBox* pBoxes[32];
    };
    RE_VERIFY_SIZE(SSpriteArrayBox, 0xA4);
}
