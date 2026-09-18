#include <Glacier/GUI/XMLInterface/Elements/ZCloseAndDeleteVoiceMessage.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>


namespace Glacier
{
    ZCloseAndDeleteVoiceMessage::ZCloseAndDeleteVoiceMessage()
    {
    }

    ZCloseAndDeleteVoiceMessage::~ZCloseAndDeleteVoiceMessage()
    {
    }

    void ZCloseAndDeleteVoiceMessage::Click(eZWUserEvents eEvent, int iSubId, ZXMLGUISystem* pGUI)
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
