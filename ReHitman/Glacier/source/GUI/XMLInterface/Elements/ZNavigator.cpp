#include <Glacier/GUI/XMLInterface/Elements/ZNavigator.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>


namespace Glacier
{
    ZNavigator::ZNavigator()
    {
        m_pButton = nullptr;
        m_iActivateButton = 127;
        m_iMoveButton = -1;
        m_bReceiveAllInput = false;
        m_bLink = true;
        m_bDisable = false;
        m_bTakeFocus = true;
        m_bMouseColiOnHiddenObjects = false;
        SetType(0x29);
    }

    ZNavigator::~ZNavigator()
    {
    }

    void ZNavigator::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readString(m_sText, "Text", ppParams);
        GuiOption::readBool(&m_bReceiveAllInput, ppParams, "ReceiveAllInput", false);
        GuiOption::readVirtualKey(&m_iActivateButton, ppParams, "ActivateButton");
        GuiOption::readBool(&m_bLink, ppParams, "Link", false);
        GuiOption::readString(m_sMoveFocus, "MoveFocus", ppParams);
        GuiOption::readMoveButton(&m_iMoveButton, ppParams, "MoveButton");
        GuiOption::readBool(&m_bDisable, ppParams, "Disable", false);
        GuiOption::readBool(&m_bTakeFocus, ppParams, "TakeFocus", false);
        GuiOption::readBool(&m_bMouseColiOnHiddenObjects, ppParams, "MouseColiOnHiddenObjects", false);
    }

    ZGUIElementLink ZNavigator::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        ZASSERT(m_pButton == nullptr);

        const ZVector2 vPos { pfPos[0], pfPos[1] };
        m_pButton = pResourceManager->GetButton(vPos, m_pColorSet, pGroup, m_iIndex, nullptr, nullptr,
            m_eAlignment, m_pButtonGraphic, m_iType, m_eFontType, true, false);

        if (!m_pButton)
            return ZGUIElementLink();

        SetText();

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
            GetSize(m_pButton, aSize);
            v2Size.x = aSize[0];
            v2Size.y = aSize[1];
        }

        if (m_bReceiveAllInput)
            g_pGameData->m_pMenuElements->m_pXMLGUISystem->AddRecieveAllInput(this);

        if (m_bDisable)
        {
            for (ZBaseGeom* pBaseGeom = m_pButton->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
            {
                ZWINOBJ* pWinObj = geom_cast<ZWINOBJ>(pBaseGeom->GetGeom());
                if (pWinObj)
                    pWinObj->m_bAnimateAlpha = false;
            }

            return ZGUIElementLink(v2Size.x, v2Size.y);
        }

        if (m_bLink)
            return ZGUIElementLink(v2Size.x, v2Size.y, m_pButton);

        return ZGUIElementLink(v2Size.x, v2Size.y);
    }

    void ZNavigator::ReleaseResources(ZResourceManager* pResourceManager)
    {
        pResourceManager->ReleaseButton(m_pButton);
        m_pButton = nullptr;

        g_pGameData->m_pMenuElements->m_pXMLGUISystem->RemoveRecieveAllInput(this);
    }

    bool ZNavigator::SetFocus(bool bSet)
    {
        if (!m_pButton || !m_bLink || (!m_bTakeFocus && !bSet))
            return false;

        ZWINDOWS* pWindows = m_pButton->GetSystem();
        pWindows->SetFocusedControl(m_pButton);
        return true;
    }

    void ZNavigator::Click(eZWUserEvents eEvent, int, ZXMLGUISystem*)
    {
        if (m_iMoveButton == static_cast<int32_t>(eEvent))
        {
            IGUIElement* pElement = g_pGameData->m_pMenuElements->GetGUIElement(m_sMoveFocus.c_str());
            if (pElement)
                pElement->SetFocus(true);
        }
    }

    void ZNavigator::SetText()
    {
        const char* pText = m_sText.c_str();
        if (!m_sText.empty())
        {
            if (*pText == '%')
                ++pText;
            else
                pText = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", pText);
        }

        zstring sText;
        g_pGameData->InsertIconsInText(sText, pText, false);
        m_pButton->SetText(sText.c_str());
    }
    void ZNavigator::ForceSetText(const char* pText)
    {
        m_sText = zstring(pText);
    }

    const char* ZNavigator::GetText()
    {
        return m_sText.c_str();
    }

    void ZNavigator::SetType(uint32_t iType)
    {
        m_iType = iType;
    }
}
