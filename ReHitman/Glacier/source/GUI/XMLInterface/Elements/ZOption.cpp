#include <Glacier/GUI/XMLInterface/Elements/ZOption.h>
#include <Glacier/GUI/XMLInterface/System/IOptionsInterface.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/Data/ZGameData.h>


namespace Glacier
{
    ZOption::ZOption()
    {
        m_iLowerBound = 0;
        m_iUpperBound = 0;
        m_iStep = 1;
        m_bTakeFocus = true;
        m_iOption = 0;
        m_pOptionInterface = g_pGameData->m_pMenuElements->GetOptionsInterface();
    }

    void ZOption::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readInt(m_iLowerBound, ppParams, "LowerBound");
        GuiOption::readInt(m_iUpperBound, ppParams, "UpperBound");
        GuiOption::readInt(m_iStep, ppParams, "Step");
        GuiOption::readString(m_sOption, "Option", ppParams);
        GuiOption::readBool(&m_bTakeFocus, ppParams, "TakeFocus", false);
    }

    ZGUIElementLink ZOption::Setup(float*, ZResourceManager*, ZWINGROUP*)
    {
        m_pOptionInterface->UseDynamicBounds(m_sOption.c_str(), m_iLowerBound, m_iUpperBound);

        if (g_pGameData->m_pMenuElements->m_pXMLGUISystem->m_bWindowPush)
            m_iBackup = GetOptionValue();

        return ZGUIElementLink();
    }

    void ZOption::Cancel()
    {
        m_pOptionInterface->SetFeedback(false);
        SetOptionValue(m_iBackup);
        m_pOptionInterface->SetFeedback(true);
    }

    int32_t ZOption::GetOptionValue()
    {
        if (m_sOption.empty())
            return m_iOption;

        return m_pOptionInterface->GetOptionValue(m_sOption.c_str(), 0);
    }

    void ZOption::SetOptionValue(int32_t iValue)
    {
        if (m_sOption.empty())
        {
            m_iOption = iValue;
            return;
        }

        m_pOptionInterface->SetOptionValue(m_sOption.c_str(), iValue, 0);
    }

    int32_t ZOption::GetUpperBound()
    {
        return m_iUpperBound;
    }

    void ZOption::SetUpperBound(int32_t iUpperBound)
    {
        m_iUpperBound = iUpperBound;
    }

    bool ZOption::IsOptionEnabled() const
    {
        return m_pOptionInterface->IsOptionEnabled(m_sOption.c_str(), 0);
    }
}
