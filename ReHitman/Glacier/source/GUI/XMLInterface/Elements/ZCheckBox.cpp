#include <Glacier/GUI/XMLInterface/Elements/ZCheckBox.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>


namespace Glacier
{
    ZCheckBox::ZCheckBox()
    {
        m_pResourceManager = nullptr;
        m_pBGChecked = nullptr;
        m_pBGUnchecked = nullptr;
        m_bSetup = false;
    }

    ZCheckBox::~ZCheckBox()
    {
    }

    void ZCheckBox::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZToggle::readParams(ppParams, pElems);

        zstring sName;
        GuiOption::readString(sName, "BGChecked", ppParams);
        m_pBGChecked = pElems->GetButtonGraphic(sName.c_str());

        GuiOption::readString(sName, "BGUnchecked", ppParams);
        m_pBGUnchecked = pElems->GetButtonGraphic(sName.c_str());
    }

    ZGUIElementLink ZCheckBox::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        m_iUpperBound = 1;
        m_iStep = 1;
        m_bSetup = true;
        m_iLowerBound = 0;

        const ZGUIElementLink link = ZToggle::Setup(pfPos, pResourceManager, pGroup);

        m_bSetup = false;
        m_pResourceManager = pResourceManager;
        OptionChanged();

        return link;
    }

    void ZCheckBox::OptionChanged()
    {
        if (m_bSetup)
            return;

        const char* pszText = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_sText.c_str());
        m_pZButton->SetText(pszText);

        m_pResourceManager->ReleaseButtonGraphic(m_pZButton);

        ZButtonGraphic* pButtonGraphic = GetOptionValue() == 1 ? m_pBGChecked : m_pBGUnchecked;
        m_pResourceManager->AddButtonGraphic(m_pZButton, pButtonGraphic, m_pColorSet, ELEFT, nullptr, nullptr);

        m_pZButton->GetSystem()->ForceUpdateMouse();
    }
}
