#pragma once

#include <Glacier/Data/ZLoadGameInfoBase.h>

namespace Glacier
{
    // PC concrete ZLoadGameInfoBase. The save-game GUI code (hitman3\gui\savepc.cpp)
    // constructs it with the "<path>/SaveGame#<slot>.control"/".data" file names for
    // the slot being loaded, so the engine can open the saved-game streams.
    class ZLoadGameInfoPC : public ZLoadGameInfoBase
    {
    public:
        // vtbl (PC 0x007A2828)
        ~ZLoadGameInfoPC() override;                                  // PC 0x00675330 / 0x00675350
        IInputStream* CreateStream(const char* pszFilename) override; // PC 0x00675240

        // methods
        ZLoadGameInfoPC();                                            // PC 0x006752C0
    };
    RE_VERIFY_SIZE(ZLoadGameInfoPC, 0x204);
}
