#include <Glacier/GUI/XMLInterface/Elements/ZXMLFrame.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>


namespace Glacier
{
    ZXMLFrame::ZXMLFrame()
    {
        m_v2Size.x = 100.0f;
        m_v2Size.y = 100.0f;
        m_pFrame = nullptr;
    }

    ZXMLFrame::~ZXMLFrame()
    {
    }

    void ZXMLFrame::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readString(m_sFrame, "Frame", ppParams);
        GuiOption::readInt2(m_v2Size.x, ppParams, "W");
        GuiOption::readInt2(m_v2Size.y, ppParams, "H");
    }

    ZGUIElementLink ZXMLFrame::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        ZASSERT(m_pFrame == nullptr);

        const ZVector2 vPos { pfPos[0], pfPos[1] };

        m_pFrame = pResourceManager->GetFrame(vPos, m_pColorSet, pGroup, m_v2Size, m_sFrame.c_str(), m_eAlignment);

        return ZGUIElementLink(m_v2Size.x, m_v2Size.y);
    }

    void ZXMLFrame::ReleaseResources(ZResourceManager* pResourceManager)
    {
        pResourceManager->ReleaseFrame(m_pFrame);
        m_pFrame = nullptr;
    }
}
