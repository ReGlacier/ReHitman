#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <BloodMoney/Game/UI/UIFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <BloodMoney/Game/UI/ZWINOBJ.h>
#include <BloodMoney/Game/UI/ZFONT.h>
#include <Glacier/ZSTL/ZMemory.h>

namespace Hitman::BloodMoney
{
    class ZCHAROBJ : public ZWINOBJ
    {
    public:
        // vftable
        virtual void CreateGeometry();
        virtual void SetFont(ZFONT*);
        virtual void SetText(char const*);
        virtual void SetText(char const*, uint32_t);
        virtual void SetTextId(char const*, char const*);
        virtual void Clear();
        virtual int GetCharPosition(int);

        // api

        // data (size is 0xA0, base size is 0x88)
        int8_t m_iLineSpacing;
        uint32_t m_iIconPrim;
        Glacier::ZRTString m_szTextPtr;
        Glacier::ZRTString m_szStringId;
        ZFONT* m_pFont;
        int8_t m_iTabSize;
        int8_t m_iMonoSpaceSize;
        int8_t m_iSpacingAdd;
    };
    RE_VERIFY_SIZE(ZCHAROBJ, 0xA0);
}