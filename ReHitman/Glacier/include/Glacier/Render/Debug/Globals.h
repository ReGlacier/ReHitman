#pragma once

#include <Glacier/Render/Debug/Fwd.h>
#include <Glacier/Render/Debug/ZRenderDebugMenu.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <cstdint>


namespace Glacier
{
#pragma region " --- DEBUG GLOBALS --- "
    extern ZDrawDebugText* g_pDrawDebugText;
    extern ZDrawDebugTimer* g_pDrawDebugTimer;
    extern ZDrawDebugConCmd* g_pDrawDebugConCmd;
    extern ZRenderDebugMenu g_RenderDebugMenu;
    extern bool g_bInfoDisplayEnabled;
    extern bool g_bConCmdDisplayEnabled;

    // Show/timer debug variables (Xbox_KL1 zdrawdebugtimer.cpp / zdrawdebugtext.cpp).
    extern uint32_t g_lTimerDisplayEnabled;
    extern ZDebugInt g_lShowFrameRate;
    extern ZDebugInt g_lShowDebug;
    extern ZDebugInt g_lShowDebugText;
#pragma endregion
}
