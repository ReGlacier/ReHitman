#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/UTC4.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SGlyph
    {
        UTC4CHAR uChar;
        ZREF rTexture;
        int8_t vUV1[2];
        int8_t vUV2[2];
        uint8_t vSize[2];
        int8_t vStart[2];
        int8_t vAdvance;
        int8_t bIcon;
        int8_t _pad;
    };
    RE_VERIFY_SIZE(SGlyph, 0x14); // Verified by ZTTFONT::GetCharInfo (bsearch method contains size of entry)

    struct STempGlyph
    {
        uint32_t lColor;
        const SGlyph* pGlyph;
        ZVector2 vPos;
    };

    struct STempWord
    {
        STempGlyph* pBegin;
        uint32_t lHeight;
    };
}
