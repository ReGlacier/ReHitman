#pragma once

#include <BloodMoney/UI/DebugWidget.h>

namespace Hitman::BloodMoney
{
    class SandboxWidget final : public DebugWidget
    {
        static bool g_bIsVisible;
    public:
        void draw() override;
    };
}