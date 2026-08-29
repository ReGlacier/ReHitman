#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Physics/ZCloth.h>


namespace Hitman::BloodMoney
{
    class ZTie : public Glacier::ZCloth
    {
    public:
        // vftable (no changes)
        // api
        void HideTie(bool);
        void HideTieInMirror(bool);

        // Data (total size is 0xB4, ZCloth size is 0xB0)
        bool m_bHidden;
        bool m_bHiddenInMirror;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZTie, 0xB4); // Verified
}
