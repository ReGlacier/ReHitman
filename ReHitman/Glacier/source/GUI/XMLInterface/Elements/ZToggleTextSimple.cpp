#include <Glacier/GUI/XMLInterface/Elements/ZToggleTextSimple.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggleTextItem.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>
#include <cstring>


namespace Glacier
{
    ZToggleTextSimple::ZToggleTextSimple()
    {
        m_iNumOfTextAdded = 0;
    }

    ZToggleTextSimple::~ZToggleTextSimple()
    {
    }

    void ZToggleTextSimple::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "ToggleTextItem") != 0)
            return;

        ZASSERT(m_iNumOfTextAdded < NUM_OF_TEXT_STRINGS);

        SetUpperBound(m_iNumOfTextAdded);
        m_asTextValues[m_iNumOfTextAdded] = zstring(static_cast<ZToggleTextItem*>(pEntry)->GetText());
        ++m_iNumOfTextAdded;
    }

    void ZToggleTextSimple::OptionChanged()
    {
        const int32_t iValue = GetOptionValue();
        ZASSERT(iValue >= 0 && iValue < m_iNumOfTextAdded);

        const char* pszText = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_sText.c_str());
        const char* pszValue = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_asTextValues[iValue].c_str());

        zstring sText;
        sText.format("%s: %s", pszText, pszValue);
        m_pZButton->SetText(sText.c_str());
    }
}
