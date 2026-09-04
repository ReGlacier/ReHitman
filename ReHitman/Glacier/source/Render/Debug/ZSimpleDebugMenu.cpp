#include <Glacier/Render/Debug/ZSimpleDebugMenu.h>
#include <Glacier/Render/Debug/ZDrawDebugText.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>


namespace Glacier
{
    namespace
    {
        static Action::ZStaticBinds pszActionMapDef_0 {
            "DebugAlways={DrawDebugText={DDTConCmdMenu=&& hold(gc,left2) hold(gc,left1) tap(gc,start);DDTEnableMenu=& tap(gc,right2) tap(gc,left2);DDTUp=| tap(gc,up) tap(kb,up);DDTDown=| tap(gc,down) tap(kb,down);DDTUpHold=| hold(gc,up) hold(kb,up);DDTDownHold=| hold(gc,down) hold(kb,down);DDTLeft=| tap(gc,left) tap(kb,left);DDTRight=| tap(gc,right) tap(kb,right);DDTLeftHold=| hold(gc,left) hold(kb,left);DDTRightHold=| hold(gc,right) hold(kb,right);DDTSelect=| release(gc,cross) release(kb,return);DDTExit=| release(gc,triangle) release(kb,esc);DDTHideMenu=| hold(gc,square) hold(kb,space);};};"
        };

        static Action::ZHandle a_DDTConsoleCommandMenu { "DDTConCmdMenu" };
        static Action::ZHandle a_DDTEnableMenu { "DDTEnableMenu" };
        static Action::ZHandle a_DDTUp { "DDTUp" };
        static Action::ZHandle a_DDTUpHold { "DDTUpHold" };
        static Action::ZHandle a_DDTDown { "DDTDown" };
        static Action::ZHandle a_DDTDownHold { "DDTDownHold" };
        static Action::ZHandle a_DDTLeft { "DDTLeft" };
        static Action::ZHandle a_DDTRight { "DDTRight" };
        static Action::ZHandle a_DDTLeftHold { "DDTLeftHold" };
        static Action::ZHandle a_DDTRightHold { "DDTRightHold" };
        static Action::ZHandle a_DDTSelect { "DDTSelect" };
        static Action::ZHandle a_DDTExit { "DDTExit" };
        static Action::ZHandle a_DDTHideMenu { "DDTHideMenu" };

        bool MoveDoWait(bool bRepeat)
        {
            static TIMETYPE fWaitTime {};

            if (!bRepeat)
            {
                fWaitTime = g_pSysInterface->ActualFrameTime();
                return false;
            }

            auto dt = fWaitTime - g_pSysInterface->ActualFrameTime();
            return static_cast<float>(dt) < 0.0f;
        }
    }

    ZSimpleDebugMenu::ZSimpleDebugMenu()
    {
        // Do nothing
    }

    void ZSimpleDebugMenu::CountMenuItems()
    {
        uint32_t lItemsNr = 0;

        if (m_pCurrentMenu)
        {
            while (m_pCurrentMenu->pItems[lItemsNr].pText)
            {
                ++lItemsNr;
            }
        }

        m_iNumberItems = lItemsNr;
    }

    uint32_t ZSimpleDebugMenu::CalcMenuSizeX() const
    {
        uint32_t lItemsNr = 0, lWidth = 0;

        if (m_pCurrentMenu)
        {
            while (m_pCurrentMenu->pItems[lItemsNr].pText)
            {
                ++lItemsNr;
                lWidth = std::max(lWidth, strlen(m_pCurrentMenu->pItems[lItemsNr].pText));
            }
        }

        return lWidth;
    }

    uint32_t ZSimpleDebugMenu::CalcMenuSizeY() const
    {
        return m_iNumberItems;
    }

    bool ZSimpleDebugMenu::KeyRepeatWait(bool bRepeat)
    {
        return MoveDoWait(bRepeat);
    }

    void ZSimpleDebugMenu::Init()
    {
        // Do nothing
    }

