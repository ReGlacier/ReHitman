#include <Glacier/GUI/XMLInterface/Elements/ZToggle.h>
#include <Glacier/GUI/XMLInterface/System/IOptionsInterface.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/ResourceCollection.h>


namespace Glacier
{
    ZToggle::ZToggle()
    {
        m_pZButton = nullptr;
        m_bActive = true;
    }

    ZToggle::~ZToggle()
    {
    }

    void ZToggle::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZOption::readParams(ppParams, pElems);

        GuiOption::readString(m_sText, "Text", ppParams);
    }

    ZGUIElementLink ZToggle::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        ZOption::Setup(pfPos, pResourceManager, pGroup);

        ZASSERT(m_pZButton == nullptr);

        const ZVector2 vPos { pfPos[0], pfPos[1] };
        m_pZButton = pResourceManager->GetButton(vPos, m_pColorSet, pGroup, m_iIndex, nullptr, nullptr,
            m_eAlignment, m_pButtonGraphic, 0x29, FT_MENU, true, false);

        OptionChanged();

        ZVector2 v2Size;
        bool bGetGroupSize = true;

        if (m_pButtonGraphic)
        {
            m_pButtonGraphic->GetButtonSize(&v2Size);
            bGetGroupSize = v2Size.x == 0.0f;
        }

        if (bGetGroupSize)
        {
            float aSize[2];
            GetSize(m_pZButton, aSize);
            v2Size.x = aSize[0];
            v2Size.y = aSize[1];
        }

        SetActive(m_bActive);

        if (!IsOptionEnabled())
            m_pZButton->Disable();

        return ZGUIElementLink(v2Size.x, v2Size.y, m_pZButton);
    }

    void ZToggle::ReleaseResources(ZResourceManager* pResourceManager)
    {
        pResourceManager->ReleaseButton(m_pZButton);
        m_pZButton = nullptr;
    }

    void ZToggle::Click(eZWUserEvents eEvent, int, ZXMLGUISystem*)
    {
        int32_t iValue;

        switch (eEvent)
        {
        case eZW_LEFT:
        case eZW_RBUTTON:
            g_pGameData->GetAudioOSDInterface().PlaySound(5);
            iValue = GetOptionValue() - m_iStep;
            if (iValue < m_iLowerBound)
                iValue = m_iUpperBound;
            break;

        case eZW_RIGHT:
            g_pGameData->GetAudioOSDInterface().PlaySound(5);
            iValue = m_iStep + GetOptionValue();
            if (iValue > m_iUpperBound)
                iValue = m_iLowerBound;
            break;

        case eZW_SELECT:
            iValue = m_iStep + GetOptionValue();
            if (iValue > m_iUpperBound)
                iValue = m_iLowerBound;
            break;

        default:
            return;
        }

        SetOptionValue(iValue);
        OptionChanged();
    }

    bool ZToggle::SetFocus(bool)
    {
        if (!m_pZButton || !m_bTakeFocus)
            return false;

        ZWINDOWS* pWindows = m_pZButton->GetSystem();
        pWindows->SetFocusedControl(m_pZButton);
        return true;
    }

    void ZToggle::OptionChanged()
    {
        const char* pszText = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_sText.c_str());

        zstring sText;
        sText.format("%s: %i", pszText, GetOptionValue());
        m_pZButton->SetText(sText.c_str());
    }

    void ZToggle::Invalidate()
    {
        if (IsOptionEnabled())
            m_pZButton->Enable();
        else
            m_pZButton->Disable();

        OptionChanged();
    }

    void ZToggle::SetActive(bool bActive)
    {
        m_bActive = bActive;

        if (bActive)
            m_pZButton->Enable();
        else
            m_pZButton->Disable();
    }
}
