#include <Glacier/Render/Debug/ZRenderDebugMenu.h>
#include <Glacier/Render/Debug/Globals.h>


namespace Glacier
{
    namespace
    {
        constexpr const char* pTimerMode[6] {
            "Timers None",
            "Timers Normal",
            "Timers Geoms",
            "Timers Events",
            "Timers Single",
            "Timers Online",
        };
        constexpr const char* pPauseMode[2] { "Pause Off", "Pause On" };
        constexpr const char* pConCmd[2] { "ConCmd Off", "ConCmd On" };
        constexpr const char* pMipCoverage[2] { "Disp Mip Off", "Disp Mip On" };
        constexpr const char* pSInfoMode[2] { "Sound Info Off", "Sound Info On" };
        constexpr const char* pInfoMode[2] { "Info Off", "Info On" };
        constexpr const char* pCameraMode[3] { "Fixed Camera", "Free Camera", "Free Camera 2" };
        constexpr const char* pDrawMode[4] { "Normal", "Texture", "Light", "Wire" };

        void ToggleCameraMode(void* ptr) { reinterpret_cast<ZRenderDebugMenu*>(ptr)->ToggleCameraMode(); }
        void ToggleDrawMode(void* ptr) { reinterpret_cast<ZRenderDebugMenu*>(ptr)->ToggleDrawMode(); }

        void ToggleMipCoverageMode(void* ptr)
        {
            // TODO: Finish me
        }

        void ToggleInfoMode(void* ptr)
        {
            // TODO: Finish me
        }

        void ToggleSInfoMode(void* ptr)
        {
            // TODO: Finish me
        }

        void TogglePauseMode(void* ptr)
        {
            // TODO: Finish me
        }

        void ToggleTimerMode(void* ptr)
        {
            // TODO: Finish me
        }

        void ToggleConCmdMode(void* ptr)
        {
            if (g_bConCmdDisplayEnabled)
            {
                g_bConCmdDisplayEnabled = false;
                // TODO: Finish me
            }
            else
            {
                // TODO: Finish me
                g_bConCmdDisplayEnabled = true;
            }
        }

        ZMenuItem RenderDebugItems[9] {
            {
                .pText = nullptr,
                .pCallback = ToggleCameraMode,
            }, {
                .pText = nullptr,
                .pCallback = ToggleDrawMode,
            },
            {
                .pText = nullptr,
                .pCallback = ToggleMipCoverageMode,
            },
            {
                .pText = nullptr,
                .pCallback = ToggleInfoMode
            },
            {
                .pText = nullptr,
                .pCallback = ToggleSInfoMode
            },
            {
                .pText = nullptr,
                .pCallback = TogglePauseMode
            },
            {
                .pText = nullptr,
                .pCallback = ToggleTimerMode
            },
            {
                .pText = nullptr,
                .pCallback = ToggleConCmdMode
            },
            {
            }
        };

        ZMenu RenderDebugMenu
        {
            .pEnterMenuCallback = nullptr,
            .pLeaveMenuCallback = nullptr,
            .pMoveLeftCallback = nullptr,
            .pMoveRightCallback = nullptr,
            .m_pItems = RenderDebugItems,
            .m_lLastSelectedMenuItem = 0
        };
    }
    // ZRenderDebugMenu
}
