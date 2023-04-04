#pragma once

#include <BloodMoney/UI/DebugWidget.h>

namespace Hitman::BloodMoney
{
    class PlayerTeleportWidget final : public DebugWidget
    {
    public:
        static bool g_bIsShowed;

        void draw() override;
    };
}