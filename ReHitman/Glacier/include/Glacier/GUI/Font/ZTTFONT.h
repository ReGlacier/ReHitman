#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/Font/ZFONT.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    struct SFontData
    {
        uint32_t lFontSize;
        float  fAspect;
        uint32_t lNumChars;
        uint32_t lMaxHeight;
        uint32_t lNumKerningPairs;
        uint32_t lNumTextures;
    };

    class ZTTFONT : public ZFONT
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZTTFONT, 0x20003Au);

        // vtbl
        ~ZTTFONT() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;

        // ZFONT
        uint32_t GetNumTextures() const override;
        SGlyph* GetCharInfo(UTC4CHAR ch) const override;
        int GetMaxHeight() const override;
        bool ContainsChar(UTC4CHAR ch) const override;
        int32_t GetKerning(unsigned int, unsigned int) const override;

        // methods
        ZTTFONT(const char* psName, ZBaseGeom* pBaseGeom);

    #   pragma region " --- RTTI Methods --- "
        void GetDataHandle(uint32_t& lDataHandle);
        void SetDataHandle(const uint32_t& lDataHandle);
    #   pragma endregion

        // members
        uint32_t m_iDataHandle;
        SFontData* m_pFontData;
        SGlyph* m_pGlyphData;
        void* m_pKerningTable;
    };
    RE_VERIFY_SIZE(ZTTFONT, 0x98); // Verified PC alloc
}
