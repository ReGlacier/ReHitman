#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/RTP/PropertyTypes.h>


namespace Glacier
{
    // fwds
    class ZFONT;

    class ZCHAROBJ : public ZWINOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZCHAROBJ, 0x20002Du);

        // vtbl
        ~ZCHAROBJ() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void PostClassInit() override;

        // ZSTDOBJ

        // ZWINOBJ
        void SetColor(uint32_t lColor) override;
        void SetAlpha(uint8_t lAlpha) override;

        // ZCHAROBJ
        virtual bool CreateGeometry();
        virtual void SetFont(ZFONT* pFont);
        virtual bool SetText(const char* pszText, uint32_t dwLength);
        virtual bool SetText(const char* pszText);
        virtual bool SetTextId(const char* szStringTable, const char* szTextId);
        virtual void Clear();
        virtual int32_t GetCharPosition(int lIdx);

        // methods
        ZCHAROBJ(const char* psName, ZBaseGeom* pBaseGeom);

        void SetLineSpacing(int8_t spacing);
        void SetTabSize(int8_t size);
        int32_t GetTabSize(int8_t unused) const;
        void SetMonoSpace(int8_t size);
        const char* GetText() const;
        ZFONT* GetFont() const;
        void SetSpacingAdd(int8_t spacing);
        void GetFontRef(ZGEOMREF& font);
        void SetFontRef(const ZGEOMREF& font);
        void GetColor(float (&color)[3]);
        void SetColor(const float (&color)[3]);

        // members
        int8_t m_iLineSpacing;
        uint32_t m_iIconPrim;
        ZRTString m_szTextPtr;
        ZRTString m_szStringId;
        ZFONT* m_pFont;
        int8_t m_iTabSize;
        int8_t m_iMonoSpaceSize;
        int8_t m_iSpacingAdd;
    };
    RE_VERIFY_SIZE(ZCHAROBJ, 0xA0);
    RE_VERIFY_OFFSET(ZCHAROBJ, m_iLineSpacing, 0x88);
    RE_VERIFY_OFFSET(ZCHAROBJ, m_iIconPrim, 0x8C);
    RE_VERIFY_OFFSET(ZCHAROBJ, m_szTextPtr, 0x90);
    RE_VERIFY_OFFSET(ZCHAROBJ, m_pFont, 0x98);
}
