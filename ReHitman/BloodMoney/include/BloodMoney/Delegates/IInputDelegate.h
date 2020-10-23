#pragma once

#include <cstdint>
#include <Windows.h>

namespace Hitman::BloodMoney
{
    class IInputDelegate
    {
    public:
        virtual ~IInputDelegate() noexcept = default;

        virtual void setKeyState(int keyCode, int state) {};
        virtual void setMouseKeyState(int keyId, bool state) {};
        virtual void setMouseWheelState(float dt) {};

        virtual void onWindowsEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {};
    };
}