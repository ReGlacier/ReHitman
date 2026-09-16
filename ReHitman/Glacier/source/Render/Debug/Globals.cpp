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

    // Xbox_KL1 0x8301AE28 - plain uint driven by the Show command and the "show_timer" option.
    uint32_t g_lTimerDisplayEnabled = 0;

    // Xbox_KL1 g_lShowFrameRate / g_lShowDebug (ZDebugInt, registered as console variables).
    ZDebugInt g_lShowFrameRate { "show_framerate", "Shows the frame rate graph", 1, 0, 1, 1, "Render/" };
    ZDebugInt g_lShowDebug { "show_debug", "Toggles 'all' the show_debug* vars", 1, -1000000, 1000000, 1, nullptr };
#pragma endregion
}
