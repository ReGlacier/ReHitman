#pragma once

#include <Glacier/Render/Debug/Fwd.h>
#include <Glacier/Render/Debug/ZRenderDebugMenu.h>


namespace Glacier
{
#pragma region " --- DEBUG GLOBALS --- "
    extern ZDrawDebugText* g_pDrawDebugText;
    extern ZDrawDebugTimer* g_pDrawDebugTimer;
    extern ZDrawDebugConCmd* g_pDrawDebugConCmd;
    extern ZRenderDebugMenu g_RenderDebugMenu;
    extern bool g_bInfoDisplayEnabled;
    extern bool g_bConCmdDisplayEnabled;
#pragma endregion
}
