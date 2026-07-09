#pragma once

#include <Glacier/ReGlacier.h>
#include <BloodMoney/Game/UI/ZFONT.h>


namespace Hitman::BloodMoney 
{
    struct SFontData 
    {
        uint32_t lFontSize;
        float fAspect;
        uint32_t lNumChars;
        uint32_t lMaxHeight;
        uint32_t lNumKerningPairs;
        uint32_t lNumTextures;
    };
    RE_VERIFY_SIZE(SFontData, 0x18);

    class ZTTFONT : public ZFONT
    {
    public:
        // vtbl (no changes)
        // methods
        static ZTTFONT* Create();

        // data (size is 0x98, base size is 0x88)
        uint32_t m_iDataHandle;
        SFontData* m_pFontData;
        SGlyph* m_pGlyphData;
        void* m_pKerningTable;
    };
    RE_VERIFY_SIZE(ZTTFONT, 0x98); // Verified
}