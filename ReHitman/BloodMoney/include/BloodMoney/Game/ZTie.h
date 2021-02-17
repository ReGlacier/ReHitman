#pragma once

#include <BloodMoney/Game/ZCloth.h>

namespace Hitman::BloodMoney
{
    class ZTie : public ZCloth
    {
    public:
        // vftable (no changes)
        // api
        void HideTie(bool);
        void HideTieInMirror(bool);
        // Data (total size is 0xB4, ZCloth size is 0xB0)
        int field_B0;
    };
}