#pragma once

#include <BloodMoney/PC/IHM3SaveInterface.h>

namespace Hitman
{
    class ZHM3SaveBase : public Hitman::IHM3SaveInterface
    {
    public:
        // static
        static bool s_bLoadingSaveGame;

        // vtbl
        // TODO: Finish me

        // methods
        // TODO: Finish me

        // members
        int m_iSaveGameSlot;
        ENUMERATED_PROFILES m_EnumeratedProfiles;
        bool m_bLoadedSaveGame;
    };
}
