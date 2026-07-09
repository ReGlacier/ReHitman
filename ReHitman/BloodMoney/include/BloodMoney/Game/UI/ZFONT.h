#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/UI/ZWINOBJ.h>

namespace Hitman::BloodMoney 
{
    using UTC4CHAR = uint32_t;

    struct SGlyph 
    {
        UTC4CHAR uChar;
        Glacier::ZREF rTexture;
        int8_t vUV1[2];
        int8_t vUV2[2];
        uint8_t vSize[2];
        int8_t vStart[2];
        int8_t vAdvance;
        int8_t bIcon;
        int8_t _pad;
    };
    RE_VERIFY_SIZE(SGlyph, 0x14); // Verified by ZTTFONT::GetCharInfo (bsearch method contains size of entry)

    enum EFontFilterMode {
        FFM_Normal,
        FFM_Add,
        FFM_Sub,
        FFM_Invert
    };

    class ZFONT : public ZWINOBJ 
    {
    public:
        // vftable - 5 pure virtual methods
        virtual int GetNumTextures();
        virtual SGlyph* GetCharInfo(unsigned int);
        virtual int GetMaxHeight();
        virtual bool ContainsChar(unsigned int);
        virtual void* GetKerning(unsigned int, unsigned int);
        // data (size is 0x88, base size is 0x88)
    };
    RE_VERIFY_SIZE(ZFONT, 0x88);
}