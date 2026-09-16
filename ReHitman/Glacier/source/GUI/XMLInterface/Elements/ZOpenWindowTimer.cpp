#include <Glacier/GUI/XMLInterface/Elements/ZOpenWindowTimer.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/eZWUserEvents.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    ZOpenWindowTimer::ZOpenWindowTimer()
    {
        m_fTime = 5.0f;
    }

    ZOpenWindowTimer::~ZOpenWindowTimer()
    {
    }

    ZGUIElementLink ZOpenWindowTimer::Setup(float*, ZResourceManager*, ZWINGROUP*)
    {
        m_fElapsedTime = 0.0f;
        return ZGUIElementLink();
    }

    void ZOpenWindowTimer::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZOpenWindow::readParams(ppParams, pElems);

        GuiOption::readInt2(m_fTime, ppParams, "Time");
    }

    void ZOpenWindowTimer::Update(bool)
    {
        m_fElapsedTime += g_pSysInterface->m_fActualTimeDelta;

        if (m_fElapsedTime > m_fTime)
        {
            Click(eZW_SELECT, 0, g_pGameData->m_pMenuElements->m_pXMLGUISystem);
            return;
        }

        if (ZSysInterface::GetOption("DisableMenuTimers", nullptr))
            Click(eZW_SELECT, 0, g_pGameData->m_pMenuElements->m_pXMLGUISystem);
    }

    bool ZOpenWindowTimer::SetFocus(bool)
    {
        return false;
    }
}
