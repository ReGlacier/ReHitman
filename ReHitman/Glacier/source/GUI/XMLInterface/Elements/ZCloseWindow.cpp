#include <Glacier/GUI/XMLInterface/Elements/ZCloseWindow.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>


namespace Glacier
{
    ZCloseWindow::ZCloseWindow()
    {
        m_bFade = false;
        m_bCloseAll = false;
        m_bCancel = false;
    }

    ZCloseWindow::~ZCloseWindow()
    {
    }

    void ZCloseWindow::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZNavigator::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bCloseAll, ppParams, "CloseAll", false);
        GuiOption::readBool(&m_bCancel, ppParams, "Cancel", false);
    }

    void ZCloseWindow::Click(eZWUserEvents eEvent, int iSubId, ZXMLGUISystem* pGUI)
    {
        ZNavigator::Click(eEvent, iSubId, pGUI);

        if (eEvent == static_cast<eZWUserEvents>(m_iActivateButton))
        {
            if (m_bCancel)
                pGUI->Cancel();

            pGUI->CloseWindow(m_bCloseAll);
        }
    }
}
