#include <Glacier/GUI/XMLInterface/Elements/ZGUISliderText.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggleTextItem.h>
#include <Glacier/GUI/XMLInterface/System/IOptionsInterface.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/ZSlider.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>
#include <cstring>


namespace Glacier
{
    ZGUISliderText::ZGUISliderText()
    {
        m_iNumOfTextAdded = 0;
        m_bUseOptionText = false;
    }

    ZGUISliderText::~ZGUISliderText()
    {
    }

    void ZGUISliderText::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUISlider::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bUseOptionText, ppParams, "UseOptionText", false);
    }

    void ZGUISliderText::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "ToggleTextItem") != 0)
            return;

        ZASSERT(m_iNumOfTextAdded < 8);

        m_sTextValues[m_iNumOfTextAdded++] = static_cast<ZToggleTextItem*>(pEntry)->m_sText;
    }

    void ZGUISliderText::SetValue(int iValue)
    {
        if (m_bSyncOption)
            SetOptionValue(iValue);

        ZASSERT(iValue >= 0 && (iValue < 8 || m_bUseOptionText));

        if (m_bUseOptionText)
        {
            zstring sText;

            if (!m_sOption.empty())
                m_pOptionInterface->GetOptionTextValue(m_sOption.c_str(), sText);

            m_pSlider->SetExtraText(sText.c_str());
        }
        else
        {
            const char* pszText = g_pEngineData->m_pLocaleResources->GetResourceText(
                "AllLevels/Interface", m_sTextValues[iValue].c_str());
            m_pSlider->SetExtraText(pszText);
        }
    }
}
