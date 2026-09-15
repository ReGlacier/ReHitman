#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/GUI/Font/ZFONT.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        unsigned int GetLineChar(const char*& text)
        {
            const uint8_t first = static_cast<uint8_t>(*text++);
            if (first < 0x80) return first;
            if ((first & 0xE0) == 0xC0) return ((first & 0x1F) << 6) | (*text++ & 0x3F);
            if ((first & 0xF0) == 0xE0) return ((first & 0x0F) << 12) | ((*text++ & 0x3F) << 6) | (*text++ & 0x3F);
            return ((first & 7) << 18) | ((*text++ & 0x3F) << 12) | ((*text++ & 0x3F) << 6) | (*text++ & 0x3F);
        }

        const char* FindPreviousChar(const char* text, int maxBytes)
        {
            for (int i = 0; i < maxBytes; ++i, --text)
            {
                if ((*text & 0xC0) != 0x80)
                    return text;
            }
            return text;
        }
    }

    ZLINEOBJ::ZLINEOBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZCHAROBJ(psName, pBaseGeom)
        , m_szText(nullptr)
        , m_iWidth(0)
        , m_iNumberOfLines(0)
    {
    }

    ZLINEOBJ::~ZLINEOBJ()
    {
        ZUniMemory::Free(m_szText);
    }

    const RTP::ZPropertyInfo& ZLINEOBJ::GetProperties() const { return Info; }
    uint32_t ZLINEOBJ::GetObjectId() const { return m_Id; }
    void ZLINEOBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const { id = m_Id; mask = m_Mask; }
    ZGEOMCLASSINFO* ZLINEOBJ::GetOldClassInfo() const { return m_OldClassInfo; }

    bool ZLINEOBJ::SetText(const char* text)
    {
        ZUniMemory::Free(m_szText);
        m_szText = static_cast<char*>(ZUniMemory::Allocate(static_cast<int>(std::strlen(text) + 1)));
        std::strcpy(m_szText, text);
        return ZCHAROBJ::SetText(m_szText);
    }

    void ZLINEOBJ::Clear()
    {
        ZCHAROBJ::Clear();
        ZUniMemory::Free(m_szText);
        m_szText = nullptr;
    }

    bool ZLINEOBJ::Wrap()
    {
        if (!m_iWidth || !m_szText)
            return ZCHAROBJ::SetText("");

        int rWindows = 0;
        g_pEngineData->GetSceneCom()->GetVal("rWindows", &rWindows);
        const auto* pWindows = static_cast<ZWINDOWS*>(ZGEOM::RefToPtr(rWindows));
        const int width = static_cast<int>(m_iWidth * (pWindows ? pWindows->m_fBgScale : 1.0f));

        MYSTR formatted;
        const char* line = m_szText;
        m_iNumberOfLines = 1;

        while (line && *line)
        {
            const char* cursor = line;
            const char* lineEnd = line;
            int remaining = width;
            int charCount = -1;

            while (remaining >= 0 && *cursor)
            {
                lineEnd = cursor;
                ++charCount;
                const unsigned int character = GetLineChar(cursor);
                if (character == '\n' || character == '\r')
                {
                    ++m_iNumberOfLines;
                    break;
                }
                remaining -= m_pFont->GetCharInfo(character)->vAdvance;
            }

            const char* next = lineEnd;
            const bool nextIsNewline = *next == '\n' || *next == '\r';
            if (charCount < 0 && !nextIsNewline)
                return false;

            bool forceBreak = false;
            int lineLength = static_cast<int>(lineEnd - line);
            if (width < GetStringWidth(line, charCount + 1))
            {
                ++m_iNumberOfLines;
                const char* previous = FindPreviousChar(lineEnd - 1, 5);
                int skipped = 0;
                while (skipped < charCount && skipped < 26)
                {
                    const char* candidate = FindPreviousChar(previous - 1, 5);
                    const char* character = candidate;
                    const unsigned int value = GetLineChar(character);
                    if (!value || value == '\n' || value == '\t' || value == ' ')
                        break;
                    previous = candidate;
                    ++skipped;
                }

                if (skipped < 26 && skipped < charCount)
                {
                    charCount -= skipped;
                    lineLength = static_cast<int>(previous - line);
                }
                else
                {
                    forceBreak = true;
                }
            }

            char* chunk = static_cast<char*>(ZUniMemory::Allocate(lineLength + 1));
            std::memcpy(chunk, line, lineLength);
            chunk[lineLength] = '\0';
            formatted += MYSTR(chunk);
            ZUniMemory::Free(chunk);
            const char delimiter = line[lineLength];
            if (delimiter == '\n' || delimiter == '\r' || delimiter == '\t' || delimiter == ' ')
            {
                formatted += MYSTR("\n");
                line += lineLength + 1;
                if (delimiter == '\r' && *line == '\n')
                    ++line;
            }
            else
            {
                if (forceBreak)
                    formatted += MYSTR("\n");
                line += lineLength;
            }
        }

        return ZCHAROBJ::SetText(static_cast<const char*>(formatted));
    }
    int ZLINEOBJ::GetCharWidth(const char** text)
    {
        return m_pFont->GetCharInfo(GetLineChar(*text))->vAdvance;
    }
    int ZLINEOBJ::GetStringWidth(const char* text, int length)
    {
        int width = 0;
        while (*text && length--)
            width += GetCharWidth(&text);
        return width;
    }
    void ZLINEOBJ::SetWidth(int width) { m_iWidth = width; if (m_szText) Wrap(); }
    float ZLINEOBJ::GetWidth() { return static_cast<float>(m_iWidth); }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<int> Width
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "m_iWidth", .m_Filter = 1 },
            .m_VirtualTable = VirtualTable_DP__7,
            .m_Offset = CLASS_PROPERTY(ZLINEOBJ, m_iWidth)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZLINEOBJ,
        ZCHAROBJ,
        0x009A2BE8,
        "ZLINEOBJ",
        0x0077E7E0,
        cProperties::Width,
        0x008121D8,
        0x009A2B98,
        0x009A2B9C
    );
#   pragma endregion
}
