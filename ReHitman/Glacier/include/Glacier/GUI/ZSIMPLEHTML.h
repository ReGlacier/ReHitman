#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/GUI/ZCHAROBJ.h>


namespace Glacier
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
        ZSafeStackArray<16,unsigned int> ColorStack;
        ZSafeStackArray<16,unsigned int> WidthStack;
        ZSafeStackArray<16,unsigned int> IndentStack;
        ZSafeStackArray<16,unsigned int> JustifyStack;
        ZSafeStackArray<16,unsigned int> ColumnHeightStack;
        ZSafeStackArray<16,unsigned int> ColumnTopStack;
        ZSafeStackArray<16,unsigned int> FontStack;
        ZSafeStackArray<16,unsigned int> LinespaceStack;
        ZSafeStackArray<16,unsigned int> KerningStack;
        ZSafeStackArray<16,unsigned int> GutterStack;
        ZSafeStackArray<16,unsigned int> CapsStack;
        ZSafeStackArray<16,unsigned int> TabSizeStack;
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
        // RTTI
        DECLARE_GEOM_CLASS(ZSIMPLEHTML, 0x20004Au);

        // const
        static constexpr size_t MAX_FONTS_NR = 6;

        // vtbl
        // methods
        // members
        ZFONT* m_aFonts[MAX_FONTS_NR];
        uint32_t m_iNumFonts;
        const char* m_szError;
    };
    RE_VERIFY_SIZE(ZSIMPLEHTML, 0xC0);
}
