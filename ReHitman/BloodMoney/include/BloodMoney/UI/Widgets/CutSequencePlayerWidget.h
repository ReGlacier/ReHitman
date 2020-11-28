#pragma once

#include <BloodMoney/UI/DebugWidget.h>

namespace Hitman::BloodMoney
{
    class CutSequencePlayerWidget final : public DebugWidget
    {
    public:
        static bool g_bIsShowed;

        void draw() override;
    };
}