    void ZSimpleDebugMenu::Update()
    {
        if (!m_bEditEnabled)
            return;

        if (a_DDTExit.Digital() && LeaveMenu())
        {
            EndEdit();
        }
        else if (a_DDTLeft.Digital() && m_pCurrentMenu->pMoveLeftCallback)
        {
            m_pCurrentMenu->pMoveLeftCallback(this, true);
        }
        else if (a_DDTRight.Digital())
        {
            if (m_pCurrentMenu->pMoveRightCallback)
            {
                m_pCurrentMenu->pMoveRightCallback(this, false);
            }
        }
        else if (a_DDTRightHold.Digital() && m_pCurrentMenu->pMoveRightCallback)
        {
            m_pCurrentMenu->pMoveRightCallback(this, true);
        }
        else if (a_DDTUp.Digital())
        {
            --m_lSelectedMenuItem;
            ZASSERT(m_iNumberItems);
            m_lSelectedMenuItem %= m_iNumberItems;
        }
        else if (a_DDTDown.Digital())
        {
            ++m_lSelectedMenuItem;
            ZASSERT(m_iNumberItems);
            m_lSelectedMenuItem %= m_iNumberItems;
        }
        else if (a_DDTSelect.Digital())
        {
            if (m_pCurrentMenu->pItems[m_lSelectedMenuItem].pCallback)
            {
                m_pCurrentMenu->pItems[m_lSelectedMenuItem].pCallback(this);
            }
        }

        Draw();
    }

    void ZSimpleDebugMenu::BeginEdit()
    {
        g_pDrawDebugText->Lock();
        g_pEngineData->FreezeScene(true);
        m_bEditEnabled = true;
        m_lSelectedMenuItem = 0;
        CountMenuItems();
    }

    void ZSimpleDebugMenu::EndEdit()
    {
        g_pDrawDebugText->Unlock();
        g_pEngineData->FreezeScene(false);
        m_bEditEnabled = false;
    }

    bool ZSimpleDebugMenu::LeaveMenu()
    {
        if (m_pCurrentMenu->pLeaveMenuCallback)
        {
            m_pCurrentMenu->pLeaveMenuCallback(this);
        }

        if (m_LastMenu.Count())
        {
            auto res = m_LastMenu.Pop();
            m_pCurrentMenu = res.m_pMenu;
            m_lSelectedMenuItem = res.m_lSelectedMenuItem;
            CountMenuItems();

            return false;
        }

        return true;
    }

    void ZSimpleDebugMenu::EnterMenu(ZSimpleMenu* pMenu)
    {
        if (m_pCurrentMenu)
        {
            ZStoredSimpleMenu sMenu;
            sMenu.m_pMenu = m_pCurrentMenu;
            sMenu.m_lSelectedMenuItem = m_lSelectedMenuItem;
            m_LastMenu.Push(sMenu);
        }

        m_pCurrentMenu = pMenu;
        m_lSelectedMenuItem = 0;
        CountMenuItems();

        if (m_pCurrentMenu->pEnterMenuCallback)
        {
            m_pCurrentMenu->pEnterMenuCallback(this);
        }
    }

    void ZSimpleDebugMenu::DrawAlways()
    {
        // Do nothing
    }

    void ZSimpleDebugMenu::Draw()
    {
        auto* pFrame = g_pDrawDebugText->AddFrame();

        ZDebugFrame sFrame { pFrame };
        const auto lWidth = CalcMenuSizeX();
        const auto lHeight = CalcMenuSizeY();

        sFrame.SetSizeText(lWidth, lHeight);
        sFrame.Fill(0x80802020u);
        sFrame.DrawFocus();

        for (int i = 0; i < m_iNumberItems; ++i)
        {
            if (m_lSelectedMenuItem == i)
            {
                sFrame.PlotInvers(0, i, m_pCurrentMenu->pItems[i].pText);
            }
            else
            {
                sFrame.Plot(0, i, m_pCurrentMenu->pItems[i].pText);
            }
        }
    }
}
