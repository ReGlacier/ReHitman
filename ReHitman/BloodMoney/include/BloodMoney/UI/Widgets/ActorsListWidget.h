#pragma once

#include <BloodMoney/UI/DebugWidget.h>

namespace Hitman::BloodMoney
{
    class ActorsListWidget final : public DebugWidget
    {
    public:
        static bool g_bIsOpened;

        void draw() override;
    };
}