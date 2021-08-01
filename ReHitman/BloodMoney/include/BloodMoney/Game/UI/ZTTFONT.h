#pragma once

#include <BloodMoney/Game/UI/ZFONT.h>

namespace Hitman::BloodMoney {
    class ZTTFONT : public ZFONT
    {
    public:
        // vftable
        // custom api
        static ZTTFONT* Create();

        // data (size is 0x98, base size is 0x88)
        int m_field88;
        int m_field8C;
        int m_field90;
        int m_field94;
    };
}