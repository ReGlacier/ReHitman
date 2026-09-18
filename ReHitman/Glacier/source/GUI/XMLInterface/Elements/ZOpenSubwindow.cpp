#include <Glacier/GUI/XMLInterface/Elements/ZOpenSubwindow.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/Control/ZBUTTON.h>


namespace Glacier
{
    ZOpenSubwindow::ZOpenSubwindow()
    {
        m_szWindow[0] = '\0';
        m_bResetStack = false;
    }

    ZOpenSubwindow::~ZOpenSubwindow()
    {
    }

    void ZOpenSubwindow::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZNavigator::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bResetStack, ppParams, "ResetStack", false);
        ZGUI::ReadText(m_szWindow, "Window", ppParams);
    }

    void ZOpenSubwindow::Update(bool bSet)
    {
        if (bSet)
        {
            m_pButton->SetState(2);
            return;
        }

        if (m_pButton->GetState() == 2)
            m_pButton->SetState(1);
    }

    void ZOpenSubwindow::Click(eZWUserEvents eEvent, int iSubId, ZXMLGUISystem* pGUI)
    {
        ZNavigator::Click(eEvent, iSubId, pGUI);

        if (eEvent == static_cast<eZWUserEvents>(m_iActivateButton))
        {
            IWindowInterface* pTopWindow = pGUI->GetTopWindow();
            if (pTopWindow)
                pTopWindow->PushSubWindow(GetWindow(), this, true, m_bResetStack);
        }
    }

    const char* ZOpenSubwindow::GetWindow()
    {
        return m_szWindow;
    }
}
