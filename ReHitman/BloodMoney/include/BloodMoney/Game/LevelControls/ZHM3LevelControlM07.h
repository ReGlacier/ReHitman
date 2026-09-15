#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZHash.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>


namespace Hitman::BloodMoney
{
    // Unfinished game mission
    class ZHM3LevelControlM07 : public ZHM3LevelControl
    {
    public:
        // vtbl (no changes)
        // data
        bool m_bNegotiatorKilled;
        bool m_bCrabBossKilled;
        bool m_bPilotKilled;
        bool m_bMessengerKilled;
        bool m_bGamblerKilled;
        bool m_bMissionCompleted;
    };
    RE_VERIFY_SIZE(ZHM3LevelControlM07, 0x5E0); // Verified
}