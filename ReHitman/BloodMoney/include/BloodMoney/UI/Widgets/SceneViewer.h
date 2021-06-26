#pragma once


#include <BloodMoney/UI/DebugWidget.h>


namespace Hitman::BloodMoney
{
    class SceneViewer final : public DebugWidget
    {
    public:
        void draw() override;

        static bool g_bIsVisible;
    };
}