#include <Glacier/GUI/ZSIMPLEHTML.h>
#include <Glacier/Geom/ZGROUP.h>

#include <Glacier/GUI/Font/ZFONT.h>
#include <Glacier/GUI/ZWinFaces.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/ZUniAssert.h>
#include <cctype>
#include <clocale>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace Glacier
{
    ZSIMPLEHTML::ZSIMPLEHTML(const char* psName, ZBaseGeom* pBaseGeom)
        : ZCHAROBJ(psName, pBaseGeom),
          m_aFonts{},
          m_iNumFonts(0),
          m_szError(nullptr)
    {
    }

    ZSIMPLEHTML::~ZSIMPLEHTML() = default;

    const RTP::ZPropertyInfo& ZSIMPLEHTML::GetProperties() const
    {
        return ZSIMPLEHTML::Info;
    }

    uint32_t ZSIMPLEHTML::GetObjectId() const
    {
        return ZSIMPLEHTML::m_Id;
    }

    void ZSIMPLEHTML::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSIMPLEHTML::m_Id;
        mask = ZSIMPLEHTML::m_Mask;
    }

    ZGEOMCLASSINFO* ZSIMPLEHTML::GetOldClassInfo() const
    {
        return ZSIMPLEHTML::m_OldClassInfo;
    }

    void ZSIMPLEHTML::SetTextRaw(const char* pszText)
    {
        if (!pszText)
            return;
        if (m_szTextPtr && std::strcmp(m_szTextPtr, pszText) == 0)
            return;
        m_szTextPtr = pszText;
        m_szStringId = nullptr;
        if (*pszText)
            ZCHAROBJ::CreateGeometry();
        else
            Clear();
    }

    void ZSIMPLEHTML::SetFonts(const REFTAB32& fonts)
    {
        m_iNumFonts = 0;
        const int count = static_cast<int>(fonts.Count());
        ZASSERT(count <= static_cast<int>(MAX_FONTS_NR));
        for (int i = 0; i < count && i < static_cast<int>(MAX_FONTS_NR); ++i)
        {
            ZGEOM* geom = ZGEOM::RefToPtr(fonts[i]);
            if (!geom)
                continue;
            ZASSERT(geom->IsDerivedFrom<ZFONT>());
            m_aFonts[m_iNumFonts++] = static_cast<ZFONT*>(geom);
        }
    }

    void ZSIMPLEHTML::GetFonts(REFTAB32& fonts)
    {
        fonts.Clear();
        for (uint32_t i = 0; i < m_iNumFonts; ++i)
            fonts.Add(m_aFonts[i] ? m_aFonts[i]->GetObjectId() : 0);
    }

    bool ZSIMPLEHTML::CreateGeometry()
    {
        char error[0x400]{};
        m_szError = error;
        std::setlocale(LC_ALL, "danish");
        const bool result = CreateGeometryInternal();
        std::setlocale(LC_ALL, "C");
        return result;
    }

    bool ZSIMPLEHTML::CreateGeometryInternal()
    {
        if (!m_pFont || !m_szTextPtr)
            return false;

        const char* text = m_szTextPtr;
        const size_t textLength = std::strlen(text);
        auto* faces = static_cast<STempGlyph*>(ZUniMemory::Allocate(sizeof(STempGlyph) * (textLength + 1)));
        if (!faces)
            return false;

        SHtmlState state(m_aFonts);
        ZStackArray<128, STempWord> words;
        uint32_t count = 0;
        float penX = 0.0f;
        float penY = 0.0f;
        UTC4CHAR previous = 0;
        state.vPen[0] = 0.0f;
        state.vPen[1] = 0.0f;

        for (const char* cursor = text; *cursor && count <= textLength;)
        {
            if (*cursor == '<')
            {
                const char* tag = cursor + 1;
                const int32_t result = ParseHtmlTag(tag, state);
                if (result >= 0)
                {
                    cursor = tag;
                    continue;
                }
            }
            const UTC4CHAR ch = static_cast<unsigned char>(*cursor++);
            if (ch == '\r')
                continue;
            if (ch == '\n')
            {
                penX = 0.0f;
                penY += static_cast<float>(state.FontHeight()) + state.LineSpacing();
                previous = 0;
                continue;
            }
            if (ch == '\t')
            {
                const int tab = state.TabSize();
                penX += tab - static_cast<int>(penX) % tab;
                previous = 0;
                continue;
            }
            if (ch == ' ')
            {
                penX += m_pFont->GetCharInfo(' ')->vAdvance;
                previous = 0;
                continue;
            }
            SGlyph* glyph = state.Font()->GetCharInfo(ch);
            if (!glyph)
                continue;
            if (previous)
                penX -= state.Font()->GetKerning(previous, ch);
            faces[count] = {state.Color(), glyph, {penX, penY}};
            penX += glyph->vAdvance;
            previous = ch;
            ++count;
        }

        AlignFaces(static_cast<int32_t>(penX), static_cast<int32_t>(count), faces, state.Alignment());
        m_pSprites = g_pRenderDll->m_pPrimControl->AllocSpriteArrayUV(count);
        if (!m_pSprites && count)
        {
            ZUniMemory::Free(faces);
            return false;
        }
        auto* textures = static_cast<STextureRef*>(ZUniMemory::Allocate(sizeof(STextureRef) * count));
        for (uint32_t i = 0; i < count; ++i)
        {
            const SGlyph& glyph = *faces[i].pGlyph;
            m_pSprites[i].p = {faces[i].vPos.x + glyph.vStart[0] + glyph.vSize[0] * 0.5f,
                -(faces[i].vPos.y + glyph.vStart[1]) - glyph.vSize[1] * 0.5f,
                0.0f};
            m_pSprites[i].fScale = {static_cast<float>(glyph.vSize[0]), static_cast<float>(glyph.vSize[1])};
            m_pSprites[i].u = {static_cast<float>(glyph.vUV1[0]) / 128.0f, static_cast<float>(glyph.vUV2[0]) / 128.0f};
            m_pSprites[i].v = {static_cast<float>(glyph.vUV1[1]) / 128.0f, static_cast<float>(glyph.vUV2[1]) / 128.0f};
            m_pSprites[i].lColor = faces[i].lColor;
            textures[i] = {glyph.rTexture, i};
        }
        m_iNumSprites = static_cast<uint16_t>(count);
        m_iNumSpriteArrays = static_cast<uint16_t>(CreateWinFaces(textures, m_pSprites, m_pPrims, m_pSpriteArrays, count, m_dwDrawMode, this));
        ZUniMemory::Free(textures);
        ZUniMemory::Free(faces);
        CalcCenSize();
        m_bModified = true;
        return true;
    }

    SHtmlState::SHtmlState(ZFONT** pFonts)
        : m_pFonts(pFonts)
    {
        SetDefault();
    }

    int32_t ZSIMPLEHTML::GetTagType(const char* pszTag)
    {
        if (!pszTag)
            return -1;
        uint32_t code = 0;
        for (uint32_t i = 0; i < 4 && pszTag[i]; ++i)
            code = (code << 8) | static_cast<unsigned char>(pszTag[i]);
        switch (code)
        {
        case 0x2F4C494Eu:
            return 11; // /LIN
        case 0x2F424546u:
            return 25; // /BEF
        case 0x2F494E44u:
            return 7; // /IND
        case 0x2F4A5553u:
            return 15; // /JUS
        case 0x2F4B4552u:
            return 13; // /KER
        case 0x2F475554u:
            return 17; // /GUT
        case 0x2F434150u:
            return 21; // /CAP
        case 0x2F434F4Cu:
            return 1; // /COL
        case 0x2F464F4Eu:
            return 9; // /FON
        case 0x2F415554u:
            return 3; // /AUT
        case 0x2F494D47u:
            return 26; // /IMG
        case 0x212D2Du:
            return 18; // !--
        case 0x4252u:
            return 28; // BR
        case 0x4754u:
            return 30; // GT
        case 0x4C54u:
            return 29; // LT
        case 0x494D47u:
            return 27; // IMG
        case 0x43415053u:
            return 20; // CAPS
        case 0x434F4C4Fu:
            return 0; // COLO
        case 0x464F4E54u:
            return 8; // FONT
        case 0x4245464Fu:
            return 24; // BEFO
        case 0x2F544142u:
            return 23; // /TAB
        case 0x2F574944u:
            return 5; // /WID
        case 0x4155544Fu:
            return 2; // AUTO
        case 0x4C494E45u:
            return 10; // LINE
        case 0x494E4445u:
            return 6; // INDE
        case 0x4A555354u:
            return 14; // JUST
        case 0x4B45524Eu:
            return 12; // KERN
        case 0x54414253u:
            return 22; // TABS
        case 0x57494454u:
            return 4; // WIDT
        case 0x47555454u:
            return 16; // GUTT
        case 0x53504143u:
            return 19; // SPAC
        default:
            return -1;
        }
    }

    bool ZSIMPLEHTML::IsWhiteSpace(uint32_t ch) const
    {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
    }

    bool ZSIMPLEHTML::IsNonWord(uint32_t ch) const
    {
        return IsWhiteSpace(ch) || ch == '=' || ch == '>' || ch == 0;
    }

    bool ZSIMPLEHTML::IsPunctuation(uint32_t ch) const
    {
        return ch == '.' || ch == ',' || ch == ':' || ch == ';' || ch == '-' || ch == '"' || ch == '\'';
    }

    int32_t ZSIMPLEHTML::ReadWord(const char* pszText, MYSTR& word) const
    {
        if (!pszText)
            return 0;
        if (*pszText == '"')
        {
            const char* end = std::strchr(pszText + 1, '"');
            if (!end)
                return 0;
            const_cast<char*>(end)[0] = 0;
            word = pszText + 1;
            const_cast<char*>(end)[0] = '"';
            return static_cast<int32_t>(end + 1 - pszText);
        }
        const char* end = pszText;
        while (*end && !IsNonWord(static_cast<unsigned char>(*end)))
            ++end;
        word = MYSTR();
        for (const char* p = pszText; p != end; ++p)
        {
            char value[2] = {*p, 0};
            word += MYSTR(value);
        }
        return static_cast<int32_t>(end - pszText);
    }

    int32_t ZSIMPLEHTML::ReadAttribute(char* pszTag, const char* pszAttribute, MYSTR& value) const
    {
        if (!pszTag || !pszAttribute)
            return 0;
        char* end = std::strchr(pszTag, '>');
        if (!end)
            return 0;
        const char saved = *end;
        *end = 0;
        const size_t attributeLength = std::strlen(pszAttribute);
        for (char* p = pszTag; *p; ++p)
        {
            bool matches = true;
            for (size_t i = 0; i < attributeLength; ++i)
                if (!p[i] || std::tolower(static_cast<unsigned char>(p[i])) != std::tolower(static_cast<unsigned char>(pszAttribute[i])))
                    matches = false;
            if (!matches || (p != pszTag && !IsNonWord(static_cast<unsigned char>(p[-1]))) ||
                !IsNonWord(static_cast<unsigned char>(p[attributeLength])))
                continue;
            p += attributeLength;
            while (IsWhiteSpace(static_cast<unsigned char>(*p)) || *p == '=')
                ++p;
            const int32_t result = ReadWord(p, value);
            *end = saved;
            return result != 0;
        }
        *end = saved;
        return 0;
    }

    void ZSIMPLEHTML::ParseImgTag(const char* pszTag, SHtmlState& state) const
    {
        MYSTR source;
        if (!ReadAttribute(const_cast<char*>(pszTag), "SRC", source))
            return;

        ZGROUP* parent = static_cast<ZGROUP*>(BaseGeom()->ParentGroup());
        if (!parent)
            return;
        ZGEOM* image = parent->FindGeom(static_cast<const char*>(source), nullptr);
        if (!image)
            return;

        if (image->BaseGeom()->Control() & 0x400)
            image->Activate(true);

        MYSTR align;
        int alignment = 0;
        if (ReadAttribute(const_cast<char*>(pszTag), "ALIGN", align))
        {
            if (std::strcmp(static_cast<const char*>(align), "Left") == 0)
                alignment = 1;
            else if (std::strcmp(static_cast<const char*>(align), "Right") == 0)
                alignment = 2;
            else if (std::strcmp(static_cast<const char*>(align), "Column") == 0)
                alignment = 4;
        }

        int border[4] = {0, 0, 0, 0};
        MYSTR borderText;
        if (ReadAttribute(const_cast<char*>(pszTag), "BORDER", borderText))
        {
            int count = std::sscanf(static_cast<const char*>(borderText), "%d,%d,%d,%d", &border[0], &border[1], &border[2], &border[3]);
            if (count <= 0)
                border[0] = 8;
            if (count <= 1)
                border[1] = border[0];
            if (count <= 2)
                border[2] = border[1];
            if (count <= 3)
                border[3] = border[2];
        }

        ZVector3 size;
        ZVector3 cen;
        image->GetSize(size);
        image->GetCen(cen);
        const int32_t width = state.Width(0);
        const int32_t indent = state.Indent(0);
        const int32_t fontHeight = state.FontHeight();
        const int32_t top = static_cast<int32_t>(state.vPen[1]) - fontHeight;
        const int32_t bottom = static_cast<int32_t>(state.vPen[1]) + border[1];
        ZVector3 localPosition{};

        if (alignment == 1)
        {
            localPosition.x = static_cast<float>(indent) - border[0] - size.x - cen.x;
            localPosition.y = static_cast<float>(bottom);
        }
        else if (alignment == 2)
        {
            localPosition.x = static_cast<float>(indent + width) - size.x - border[2] - cen.x;
            localPosition.y = static_cast<float>(bottom);
        }
        else
        {
            localPosition.x = static_cast<float>(indent + width) - cen.x;
            localPosition.y = static_cast<float>(bottom);
        }

        state.OffLimits.iTop = top;
        state.OffLimits.iBottom = static_cast<int32_t>(localPosition.y + size.y + border[3]);
        state.OffLimits.iLeft = static_cast<int32_t>(localPosition.x - size.x - border[0]);
        state.OffLimits.iWidth = static_cast<int32_t>(localPosition.x + size.x + border[2]);
        state.OffLimits.bFullWidth = alignment == 4;
        state.vPen[1] = localPosition.y;

        ZVector3 rootPosition = localPosition;
        BaseGeom()->GetRootPoint(rootPosition);
        image->GetLocalPoint(rootPosition);
        image->SetPos(rootPosition);
        image->Hide(false);
        if (state.OffLimits.iWidth < width + 30)
            state.iBlockSkip = 1;
    }

    int32_t ZSIMPLEHTML::ParseHtmlTag(const char*& pszText, SHtmlState& state) const
    {
        if (!pszText)
            return -1;
        const char* begin = pszText;
        const char* end = std::strchr(pszText, '>');
        if (!end)
            return -1;
        char tag[128]{};
        const size_t length = static_cast<size_t>(end - begin);
        std::memcpy(tag, begin, length < sizeof(tag) - 1 ? length : sizeof(tag) - 1);
        const int32_t type = GetTagType(tag);
        pszText = end + 1;
        if (type < 0)
            return -1;
        const char* payload = begin;
        while (*payload && !IsWhiteSpace(static_cast<unsigned char>(*payload)) && *payload != '=')
            ++payload;
        while (IsWhiteSpace(static_cast<unsigned char>(*payload)) || *payload == '=')
            ++payload;
        switch (type)
        {
        case 0:
            state.ColorStack.Push(std::strtoul(payload, nullptr, 16));
            break;
        case 1:
            state.ColorStack.PopSafe();
            break;
        case 2:
            state.ColumnHeightStack.Push(std::strtol(payload, nullptr, 10));
            break;
        case 3:
            state.ColumnHeightStack.PopSafe();
            break;
        case 4:
            state.WidthStack.Push(std::strtol(payload, nullptr, 10));
            break;
        case 5:
            state.WidthStack.PopSafe();
            break;
        case 6:
            state.IndentStack.Push(std::strtol(payload, nullptr, 10));
            break;
        case 7:
            state.IndentStack.PopSafe();
            break;
        case 8:
        {
            const int font = std::strtol(payload, nullptr, 10) - 1;
            if (font >= 0 && font < static_cast<int>(MAX_FONTS_NR) && m_aFonts[font])
                state.FontStack.Push(static_cast<uint32_t>(font));
            break;
        }
        case 9:
            state.FontStack.PopSafe();
            break;
        case 10:
            state.LinespaceStack.Push(std::strtol(payload, nullptr, 10));
            break;
        case 11:
        case 13:
            state.LinespaceStack.PopSafe();
            break;
        case 12:
            state.KerningStack.Push(std::strtol(payload, nullptr, 10));
            break;
        case 14:
            state.JustifyStack.Push(1);
            break;
        case 15:
            state.JustifyStack.PopSafe();
            break;
        case 16:
            state.GutterStack.Push(std::strtol(payload, nullptr, 10));
            break;
        case 17:
            state.GutterStack.PopSafe();
            break;
        case 18:
        case 19:
            break;
        case 20:
            state.CapsStack.Push(1);
            break;
        case 21:
            state.CapsStack.PopSafe();
            break;
        case 22:
            state.TabSizeStack.Push(std::strtol(payload, nullptr, 10));
            break;
        case 23:
            state.TabSizeStack.PopSafe();
            break;
        case 26:
        case 28:
        case 29:
        case 30:
            break;
        case 27:
            ParseImgTag(begin, state);
            break;
        default:
            return -1;
        }
        return 0;
    }

    void ZSIMPLEHTML::AlignFaces(int32_t width, int32_t count, STempGlyph* faces, uint32_t alignment)
    {
        if (!faces || !(alignment & 0xF) || (alignment & 1))
            return;
        const float offset = alignment & 4 ? width * 0.5f : static_cast<float>(width);
        for (int32_t i = 0; i < count; ++i)
            faces[i].vPos.x -= offset;
    }

    void ZSIMPLEHTML::FillJustify(ZStackArray<128, STempWord>& words, SHtmlState& state, uint32_t count, STempGlyph* last)
    {
        if (words.Count() < 2 || !last)
            return;
        const STempWord& word = *words.Peek();
        const float first = word.pBegin->vPos.x;
        const float used = last->vPos.x + last->pGlyph->vSize[0] - first;
        const float available = static_cast<float>(state.Width(0) - state.Indent(0)) - used;
        const float gap = available / static_cast<float>(words.Count() - 2);
        float shift = 0.0f;
        for (uint32_t i = 0; i + 1 < words.Count(); ++i)
        {
            STempWord& current = *words.Get(i);
            STempWord& next = *words.Get(i + 1);
            for (STempGlyph* face = current.pBegin; face < next.pBegin; ++face)
                face->vPos.x += shift;
            shift += gap;
        }
    }

    void ZSIMPLEHTML::CenterJustify(SHtmlState& state, STempGlyph* first, STempGlyph* last)
    {
        if (!first || !last)
            return;
        const float lineWidth = last->vPos.x + last->pGlyph->vSize[0] - first->vPos.x;
        const float offset = (static_cast<float>(state.Width(0) - state.Indent(0)) - lineWidth) * 0.5f;
        for (STempGlyph* face = first; face <= last; ++face)
            face->vPos.x += offset;
    }

    void ZSIMPLEHTML::RightJustify(SHtmlState& state, STempGlyph* first, STempGlyph* last)
    {
        if (!first || !last)
            return;
        const float lineWidth = last->vPos.x + last->pGlyph->vSize[0] - first->vPos.x;
        const float offset = static_cast<float>(state.Width(0) - state.Indent(0)) - lineWidth;
        for (STempGlyph* face = first; face <= last; ++face)
            face->vPos.x += offset;
    }
    void ZSIMPLEHTML::Error(const char* pszFormat, ...) const
    {
        va_list args;
        va_start(args, pszFormat);
        std::vsprintf(const_cast<char*>(m_szError), pszFormat, args);
        va_end(args);
    }

    void SHtmlState::SetDefault()
    {
        ColorStack.Clear();
        WidthStack.Clear();
        IndentStack.Clear();
        JustifyStack.Clear();
        ColumnHeightStack.Clear();
        ColumnTopStack.Clear();
        FontStack.Clear();
        LinespaceStack.Clear();
        KerningStack.Clear();
        GutterStack.Clear();
        CapsStack.Clear();
        TabSizeStack.Clear();
        OffLimits = {0, 0, 0, 0, false};
        vPen[0] = 0.0f;
        vPen[1] = 0.0f;
        iColumnLeft = 0;
        iCurrentLine = 1;
        iBlockSkip = 0;
        ColorStack.Push(0xFF000000);
        WidthStack.Push(450);
        IndentStack.Push(0);
        JustifyStack.Push(1);
        ColumnHeightStack.Push(0);
        ColumnTopStack.Push(0);
        FontStack.Push(0);
        LinespaceStack.Push(0);
        KerningStack.Push(-10);
        GutterStack.Push(0);
        CapsStack.Push(0);
        TabSizeStack.Push(50);
    }

    uint32_t SHtmlState::Color() const
    {
        return *ColorStack.Peek();
    }
    int32_t SHtmlState::Width(int iOffset) const
    {
        const int32_t width = static_cast<int32_t>(*WidthStack.Peek()) - (ColumnHeight() ? Gutter() : 0);
        const float end = static_cast<float>(iOffset) + vPen[0];
        const int32_t height = FontHeight();
        return end - height >= OffLimits.iBottom || OffLimits.iTop >= end || OffLimits.iLeft >= width
            ? width
            : OffLimits.iWidth;
    }
    int32_t SHtmlState::FontHeight() const
    {
        return Font()->GetCharInfo('W')->vSize[1];
    }
    int32_t SHtmlState::Indent(int iOffset) const
    {
        return static_cast<float>(iOffset) + vPen[0] >= OffLimits.iBottom || OffLimits.iTop >= static_cast<float>(iOffset) + vPen[0]
            ? static_cast<int32_t>(*IndentStack.Peek())
            : OffLimits.iLeft;
    }
    ZFONT* SHtmlState::Font() const
    {
        return m_pFonts[*FontStack.Peek()];
    }
    int32_t SHtmlState::LineSpacing() const
    {
        return static_cast<int32_t>(*LinespaceStack.Peek());
    }
    int32_t SHtmlState::Kerning() const
    {
        return static_cast<int32_t>(*KerningStack.Peek());
    }
    int32_t SHtmlState::ColumnHeight() const
    {
        return static_cast<int32_t>(*ColumnHeightStack.Peek());
    }
    int32_t SHtmlState::ColumnTop() const
    {
        return static_cast<int32_t>(*ColumnTopStack.Peek());
    }
    int32_t SHtmlState::Gutter() const
    {
        return static_cast<int32_t>(*GutterStack.Peek());
    }
    int32_t SHtmlState::Alignment() const
    {
        return static_cast<int32_t>(*JustifyStack.Peek());
    }
    int32_t SHtmlState::Caps() const
    {
        return static_cast<int32_t>(*CapsStack.Peek());
    }
    int32_t SHtmlState::TabSize() const
    {
        return static_cast<int32_t>(*TabSizeStack.Peek());
    }

#pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<REFTAB32> Fonts{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_pvFontsList", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Virtual_REFTAB32,
            .m_Get = &ZSIMPLEHTML::GetFonts,
            .m_Set = &ZSIMPLEHTML::SetFonts};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZSIMPLEHTML,
        ZCHAROBJ,
        0x009A2DA0,
        "ZSIMPLEHTML",
        0x0077F61C,
        cProperties::Fonts,
        0x0081345C,
        0x009A2D50,
        0x009A2D54);
#pragma endregion
}
