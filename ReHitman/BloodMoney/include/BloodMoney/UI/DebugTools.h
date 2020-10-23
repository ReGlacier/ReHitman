#pragma once

#include <BloodMoney/UI/DebugWidget.h>

namespace Hitman::BloodMoney
{
    class DebugTools : public DebugWidget
    {
    private:
        bool m_bIsVisible { false };

    public:
        void draw() override;
        void toggleVisibility();
        [[nodiscard]] bool isVisible() const;

    private:
        void onPreDraw();
        void onPostDraw();

        void drawTopMenu();
    };
}