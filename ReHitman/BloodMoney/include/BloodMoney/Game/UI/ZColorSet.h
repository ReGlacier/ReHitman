#pragma once

#include <BloodMoney/Game/UI/ZGUIBase.h>

namespace Hitman::BloodMoney {
    class ZColorSet : public ZGUIBase {
    public:
        enum EColorIndex {
            DisableColor,
            FocusColor,
            NormalColor
        };

        //vftable (no changes)
        //api
        uint32_t GetColor(EColorIndex colorIndex);

        //data (total size is 0x58, base size is 0x48)
        int m_field48;
        uint32_t m_NormalColor;
        uint32_t m_FocusColor;
        uint32_t m_DisableColor;
    };
}