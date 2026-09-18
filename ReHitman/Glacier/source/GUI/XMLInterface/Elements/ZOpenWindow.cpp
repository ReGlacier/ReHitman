#include <Glacier/GUI/XMLInterface/Elements/ZOpenWindow.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/Data/ZGameData.h>


namespace Glacier
{
    ZOpenWindow::ZOpenWindow()
    {
        m_bPushOnStack = true;
        m_bResetStack = false;
        m_bRollBack = false;
        m_bKeepRollBackWindow = false;
    }

    ZOpenWindow::~ZOpenWindow()
    {
    }

    void ZOpenWindow::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZNavigator::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bPushOnStack, ppParams, "PushOnStack", false);
        GuiOption::readBool(&m_bResetStack, ppParams, "ResetStack", false);
        GuiOption::readBool(&m_bRollBack, ppParams, "RollBack", false);
        GuiOption::readString(m_sWindow, "Window", ppParams);
        GuiOption::readString(m_sRollBackPushWindow, "PushWindow", ppParams);
        GuiOption::readBool(&m_bKeepRollBackWindow, ppParams, "KeepRollbackWindow", false);
    }

    void ZOpenWindow::Click(eZWUserEvents eEvent, int iSubId, ZXMLGUISystem* pGUI)
    {
        ZNavigator::Click(eEvent, iSubId, pGUI);

        if (eEvent == static_cast<eZWUserEvents>(m_iActivateButton))
            Execute();
    }

    void ZOpenWindow::Execute()
    {
        ZXMLGUISystem* pXMLGUISystem = g_pGameData->m_pMenuElements->m_pXMLGUISystem;

        if (m_bRollBack)
        {
            if (m_sRollBackPushWindow.empty())
                pXMLGUISystem->RollBack(m_sWindow.c_str(), nullptr, false, false);
            else
                pXMLGUISystem->RollBack(m_sWindow.c_str(), m_sRollBackPushWindow.c_str(), true, m_bKeepRollBackWindow);
        }
        else
        {
            pXMLGUISystem->OpenWindow(m_sWindow.c_str(), m_bPushOnStack, m_bResetStack);
        }
    }

    void ZOpenWindow::setWindowTarget(const char* pszWindowTarget)
    {
        m_sWindow = pszWindowTarget;
    }

    zstring ZOpenWindow::getWindowTarget()
    {
        return m_sWindow;
    }
}
