#include <Glacier/GUI/XMLInterface/Elements/ZGUIToggle.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <cstring>


namespace Glacier
{
    ZGUIToggle::ZGUIToggle()
    {
        m_pResourceManager = nullptr;
        m_pParent = nullptr;
    }

    ZGUIToggle::~ZGUIToggle()
    {
    }

    void ZGUIToggle::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "GuiElement") == 0)
            m_vecGraphic.push_back(static_cast<ZGraphic*>(pEntry));
    }

    ZGUIElementLink ZGUIToggle::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        m_pResourceManager = pResourceManager;
        m_pParent = pGroup;

        return ZToggle::Setup(pfPos, pResourceManager, pGroup);
    }

    void ZGUIToggle::ReleaseResources(ZResourceManager* pResourceManager)
    {
        for (uint32_t i = 0; i < m_vecGraphic.size(); ++i)
            m_vecGraphic[i]->ReleaseResources(m_pResourceManager);

        ZToggle::ReleaseResources(pResourceManager);
    }

    void ZGUIToggle::OptionChanged()
    {
        SetupGraphic();
    }

    void ZGUIToggle::SetupGraphic()
    {
        const uint32_t iValue = static_cast<uint32_t>(GetOptionValue());
        if (iValue >= m_vecGraphic.size())
            return;

        for (uint32_t i = 0; i < m_vecGraphic.size(); ++i)
            m_vecGraphic[i]->ReleaseResources(m_pResourceManager);

        float aPos[2] = { 0.0f, 0.0f };
        m_vecGraphic[iValue]->Setup(aPos, m_pResourceManager, m_pZButton);

        if (m_pZButton)
            m_pZButton->GetSystem()->ForceUpdateMouse();
    }

    void ZGUIToggle::ChangeColorSet(ZColorSet* pColorSet)
    {
        for (uint32_t i = 0; i < m_vecGraphic.size(); ++i)
            m_vecGraphic[i]->m_pColorSet = pColorSet;

        SetupGraphic();
    }
}
