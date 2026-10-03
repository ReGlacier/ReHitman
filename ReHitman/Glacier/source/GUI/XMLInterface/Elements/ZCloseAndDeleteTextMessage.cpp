#include <Glacier/GUI/XMLInterface/Elements/ZCloseAndDeleteTextMessage.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>


namespace Glacier
{
    ZCloseAndDeleteTextMessage::ZCloseAndDeleteTextMessage()
    {
    }

    ZCloseAndDeleteTextMessage::~ZCloseAndDeleteTextMessage()
    {
    }

    void ZCloseAndDeleteTextMessage::Click(eZWUserEvents eEvent, int iSubId, ZXMLGUISystem* pGUI)
    {
        ZNavigator::Click(eEvent, iSubId, pGUI);

        if (eEvent == static_cast<eZWUserEvents>(m_iActivateButton))
        {
            if (m_bCancel)
                pGUI->Cancel();

            ZASSERT(false);
        }
    }
}
