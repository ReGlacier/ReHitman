#pragma once

#include <Glacier/EventBase/ZEventBase.h>

namespace Hitman::BloodMoney
{
    class ZVCR : public Glacier::ZEventBase
    {
    public:
        // vtbl (no changes)
        // custom api
        static void SetTapeStolen(bool value);
        static bool TapeWasStolen();

        // data (size is 0x34)
        Glacier::ZAction* m_stealTapeAction;
    };
}