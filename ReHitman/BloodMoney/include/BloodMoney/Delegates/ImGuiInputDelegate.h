#pragma once

#include <BloodMoney/Delegates/IInputDelegate.h>

namespace Hitman::BloodMoney
{
    class ImGuiInputDelegate final : public IInputDelegate
    {
    public:
        void setKeyState(int keyCode, int state) override;
        void setMouseKeyState(int keyId, bool state) override;
        void setMouseWheelState(float dt) override;

        void onWindowsEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    };
}