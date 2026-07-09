#pragma once

#include <BloodMoney/Game/UI/ZGUIBase.h>

namespace Hitman::BloodMoney {
    class ZColorSet : public ZGUIBase {
    public:
        enum EColorIndex 
        {
            DisableColor = 0,
            FocusColor = 1,
            NormalColor = 2
        };

        //vftable (no changes)
        //api
        uint32_t GetColor(EColorIndex colorIndex);

        //data (total size is 0x58, base size is 0x4C)
        uint32_t m_aiColors[3];
    };
    RE_VERIFY_SIZE(ZColorSet, 0x58); // Verified
}