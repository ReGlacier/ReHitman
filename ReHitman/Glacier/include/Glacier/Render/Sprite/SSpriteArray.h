#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Sprite/SDrawArray.h>
#include <cstdint>


namespace Glacier
{
    struct SSpriteArrayElement;
    struct SSpriteArrayElementUV;
    struct SSpriteArrayElementRaw;
    struct SSpriteArrayElementParticle;

    struct SSpriteArray : public SDrawArray
    {
        uint32_t lNumSprites;
        uint32_t lDrawMask;
        union
        {
            SSpriteArrayElement* pSprites;
            SSpriteArrayElementUV* pSpritesUV;
            SSpriteArrayElementRaw* pSpritesRaw;
            SSpriteArrayElementParticle* pSpritesParticle;
        };
    };
    RE_VERIFY_SIZE(SSpriteArray, 0xC);
}