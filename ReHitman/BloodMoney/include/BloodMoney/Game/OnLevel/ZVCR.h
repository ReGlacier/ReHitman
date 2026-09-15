#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Hitman::BloodMoney
{
    class ZVCR : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        // vtbl (no changes)
        // custom api
        static void SetTapeStolen(bool value);
        static bool TapeWasStolen();

        // data (size is 0x34)
        Glacier::ZAction* m_pAction;
    };
    RE_VERIFY_SIZE(ZVCR, 0x34); // Verified
}