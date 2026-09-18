#include <Glacier/GUI/XMLInterface/Elements/ZGUISlider.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/ZSlider.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>


namespace Glacier
{
    ZGUISlider::ZGUISlider()
    {
        m_pSlider = nullptr;
        m_eTextAlignment = ERIGHT;
        m_fSliderSize = 100.0f;
        m_fSliderOffset = 0.0f;
        m_bDisplayAsFloat = false;
        m_fFloatMultiplier = 1.0f;
        m_bSyncOption = true;
    }

    ZGUISlider::~ZGUISlider()
    {
    }

    void ZGUISlider::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZOption::readParams(ppParams, pElems);

        GuiOption::readString(m_sText, "Text", ppParams);
        ReadElementAlignment(m_eTextAlignment, ppParams, "TextAlignment");
        GuiOption::readInt2(m_fSliderSize, ppParams, "SliderSize");
        GuiOption::readInt2(m_fSliderOffset, ppParams, "SliderOffset");
        GuiOption::readBool(&m_bDisplayAsFloat, ppParams, "DisplayAsFloat", false);
        GuiOption::readInt2(m_fFloatMultiplier, ppParams, "FloatMultiplier");
    }

    ZGUIElementLink ZGUISlider::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        ZOption::Setup(pfPos, pResourceManager, pGroup);

        ZASSERT(m_pSlider == nullptr);

        m_pSlider = pResourceManager->GetSlider(pfPos, m_pColorSet, pGroup, m_iIndex, m_iLowerBound,
            m_iUpperBound, (m_iUpperBound - m_iLowerBound) / m_iStep + 1, m_pButtonGraphic, 0x29,
            m_eFontType, m_bShadow, m_eAlignment, m_fSliderSize, m_fSliderOffset);

        if (!m_pSlider)
            return ZGUIElementLink();

        m_pSlider->SetText(g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_sText.c_str()));

        const int32_t iValue = GetOptionValue();

        m_bSyncOption = false;
        m_pSlider->SetValue(iValue, true);
        SetValue(iValue);
        m_bSyncOption = true;

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
            GetSize(m_pSlider, aSize);
            v2Size.x = aSize[0];
            v2Size.y = aSize[1];
        }

        if (!IsOptionEnabled())
            m_pSlider->Disable();

        return ZGUIElementLink(v2Size.x, v2Size.y, m_pSlider, m_pSlider, m_pSlider, m_pSlider);
    }

    void ZGUISlider::ReleaseResources(ZResourceManager* pResourceManager)
    {
        pResourceManager->ReleaseSlider(m_pSlider);
        m_pSlider = nullptr;
    }

    bool ZGUISlider::SetFocus(bool)
    {
        if (!m_pSlider)
            return false;

        ZWINDOWS* pWindows = m_pSlider->GetSystem();
        pWindows->SetFocusedControl(m_pSlider);
        return true;
    }

    void ZGUISlider::SetValue(int iValue)
    {
        if (m_bSyncOption)
            SetOptionValue(iValue);

        char szText[16];

        if (m_bDisplayAsFloat)
            g_pSysInterface->BeforeFormat()->SPrintF(szText, "%.1f", static_cast<double>(iValue) * m_fFloatMultiplier);
        else
            g_pSysInterface->BeforeFormat()->SPrintF(szText, "%i", iValue);

        m_pSlider->SetExtraText(szText);
    }

    void ZGUISlider::Invalidate()
    {
        if (IsOptionEnabled())
            m_pSlider->Enable();
        else
            m_pSlider->Disable();

        const int32_t iValue = GetOptionValue();
        m_pSlider->SetValue(iValue, true);

        m_bSyncOption = false;
        SetValue(iValue);
        m_bSyncOption = true;
    }
}
