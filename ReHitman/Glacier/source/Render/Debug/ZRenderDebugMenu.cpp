#include <Glacier/Render/Debug/ZRenderDebugMenu.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/Render/Debug/ZDrawDebugTimer.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    namespace
    {
        extern ZMenu RenderDebugMenu;
        extern ZMenuItem RenderDebugItems[9];
        constexpr const char* pTimerMode[6] {
            "Timers None", "Timers Normal", "Timers Events", "Timers Geoms", "Timers Single", "Timers Online",
        };
        constexpr const char* pPauseMode[2] { "Pause Off", "Pause On" };
        constexpr const char* pConCmd[2] { "ConCmd Off", "ConCmd On" };
        constexpr const char* pMipCoverage[2] { "Disp Mip Off", "Disp Mip On" };
        constexpr const char* pSInfoMode[2] { "Sound Info Off", "Sound Info On" };
        constexpr const char* pInfoMode[2] { "Info Off", "Info On" };
        constexpr const char* pCameraMode[3] { "Fixed Camera", "Free Camera", "Free Camera 2" };
        constexpr const char* pDrawMode[4] { "Normal", "Texture", "Light", "Wire" };

        void UpdateRenderMenuText()
        {
            RenderDebugItems[0].pText = pCameraMode[g_RenderDebugMenu.m_lCameraModeIndex % 3];
            RenderDebugItems[1].pText = pDrawMode[g_RenderDebugMenu.m_lDrawModeIndex % 4];
            RenderDebugItems[2].pText = pMipCoverage[g_dwMipFilter != 0];
            RenderDebugItems[3].pText = pInfoMode[g_bInfoDisplayEnabled];
            RenderDebugItems[4].pText = pSInfoMode[g_pEngineData && g_pEngineData->m_bSoundDisplay];
            RenderDebugItems[5].pText = pPauseMode[g_pEngineData && g_pEngineData->m_bPause];
            RenderDebugItems[6].pText = pTimerMode[g_pDrawDebugTimer ? g_pDrawDebugTimer->m_lTimerType : NONE];
            RenderDebugItems[7].pText = pConCmd[g_bConCmdDisplayEnabled];
        }
    }

    ZRenderDebugMenu::ZRenderDebugMenu()
        : m_lDrawModeIndex(0)
        , m_lCameraModeIndex(0)
        , m_lNumberItems(0)
        , m_bEditEnabled(false)
        , m_lSelectedMenuItem(0)
        , m_pCurrentMenu(nullptr)
        , m_LastMenu()
    {
    }

    ZRenderDebugMenu::~ZRenderDebugMenu() = default;

    void ZRenderDebugMenu::Init()
    {
        m_lDrawModeIndex = 0;
        m_lCameraModeIndex = 0;
        m_lSelectedMenuItem = 0;
        m_pCurrentMenu = &RenderDebugMenu;
        UpdateRenderMenuText();
        CountMenuItems();
    }

    void ZRenderDebugMenu::Update()
    {
        if (!m_bEditEnabled || !m_pCurrentMenu)
            return;

        UpdateRenderMenuText();
    }

    void ZRenderDebugMenu::ToggleCameraMode()
    {
        m_lCameraModeIndex = (m_lCameraModeIndex + 1) % 3;
    }

    void ZRenderDebugMenu::ToggleDrawMode()
    {
        m_lDrawModeIndex = (m_lDrawModeIndex + 1) % 4;
    }

    void ZRenderDebugMenu::CountMenuItems()
    {
        m_lNumberItems = 0;
        while (m_pCurrentMenu && m_pCurrentMenu->m_pItems[m_lNumberItems].pText)
            ++m_lNumberItems;
    }

    void ZRenderDebugMenu::CalcMenuSizeX() {}
    void ZRenderDebugMenu::CalcMenuSizeY() {}
    void ZRenderDebugMenu::BeginEdit() { m_bEditEnabled = true; m_lSelectedMenuItem = 0; }
    void ZRenderDebugMenu::EndEdit() { m_bEditEnabled = false; }
    void ZRenderDebugMenu::EnterMenu(ZMenu* pMenu) { m_pCurrentMenu = pMenu; m_lSelectedMenuItem = 0; CountMenuItems(); }
    void ZRenderDebugMenu::LeaveMenu() { m_pCurrentMenu = &RenderDebugMenu; m_lSelectedMenuItem = 0; CountMenuItems(); }

    namespace
    {
        void ToggleCameraMode(void* ptr) { reinterpret_cast<ZRenderDebugMenu*>(ptr)->ToggleCameraMode(); }
        void ToggleDrawMode(void* ptr) { reinterpret_cast<ZRenderDebugMenu*>(ptr)->ToggleDrawMode(); }

        void ToggleMipCoverageMode(void* ptr)
        {
            (void)ptr;
            g_dwMipFilter = g_dwMipFilter ? 0u : 2u;
        }

        void ToggleInfoMode(void* ptr)
        {
            (void)ptr;
            g_bInfoDisplayEnabled = !g_bInfoDisplayEnabled;
        }

        void ToggleSInfoMode(void* ptr)
        {
            (void)ptr;
            if (g_pEngineData)
                g_pEngineData->m_bSoundDisplay = !g_pEngineData->m_bSoundDisplay;
        }

        void TogglePauseMode(void* ptr)
        {
            (void)ptr;
            if (g_pEngineData)
                g_pEngineData->PauseScene(!g_pEngineData->m_bPause);
        }

        void ToggleTimerMode(void* ptr)
        {
            (void)ptr;
            if (g_pDrawDebugTimer)
            {
                auto& type = g_pDrawDebugTimer->m_lTimerType;
                type = static_cast<TIMERTYPE>((type + 1) % 5);
                g_pDrawDebugTimer->m_bRunning = type != NONE;
            }
        }

        void ToggleConCmdMode(void* ptr)
        {
            (void)ptr;
            if (g_bConCmdDisplayEnabled)
            {
                g_bConCmdDisplayEnabled = false;
            }
            else
            {
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
