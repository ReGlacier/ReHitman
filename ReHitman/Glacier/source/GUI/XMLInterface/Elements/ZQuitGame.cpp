#include <Glacier/GUI/XMLInterface/Elements/ZQuitGame.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    ZQuitGame::ZQuitGame()
    {
    }

    ZQuitGame::~ZQuitGame()
    {
    }

    void ZQuitGame::Click(eZWUserEvents eEvent, int iSubId, ZXMLGUISystem* pGUI)
    {
        ZNavigator::Click(eEvent, iSubId, pGUI);

        if (eEvent == static_cast<eZWUserEvents>(m_iActivateButton))
            g_pSysInterface->m_pEngineData->UnloadScene();
    }
}
