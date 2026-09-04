#include <Glacier/Render/Debug/Globals.h>


namespace Glacier
{
#pragma region " --- DEBUG GLOBALS --- "
    ZDrawDebugText* g_pDrawDebugText = nullptr;
    ZDrawDebugTimer* g_pDrawDebugTimer = nullptr;
    ZDrawDebugConCmd* g_pDrawDebugConCmd = nullptr;
    ZRenderDebugMenu g_RenderDebugMenu {};
    bool g_bInfoDisplayEnabled = false;
    bool g_bConCmdDisplayEnabled = false;
#pragma endregion
}
