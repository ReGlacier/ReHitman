#pragma once

#include <BloodMoney/UI/DebugWidget.h>

namespace Hitman::BloodMoney
{
    class SandboxWidget final : public DebugWidget
    {
    public:
        virtual void draw();
    };
}