#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/GUI/Font/ZFONT.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/GUI/ZWinFaces.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    namespace
    {
        UTC4CHAR GetNextChar(const char*& text)
        {
            const uint8_t first = static_cast<uint8_t>(*text++);
            if (first < 0x80)
                return first;
            if ((first & 0xE0) == 0xC0)
                return ((first & 0x1F) << 6) | (*text++ & 0x3F);
            if ((first & 0xF0) == 0xE0)
                return ((first & 0x0F) << 12) | ((*text++ & 0x3F) << 6) | (*text++ & 0x3F);
            return ((first & 7) << 18) | ((*text++ & 0x3F) << 12) | ((*text++ & 0x3F) << 6) | (*text++ & 0x3F);
        }
    }

    ZCHAROBJ::ZCHAROBJ(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINOBJ(psName, pBaseGeom),
          m_iLineSpacing(0),
          m_pFont(nullptr),
          m_iTabSize(64),
          m_iMonoSpaceSize(-1),
          m_iSpacingAdd(0)
    {
    }

    ZCHAROBJ::~ZCHAROBJ() = default;
    const RTP::ZPropertyInfo& ZCHAROBJ::GetProperties() const
    {
        return ZCHAROBJ::Info;
    }
    uint32_t ZCHAROBJ::GetObjectId() const
    {
        return ZCHAROBJ::m_Id;
    }
    void ZCHAROBJ::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZCHAROBJ::m_Id;
        mask = ZCHAROBJ::m_Mask;
    }
    ZGEOMCLASSINFO* ZCHAROBJ::GetOldClassInfo() const
    {
        return ZCHAROBJ::m_OldClassInfo;
    }

    void ZCHAROBJ::PostClassInit()
    {
        ZGEOM::PostClassInit();
        if (const char* text = m_szTextPtr)
            SetText(text);
        else if (const char* id = m_szStringId)
            *id == '%' ? SetText(id + 1) : SetTextId("", id);
    }

    bool ZCHAROBJ::CreateGeometry()
    {
        RemoveGeometry();
        if (!m_pFont || !m_szTextPtr.c_str())
            return false;

        const char* text = m_szTextPtr;
        uint32_t count = 0;
        for (const char* p = text; *p;)
        {
            const UTC4CHAR ch = GetNextChar(p);
            if (ch && ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t')
                ++count;
        }
        if (!count)
            return false;

        m_pSprites = g_pRenderDll->m_pPrimControl->AllocSpriteArrayUV(count);
        if (!m_pSprites)
            return false;

        m_iIconPrim = uint32_t(-1);
        STextureRef* textures = static_cast<STextureRef*>(ZUniMemory::Allocate(sizeof(STextureRef) * count));
        uint32_t index = 0;
        int32_t x = 0;
        int32_t y = 0;
        UTC4CHAR previous = 0;
        for (const char* p = text;;)
        {
            const UTC4CHAR ch = GetNextChar(p);
            if (!ch)
                break;
            if (ch == '\r')
                continue;
            if (ch == '\n')
            {
                x = 0;
                y += int32_t(m_pFont->GetMaxHeight() * 1.5f) + m_iLineSpacing;
                previous = 0;
                continue;
            }
            if (ch == '\t')
            {
                x += m_iTabSize - x % m_iTabSize;
                previous = 0;
                continue;
            }
            if (ch == ' ')
            {
                x += m_pFont->GetCharInfo(' ')->vAdvance;
                previous = 0;
                continue;
            }

            SGlyph* glyph = m_pFont->GetCharInfo(ch);
            if (!glyph)
                continue;
            if (previous && m_iMonoSpaceSize < 0)
                x -= m_pFont->GetKerning(previous, ch);

            SSpriteArrayElementUV& sprite = m_pSprites[index];
            sprite.p = {float(x + glyph->vStart[0]) + glyph->vSize[0] * 0.5f,
                -float(y + glyph->vStart[1]) - glyph->vSize[1] * 0.5f,
                0.0f};
            sprite.fScale = {float(glyph->vSize[0]), float(glyph->vSize[1])};
            sprite.u = {float(glyph->vUV1[0]) / 128.0f, float(glyph->vUV2[0]) / 128.0f};
            sprite.v = {float(glyph->vUV1[1]) / 128.0f, float(glyph->vUV2[1]) / 128.0f};
            g_pRenderDll->m_pPrimControl->ColorUTOIU(&sprite.lColor, &m_dwFaceColor);
            textures[index] = {glyph->rTexture, index};
            if (glyph->bIcon)
                m_iIconPrim = glyph->rTexture;
            ++index;
            x += m_iMonoSpaceSize >= 0 ? m_iMonoSpaceSize : glyph->vAdvance + m_iSpacingAdd;
            previous = ch == ' ' ? 0 : ch;
        }

        m_iNumSprites = static_cast<uint16_t>(index);
        m_iNumSpriteArrays = static_cast<uint16_t>(CreateWinFaces(textures, m_pSprites, m_pPrims, m_pSpriteArrays, index, m_dwDrawMode, this, &m_iIconPrim));
        ZUniMemory::Free(textures);
        CalcCenSize();
        m_bModified = true;
        return true;
    }

    void ZCHAROBJ::SetFont(ZFONT* pFont)
    {
        m_pFont = pFont;
    }

    bool ZCHAROBJ::SetText(const char* pszText, uint32_t dwLength)
    {
        char* text = const_cast<char*>(pszText);
        const char saved = text[dwLength];
        text[dwLength] = 0;
        const bool result = SetText(pszText);
        text[dwLength] = saved;
        return result;
    }

    bool ZCHAROBJ::SetText(const char* pszText)
    {
        if (!pszText)
            return false;
        m_szTextPtr = nullptr;
        if (*pszText)
        {
            m_szTextPtr = pszText;
            m_szStringId = nullptr;
            return CreateGeometry();
        }
        RemoveGeometry();
        m_szStringId = nullptr;
        return true;
    }

    bool ZCHAROBJ::SetTextId(const char* szStringTable, const char* szTextId)
    {
        const char* text = szTextId;
        if (szTextId && *szTextId)
            text = g_pEngineData->m_pLocaleResources->GetResourceText(szStringTable, szTextId);
        return SetText(text);
    }

    void ZCHAROBJ::Clear()
    {
        SetModified(true);
        m_szTextPtr = nullptr;
        RemoveGeometry();
    }

    int32_t ZCHAROBJ::GetCharPosition(int lIdx)
    {
        const char* text = m_szTextPtr;
        int32_t position = 0;
        UTC4CHAR previous = 0;
        m_pFont->GetCharInfo(' ');
        for (int i = 0; i < lIdx; ++i)
        {
            const UTC4CHAR ch = GetNextChar(text);
            if (!ch)
                break;
            position += m_pFont->GetCharInfo(ch)->vAdvance;
            if (previous)
                position -= m_pFont->GetKerning(previous, ch);
            previous = ch == ' ' ? 0 : ch;
        }
        return position;
    }

    void ZCHAROBJ::SetColor(uint32_t lColor)
    {
        const uint32_t color = (lColor & 0xFFFFFF) | (uint32_t(m_dwAmount) << 24);
        if ((color & 0xFFFFFF) == (m_dwFaceColor & 0xFFFFFF))
            return;
        m_dwFaceColor = color;
        m_bAnimateAlpha = false;
        for (uint32_t i = 0; m_pSpriteArrays && i < m_iNumSpriteArrays; ++i)
        {
            if (m_pPrims[i] == m_iIconPrim)
                continue;
            for (uint32_t j = 0; j < m_pSpriteArrays[i].lNumSprites; ++j)
                g_pRenderDll->m_pPrimControl->ColorUTOIU(&m_pSpriteArrays[i].pSpritesUV[j].lColor, &m_dwFaceColor);
        }
    }

    void ZCHAROBJ::SetAlpha(uint8_t lAlpha)
    {
        SetDrawMode(lAlpha == 0xFF ? m_dwDrawMode & ~1u : m_dwDrawMode | 1u, -1);
        m_dwFaceColor = (m_dwFaceColor & 0xFFFFFF) | (uint32_t(lAlpha) << 24);
        m_dwAmount = lAlpha;
        for (uint32_t i = 0; m_pSpriteArrays && i < m_iNumSpriteArrays; ++i)
        {
            uint32_t color = m_pPrims[i] == m_iIconPrim ? (uint32_t(lAlpha) << 24) | 0xFFFFFF : m_dwFaceColor;
            for (uint32_t j = 0; j < m_pSpriteArrays[i].lNumSprites; ++j)
                g_pRenderDll->m_pPrimControl->ColorUTOIU(&m_pSpriteArrays[i].pSpritesUV[j].lColor, &color);
        }
    }

    void ZCHAROBJ::SetLineSpacing(int8_t spacing)
    {
        m_iLineSpacing = spacing;
    }
    void ZCHAROBJ::SetTabSize(int8_t size)
    {
        m_iTabSize = size;
    }
    int32_t ZCHAROBJ::GetTabSize(int8_t) const
    {
        return m_iTabSize;
    }
    void ZCHAROBJ::SetMonoSpace(int8_t size)
    {
        m_iMonoSpaceSize = size;
    }
    const char* ZCHAROBJ::GetText() const
    {
        return m_szTextPtr;
    }
    ZFONT* ZCHAROBJ::GetFont() const
    {
        return m_pFont;
    }
    void ZCHAROBJ::SetSpacingAdd(int8_t spacing)
    {
        m_iSpacingAdd = spacing;
    }
    void ZCHAROBJ::GetFontRef(ZGEOMREF&)
    {
        ZASSERT(false);
    }
    void ZCHAROBJ::SetFontRef(const ZGEOMREF& font)
    {
        SetFont(geom_cast<ZFONT>(ZGEOM::RefToPtr(font.m_Value)));
    }
    void ZCHAROBJ::GetColor(float (&)[3])
    {
        ZASSERT(false);
    }
    void ZCHAROBJ::SetColor(const float (&color)[3])
    {
        SetColor((uint32_t(color[0]) << 16) | (uint32_t(color[1]) << 8) | uint32_t(color[2]));
    }

#pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<float[3]> CharColor{
            .m_Node = {.m_Next = nullptr, .m_Name = "Color", .m_Filter = 1},
            .m_VirtualTable = VirtualTable_VP__12,
            .m_Get = &ZCHAROBJ::GetColor,
            .m_Set = &ZCHAROBJ::SetColor};
        static RTP::ZVirtualProperty<ZGEOMREF> Font{
            .m_Node = {.m_Next = CharColor, .m_Name = "Font", .m_Filter = 1},
            .m_VirtualTable = VirtualTable_VP__5,
            .m_Get = &ZCHAROBJ::GetFontRef,
            .m_Set = &ZCHAROBJ::SetFontRef};
        static RTP::ZDataProperty<char> SpacingAdd{
            .m_Node = {.m_Next = Font, .m_Name = "m_iSpacingAdd", .m_Filter = 2},
            .m_VirtualTable = VirtualTable_DP__123,
            .m_Offset = reinterpret_cast<char*>(CLASS_PROPERTY(ZCHAROBJ, m_iSpacingAdd))};
        static RTP::ZDataProperty<char> MonoSpaceSize{
            .m_Node = {.m_Next = SpacingAdd, .m_Name = "m_iMonoSpaceSize", .m_Filter = 2},
            .m_VirtualTable = VirtualTable_DP__123,
            .m_Offset = reinterpret_cast<char*>(CLASS_PROPERTY(ZCHAROBJ, m_iMonoSpaceSize))};
        static RTP::ZDataProperty<char> TabSize{
            .m_Node = {.m_Next = MonoSpaceSize, .m_Name = "m_iTabSize", .m_Filter = 2},
            .m_VirtualTable = VirtualTable_DP__123,
            .m_Offset = reinterpret_cast<char*>(CLASS_PROPERTY(ZCHAROBJ, m_iTabSize))};
        static RTP::ZDataProperty<ZRTString> StringId{
            .m_Node = {.m_Next = TabSize, .m_Name = "m_szStringId", .m_Filter = 3},
            .m_VirtualTable = VirtualTable_DP__16,
            .m_Offset = CLASS_PROPERTY(ZCHAROBJ, m_szStringId)};
        static RTP::ZDataProperty<ZRTString> Text{
            .m_Node = {.m_Next = StringId, .m_Name = "m_szTextPtr", .m_Filter = 2},
            .m_VirtualTable = VirtualTable_DP__16,
            .m_Offset = CLASS_PROPERTY(ZCHAROBJ, m_szTextPtr)};
        static RTP::ZDataProperty<char> LineSpacing{
            .m_Node = {.m_Next = Text, .m_Name = "m_iLineSpacing", .m_Filter = 2},
            .m_VirtualTable = VirtualTable_DP__123,
            .m_Offset = reinterpret_cast<char*>(CLASS_PROPERTY(ZCHAROBJ, m_iLineSpacing))};
    }
    DECLARE_GEOM_CLASS_IMPL(ZCHAROBJ, ZWINOBJ, 0x009A2838, "ZCHAROBJ", 0x0077CF84, cProperties::LineSpacing, 0x0080E5D0, 0x009A27C4, 0x009A27C8);
#pragma endregion
}
