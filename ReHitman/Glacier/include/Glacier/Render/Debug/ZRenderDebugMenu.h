#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Debug/ZCurvesMenu.h>
#include <Glacier/ZSTL/ZStackArray.h>


namespace Glacier
{    
    class ZRenderDebugMenu
    {
    public:
        // methods
        ZRenderDebugMenu();
        ~ZRenderDebugMenu();
        void Init();
        void Update();
        void ToggleCameraMode();
        void ToggleDrawMode();
        void CountMenuItems();
        void CalcMenuSizeX();
        void CalcMenuSizeY();
        void BeginEdit();
        void EndEdit();
        void EnterMenu(ZMenu* pMenu);
        void LeaveMenu();

        // members
        uint32_t m_lDrawModeIndex;
        uint32_t m_lCameraModeIndex;
        uint32_t m_lNumberItems;
        bool m_bEditEnabled;
        RE_ADD_PADDING(3);
        uint32_t m_lSelectedMenuItem;
        ZMenu* m_pCurrentMenu;
        ZSafeStackArray<32, ZStoredMenu> m_LastMenu;
    };
}