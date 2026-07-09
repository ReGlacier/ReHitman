#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <BloodMoney/Game/UI/ZCHAROBJ.h>


namespace Hitman::BloodMoney
{
    class ZFONT;

    struct SOffLimits 
    {
        int32_t iTop;
        int32_t iBottom;
        int32_t iLeft;
        int32_t iWidth;
        bool  bFullWidth;
    };
    RE_VERIFY_SIZE(SOffLimits, 0x14);

    struct SHtmlState 
    {
        // Size: 0x35c (860) bytes
        Glacier::ZSafeStackArray<16,unsigned int> ColorStack;
        Glacier::ZSafeStackArray<16,unsigned int> WidthStack;
        Glacier::ZSafeStackArray<16,unsigned int> IndentStack;
        Glacier::ZSafeStackArray<16,unsigned int> JustifyStack;
        Glacier::ZSafeStackArray<16,unsigned int> ColumnHeightStack;
        Glacier::ZSafeStackArray<16,unsigned int> ColumnTopStack;
        Glacier::ZSafeStackArray<16,unsigned int> FontStack;
        Glacier::ZSafeStackArray<16,unsigned int> LinespaceStack;
        Glacier::ZSafeStackArray<16,unsigned int> KerningStack;
        Glacier::ZSafeStackArray<16,unsigned int> GutterStack;
        Glacier::ZSafeStackArray<16,unsigned int> CapsStack;
        Glacier::ZSafeStackArray<16,unsigned int> TabSizeStack;
        SOffLimits OffLimits;
        float vPen[2];
        uint32_t iColumnLeft;
        uint32_t iCurrentLine;
        int32_t iBlockSkip;
        ZFONT** m_pFonts;
    };

    class ZSIMPLEHTML : public ZCHAROBJ
    {
    public:
        // const
        static constexpr size_t MAX_FONTS_NR = 6;

        // vtbl (no changes)
        // methods
        // data
        ZFONT* m_aFonts[MAX_FONTS_NR];
        uint32_t m_iNumFonts;
        const char* m_szError;
    };
    RE_VERIFY_SIZE(ZSIMPLEHTML, 0xC0);
}