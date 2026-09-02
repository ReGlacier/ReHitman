#include <Glacier/GUI/ZTTFONT.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/RTP/VirtualTables.h>

#include <cstdlib>


namespace Glacier
{
    namespace
    {
        int SimpleUInt32Compare(const void* pLeft, const void* pRight)
        {
            return static_cast<int>(*static_cast<const uint32_t*>(pLeft) - *static_cast<const uint32_t*>(pRight));
        }
    }

    ZTTFONT::ZTTFONT(const char* psName, ZBaseGeom* pBaseGeom)
        : ZFONT(psName, pBaseGeom)
    {
    }

    ZTTFONT::~ZTTFONT() = default;

    const RTP::ZPropertyInfo& ZTTFONT::GetProperties() const
    {
        return ZTTFONT::Info;
    }

    uint32_t ZTTFONT::GetObjectId() const
    {
        return ZTTFONT::m_Id;
    }

    void ZTTFONT::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZTTFONT::m_Id;
        mask = ZTTFONT::m_Mask;
    }

    ZGEOMCLASSINFO* ZTTFONT::GetOldClassInfo() const
    {
        return ZTTFONT::m_OldClassInfo;
    }

    void ZTTFONT::ClassInit()
    {
        ZWINOBJ::ClassInit();
        Hide(true);
    }

    uint32_t ZTTFONT::GetNumTextures() const
    {
        return m_pFontData->lNumTextures;
    }

    SGlyph* ZTTFONT::GetCharInfo(UTC4CHAR ch) const
    {
        SGlyph* pGlyph = static_cast<SGlyph*>(bsearch(&ch, m_pGlyphData, m_pFontData->lNumChars, sizeof(SGlyph), SimpleUInt32Compare));
        if (!pGlyph)
        {
            const UTC4CHAR uFallbackChar = ' ';
            pGlyph = static_cast<SGlyph*>(bsearch(&uFallbackChar, m_pGlyphData, m_pFontData->lNumChars, sizeof(SGlyph), SimpleUInt32Compare));
            if (!pGlyph)
            {
                return m_pGlyphData;
            }
        }

        return pGlyph;
    }

    int ZTTFONT::GetMaxHeight() const
    {
        return m_pFontData->lMaxHeight;
    }

    bool ZTTFONT::ContainsChar(UTC4CHAR ch) const
    {
        return GetCharInfo(ch) != nullptr;
    }

    int32_t ZTTFONT::GetKerning(unsigned int uFirstChar, unsigned int uSecondChar) const
    {
        const uint32_t uKey = uSecondChar ^ static_cast<uint32_t>(_rotl(uFirstChar, 16));
        const uint32_t* pPair = static_cast<const uint32_t*>(bsearch(&uKey, m_pKerningTable, m_pFontData->lNumKerningPairs, 2 * sizeof(uint32_t), SimpleUInt32Compare));
        if (pPair)
        {
            return static_cast<int32_t>(pPair[1]);
        }

        return 0;
    }

    void ZTTFONT::GetDataHandle(uint32_t& lDataHandle)
    {
        lDataHandle = m_iDataHandle;
    }

    void ZTTFONT::SetDataHandle(const uint32_t& lDataHandle)
    {
        m_iDataHandle = lDataHandle;

        const auto* pData = static_cast<const uint8_t*>(g_pRenderDll->m_pPrimControl->GetPrimData(lDataHandle));

        m_pFontData = nullptr;
        m_pFontData = reinterpret_cast<SFontData*>(const_cast<uint8_t*>(pData) + *reinterpret_cast<const uint32_t*>(pData + sizeof(uint32_t)));
        m_pGlyphData = reinterpret_cast<SGlyph*>(m_pFontData + 1);
        m_pKerningTable = m_pGlyphData + m_pFontData->lNumChars;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<uint32_t> DataHandle
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "DataHandle", .m_Filter = 3 },
            .m_VirtualTable = &RTP::VirtualTables::Virtual_uint,
            .m_Get = &ZTTFONT::GetDataHandle,
            .m_Set = &ZTTFONT::SetDataHandle
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZTTFONT,
        ZFONT,
        0x009A2E68,
        "ZTTFONT",
        0x0077FB94,
        cProperties::DataHandle,
        0x0080A900,
        0x009A2E00,
        0x009A2E04
    );
#   pragma endregion
}
