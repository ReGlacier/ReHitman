#include <Glacier/GUI/XMLInterface/Windows/ZComponentWindow.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIGroup.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <cstring>


namespace Glacier
{
    ZComponentWindow::ZComponentWindow(ZMenuElements* pMenuElements)
        : IWindowInterface(pMenuElements)
    {
        m_v2BrowserOffset.x = 20.0f;
        m_v2BrowserOffset.y = 0.0f;
        m_pResourceManager = nullptr;
        m_pSubwindowParent = nullptr;
        m_bSubWindowFocus = false;
        m_pParent = nullptr;
    }

    void ZComponentWindow::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IWindowInterface::readParams(ppParams, pElems);

        ReadV2(reinterpret_cast<float(&)[2]>(m_v2BrowserOffset), ppParams, "Browser");
    }

    void ZComponentWindow::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "Header") == 0)
        {
            ZGUIGroup* pGroup = static_cast<ZGUIGroup*>(pEntry);
            const ZStaticVector<IGUIElement*, 16>& rGroup = pGroup->GetGroup();
            for (uint32_t i = 0; i < rGroup.size(); ++i)
            {
                if (m_aGUIElements[0].size() < m_aGUIElements[0].capacity())
                    m_aGUIElements[0].push_back(rGroup[i]);
            }
        }
        else if (strcmp(pName, "Browser") == 0)
        {
            ZGUIGroup* pGroup = static_cast<ZGUIGroup*>(pEntry);
            const ZStaticVector<IGUIElement*, 16>& rGroup = pGroup->GetGroup();
            for (uint32_t i = 0; i < rGroup.size(); ++i)
            {
                if (m_aGUIElements[1].size() < m_aGUIElements[1].capacity())
                    m_aGUIElements[1].push_back(rGroup[i]);
            }
        }
    }

    void ZComponentWindow::OpenWindow(ZResourceManager* pResourceManager, bool bReset, ZWINGROUP* pWinGroup, bool bFocus)
    {
        m_pParent = pWinGroup;

        m_pSubwindowParent = pResourceManager->GetWingroup(pWinGroup);

        IWindowInterface::OpenWindow(pResourceManager, bReset, pWinGroup, bFocus);

        for (int i = 0; i < 2; ++i)
        {
            m_aGUICurrentElements[i].clear();

            for (uint32_t j = 0; j < m_aGUIElements[i].size(); ++j)
            {
                IGUIElement* pElement = m_aGUIElements[i][j];
                if (pElement->Use())
                    m_aGUICurrentElements[i].push_back(pElement);
            }
        }

        float aGroupPos[2] = { 0.0f, 0.0f };
        SetupGUIElements(aGroupPos, m_eNavigation, m_aGUICurrentElements[0], pResourceManager, pWinGroup, nullptr);

        float aBrowserPos[2] = { m_v2BrowserOffset.x, m_v2BrowserOffset.y };
        SetupGUIElements(aBrowserPos, m_eNavigation, m_aGUICurrentElements[1], pResourceManager, pWinGroup, nullptr);

        if (bFocus)
            SetFirstFocus(m_aGUICurrentElements[1]);

        m_pResourceManager = pResourceManager;

        if (!m_aGUICurrentElements[1].empty() && !g_pSysInterface->m_bUseGameController)
        {
            IGUIElement* pElement = m_aGUICurrentElements[1][0];
            PushSubWindow(pElement->GetWindow(), pElement, false, true);
        }
    }

    void ZComponentWindow::CloseWindow(ZResourceManager* pResourceManager, bool bReset)
    {
        IWindowInterface::CloseWindow(pResourceManager, bReset);

        for (uint32_t i = 0; i < m_aGUICurrentElements[0].size(); ++i)
            m_aGUICurrentElements[0][i]->ReleaseResources(pResourceManager);

        for (uint32_t i = 0; i < m_aGUICurrentElements[1].size(); ++i)
            m_aGUICurrentElements[1][i]->ReleaseResources(pResourceManager);

        const uint32_t iStackSize = m_WindowStack.size();
        m_pResourceManager = nullptr;
        m_bSubWindowFocus = false;

        if (iStackSize)
            m_WindowStack[iStackSize - 1]->CloseWindow(pResourceManager, false);

        m_WindowStack.clear();

        pResourceManager->ReleaseWinGroup(m_pSubwindowParent);
        m_pSubwindowParent = nullptr;
    }

    void ZComponentWindow::PushSubWindow(const char* psName, IGUIElement* pGuiElement, bool bReset, bool bFocus)
    {
        if (!psName)
            return;

        IWindowInterface* pWindow = g_pGameData->m_pMenuElements->GetWindow(psName);
        uint32_t iStackSize = m_WindowStack.size();

        if (bReset)
        {
            if (iStackSize && (iStackSize != 1 || m_WindowStack[iStackSize - 1] != pWindow))
            {
                m_WindowStack[iStackSize - 1]->CloseWindow(m_pResourceManager, false);
                m_WindowStack.clear();
                iStackSize = 0;
            }
        }

        if (iStackSize && m_WindowStack[iStackSize - 1] == pWindow)
        {
            if (bFocus)
            {
                pWindow->GrapFocus();
                m_bSubWindowFocus = true;
            }

            return;
        }

        if (iStackSize)
            m_WindowStack[iStackSize - 1]->CloseWindow(m_pResourceManager, false);

        if (pGuiElement)
            pGuiElement->OpenSubwindow();

        m_WindowStack.push_back(pWindow);
        OpenSubWindow(pWindow, true, m_pSubwindowParent, bFocus);

        if (bFocus)
            m_bSubWindowFocus = true;
    }

    bool ZComponentWindow::PopSubWindow()
    {
        const uint32_t iStackSize = m_WindowStack.size();

        if (!m_bSubWindowFocus)
            return false;

        if (iStackSize != 1)
        {
            m_WindowStack[iStackSize - 1]->CloseWindow(m_pResourceManager, false);
            OpenSubWindow(m_WindowStack[iStackSize - 2], false, m_pSubwindowParent, true);
            m_WindowStack.pop_back();
            return true;
        }

        IWindowInterface* pWindow = m_WindowStack[0];
        m_bSubWindowFocus = false;

        for (uint32_t i = 0; i < m_aGUICurrentElements[1].size(); ++i)
        {
            IGUIElement* pElement = m_aGUICurrentElements[1][i];
            if (strcmp(pWindow->GetName(), pElement->GetWindow()) == 0)
            {
                pElement->SetFocus(true);
                break;
            }
        }

        return true;
    }

    bool ZComponentWindow::Update(int iIndex)
    {
        if (!m_WindowStack.empty())
            m_bSubWindowFocus = m_WindowStack[m_WindowStack.size() - 1]->Update(iIndex);

        for (int i = 0; i < 2; ++i)
        {
            IWindowInterface* pSubWindow = reinterpret_cast<IWindowInterface*>(GetButtomSubWindow());

            for (uint32_t j = 0; j < m_aGUICurrentElements[i].size(); ++j)
            {
                IGUIElement* pElement = m_aGUICurrentElements[i][j];
                bool bFocused = false;

                if (i == 1)
                {
                    if (m_bSubWindowFocus)
                    {
                        const char* pWindowName = pElement->GetWindow();
                        if (pSubWindow && pWindowName)
                            bFocused = strcmp(pSubWindow->GetName(), pWindowName) == 0;
                    }
                    else if (pElement->m_iIndex == iIndex)
                    {
                        PushSubWindow(pElement->GetWindow(), pElement, false, true);
                    }
                }

                pElement->Update(bFocused);
            }
        }

        return true;
    }

    ZWINDOW* ZComponentWindow::GetTopSubWindow()
    {
        const uint32_t iStackSize = m_WindowStack.size();

        if (!iStackSize)
            return nullptr;

        return reinterpret_cast<ZWINDOW*>(m_WindowStack[iStackSize - 1]);
    }

    ZWINDOW* ZComponentWindow::GetButtomSubWindow()
    {
        if (m_WindowStack.empty())
            return nullptr;

        return reinterpret_cast<ZWINDOW*>(m_WindowStack[0]);
    }

    void ZComponentWindow::Cancel()
    {
        CallCancel(m_aGUICurrentElements[0]);
        CallCancel(m_aGUICurrentElements[1]);

        IWindowInterface* pTopWindow = reinterpret_cast<IWindowInterface*>(GetTopSubWindow());
        if (pTopWindow)
            pTopWindow->Cancel();
    }

    void ZComponentWindow::Invalidate()
    {
        ZXMLGUISystem* pGUISystem = g_pGameData->m_pMenuElements->m_pXMLGUISystem;
        IGUIElement* pElementInFocus = pGUISystem->GetElementInFocus();
        bool bFound = false;

        for (uint32_t i = 0; i < m_aGUICurrentElements[1].size(); ++i)
            m_aGUICurrentElements[1][i]->ReleaseResources(m_pResourceManager);

        m_aGUICurrentElements[1].clear();

        for (uint32_t i = 0; i < m_aGUIElements[1].size(); ++i)
        {
            IGUIElement* pElement = m_aGUIElements[1][i];
            if (pElement->Use())
            {
                m_aGUICurrentElements[1].push_back(pElement);

                if (pElement == pElementInFocus)
                    bFound = true;
            }
        }

        float aBrowserPos[2] = { m_v2BrowserOffset.x, m_v2BrowserOffset.y };
        SetupGUIElements(aBrowserPos, m_eNavigation, m_aGUICurrentElements[1], m_pResourceManager, m_pParent, nullptr);

        if (bFound)
        {
            pGUISystem->SetFocus();

            pElementInFocus->SetFocus(true);

            const uint32_t iStackSize = m_WindowStack.size();
            ZASSERT(iStackSize > 0);

            IWindowInterface* pTopWindow = m_WindowStack[iStackSize - 1];
            pTopWindow->CloseWindow(m_pResourceManager, false);
            m_WindowStack.pop_back();

            PushSubWindow(pTopWindow->GetName(), nullptr, false, false);
        }
    }

    void ZComponentWindow::GrapFocus()
    {
        SetFirstFocus(m_aGUICurrentElements[1]);
    }

    void ZComponentWindow::OpenSubWindow(IWindowInterface* pWindow, bool bReset, ZWINGROUP* pWinGroup, bool bFocus)
    {
        ZVector2 vPos;
        pWindow->GetTopLeftPos(vPos, m_v2WindowSize);
        pWinGroup->SetPos(vPos.x, vPos.y, 0.0f);
        pWindow->OpenWindow(m_pResourceManager, bReset, pWinGroup, bFocus);
    }
}
