#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/GUI/Font/SGlyph.h>
#include <Glacier/GUI/UTC4.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    enum EFontFilterMode
    {
        FFM_Normal,
        FFM_Add,
        FFM_Sub,
        FFM_Invert
    };

    class ZFONT : public ZWINOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZFONT, 0x20002Fu);

        // vtbl
        ~ZFONT() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZFONT
        virtual uint32_t GetNumTextures() const = 0;
        virtual SGlyph* GetCharInfo(UTC4CHAR ch) const = 0;
        virtual int GetMaxHeight() const = 0;
        virtual bool ContainsChar(UTC4CHAR ch) const = 0;
        virtual int32_t GetKerning(unsigned int, unsigned int) const = 0;

        // methods
        ZFONT(const char* psName, ZBaseGeom* pBaseGeom);

        uint32_t GetWidthOfChar(UTC4CHAR ch) const;
    };
    RE_VERIFY_SIZE(ZFONT, 0x88); // Verified PC alloc
}
