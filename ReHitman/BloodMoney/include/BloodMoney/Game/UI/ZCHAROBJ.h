#pragma once

#include <Glacier/GlacierFWD.h>
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
        int m_field88;
        int m_field8C;
        int m_field90;
        int m_field94;
        int m_field98;
        int m_field9C;
    };
}