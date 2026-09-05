#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/GUI/Font/SGlyph.h>


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
        // methods
        SHtmlState(ZFONT** pFonts);
        void SetDefault();
        uint32_t Color() const;
        int32_t Width(int iOffset) const;
        int32_t FontHeight() const;
        int32_t Indent(int iOffset) const;
        ZFONT* Font() const;
        int32_t LineSpacing() const;
        int32_t Kerning() const;
        int32_t ColumnHeight() const;
        int32_t ColumnTop() const;
        int32_t Gutter() const;
        int32_t Alignment() const;
        int32_t Caps() const;
        int32_t TabSize() const;

        // members
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
        ~ZSIMPLEHTML() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZCHAROBJ
        bool CreateGeometry() override;

        // methods
        ZSIMPLEHTML(const char* psName, ZBaseGeom* pBaseGeom);

        void SetTextRaw(const char* pszText);
        void SetFonts(const REFTAB32& fonts);
        void GetFonts(REFTAB32& fonts);
        static int32_t GetTagType(const char* pszTag);
        bool IsNonWord(uint32_t ch) const;
        bool IsWhiteSpace(uint32_t ch) const;
        bool IsPunctuation(uint32_t ch) const;
        int32_t ReadWord(const char* pszText, MYSTR& word) const;
        int32_t ReadAttribute(char* pszTag, const char* pszAttribute, MYSTR& value) const;
        void ParseImgTag(const char* pszTag, SHtmlState& state) const;
        int32_t ParseHtmlTag(const char*& pszText, SHtmlState& state) const;
        void AlignFaces(int32_t width, int32_t count, STempGlyph* pFaces, uint32_t alignment);
        bool CreateGeometryInternal();
        void FillJustify(ZStackArray<128, STempWord>& words, SHtmlState& state, uint32_t count, STempGlyph* pLast);
        void CenterJustify(SHtmlState& state, STempGlyph* pFirst, STempGlyph* pLast);
        void RightJustify(SHtmlState& state, STempGlyph* pFirst, STempGlyph* pLast);
        void Error(const char* pszFormat, ... ) const;

        // members
        ZFONT* m_aFonts[MAX_FONTS_NR];
        uint32_t m_iNumFonts;
        mutable const char* m_szError;
    };
    RE_VERIFY_SIZE(ZSIMPLEHTML, 0xC0);
}
