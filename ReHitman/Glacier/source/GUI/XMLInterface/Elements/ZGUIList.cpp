#include <Glacier/GUI/XMLInterface/Elements/ZGUIList.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIDataInterfaces.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/Font/ZTTFONT.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/ResourceCollection.h>

#include <cstring>


namespace Glacier
{
    namespace
    {
        ZWINDOWS* GetMenuZWindows()
        {
            int32_t iRef = 0;
            g_pEngineData->GetSceneCom()->GetVal("MenuZWindows", &iRef);
            return static_cast<ZWINDOWS*>(ZGEOM::RefToPtr(static_cast<ZREF>(iRef)));
        }
    }

    ZGUIList::ZGUIList()
    {
        m_iIdTop = 0;
        m_iNumOfButtonsUsed = 0;
        m_iLineDistance = -1;
        m_iLastFocusId = -1;
        m_iLastButtonFocus = 0;

        for (int32_t i = 0; i < MAX_NUM_OF_VISIBLE_BUTTONS; ++i)
            m_paButtons[i] = nullptr;

        m_pWinBorderGraphic = nullptr;
        m_szBorderName[0] = 0;
        m_pDataInterface = nullptr;
        m_szInterface[0] = 0;

        m_bUseTemplate = false;
        m_v2Size.x = 300.0f;
        m_v2Size.y = 300.0f;
        m_iNumOfVisibleButtons = -1;
        m_pBorderGraphic = nullptr;
        m_pButtonBorder = nullptr;
        m_pResourceManager = nullptr;
        m_pIconColorSet = nullptr;
        m_bListFocus = false;
        m_bInvalidate = false;
        m_v2DefaultPos.x = 0.0f;
        m_v2DefaultPos.y = 0.0f;
        m_pParent = nullptr;
        m_bDisabled = false;
        m_bLinkOut = false;
        m_bPlaceArrowsButtonRight = false;
        m_bUseDoubleArrowPos = false;
        m_vArrowsPos.x = 0.0f;
        m_vArrowsPos.y = 0.0f;
        m_vArrowsPos2.x = 0.0f;
        m_vArrowsPos2.y = 0.0f;

        m_paArrowButtons[0] = nullptr;
        m_paArrowButtons[1] = nullptr;
        m_paBorderGraphicArrows[0] = nullptr;
        m_paBorderGraphicArrows[1] = nullptr;

        m_iControlPrompt = 9;
    }

    ZGUIList::~ZGUIList()
    {
    }

    void ZGUIList::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        ZGUI::ReadText(m_szBorderName, "Border", ppParams);
        GuiOption::readString(m_sWindow, "Window", ppParams);
        ZGUI::ReadText(m_szInterface, "Interface", ppParams);
        GuiOption::readInt(m_iLineDistance, ppParams, "LineDistance");
        GuiOption::readInt(m_iNumOfVisibleButtons, ppParams, "NumVisibleButtons");
        ReadV2(reinterpret_cast<float(&)[2]>(m_v2Size), ppParams, "Size");
        GuiOption::readBool(&m_bListFocus, ppParams, "ListFocus", false);
        GuiOption::readBool(&m_bDisabled, ppParams, "Disabled", false);
        GuiOption::readBool(&m_bLinkOut, ppParams, "LinkOut", false);
        GuiOption::readBool(&m_bPlaceArrowsButtonRight, ppParams, "PlaceArrowsButtonRight", false);
        ReadV2(reinterpret_cast<float(&)[2]>(m_vArrowsPos), ppParams, "ArrowsPos");
        ReadV2(reinterpret_cast<float(&)[2]>(m_vArrowsPos2), ppParams, "ArrowsPos2");
        GuiOption::readBool(&m_bUseDoubleArrowPos, ppParams, "UseDoubleArrowPos", false);

        const char* pBorderGraphic = GUI::GetAttr(ppParams, "ButtonGraphicBorder", false);
        if (pBorderGraphic)
            m_pBorderGraphic = pElems->GetButtonGraphic(pBorderGraphic);

        const char* pIconColorSet = GUI::GetAttr(ppParams, "IconColorSet", false);
        if (!pIconColorSet)
            pIconColorSet = "Default";
        m_pIconColorSet = pElems->GetColorSet(pIconColorSet);

        const char* pUpGraphic = GUI::GetAttr(ppParams, "ButtonGraphicUp", false);
        if (pUpGraphic)
            m_paBorderGraphicArrows[0] = pElems->GetButtonGraphic(pUpGraphic);

        const char* pDownGraphic = GUI::GetAttr(ppParams, "ButtonGraphicDown", false);
        if (pDownGraphic)
            m_paBorderGraphicArrows[1] = pElems->GetButtonGraphic(pDownGraphic);
    }

    void ZGUIList::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "ItemLine") != 0 || m_bUseTemplate)
            return;

        m_bUseTemplate = true;

        const ZGUIGroup* pTemplate = static_cast<const ZGUIGroup*>(pEntry);
        m_ListTemplate.m_Group = pTemplate->m_Group;
        m_ListTemplate.m_v2Size = pTemplate->m_v2Size;
    }

    void ZGUIList::Update(bool)
    {
        ZWINDOWS* pWindows = GetMenuZWindows();

        if (pWindows)
        {
            ZWINGROUP* pFocused = pWindows->GetFocusedControl();

            m_pDataInterface->Update();

            for (int32_t i = 0; i < m_iNumOfButtonsUsed; ++i)
            {
                if (pFocused == m_paButtons[i])
                {
                    const int32_t iId = i + m_iIdTop;
                    m_iLastButtonFocus = i;

                    if (iId != m_iLastFocusId)
                    {
                        m_iLastFocusId = iId;
                        m_pDataInterface->FocusChanged(iId);
                    }
                }
                else
                {
                    const int32_t iSelectedItem = m_pDataInterface->GetSelectedItem();

                    if (iSelectedItem != -1)
                        m_paButtons[i]->SetState(iSelectedItem == i + m_iIdTop ? 2 : 1);
                }
            }
        }

        const int32_t iNumOfItems = m_pDataInterface->GetNumOfItems();
        m_iControlPrompt = 8 * (m_iNumOfButtonsUsed < iNumOfItems) + 1;
    }

    void ZGUIList::Click(eZWUserEvents eEvent, int iId, ZXMLGUISystem*)
    {
        if (eEvent == eZW_SELECT && iId == MAX_NUM_OF_VISIBLE_BUTTONS)
        {
            m_pButtonBorder->GetSystem()->SetFocusedControl(m_paButtons[0]);
            return;
        }

        if (m_pButtonBorder && eEvent == eZW_SELECT2)
        {
            m_pButtonBorder->GetSystem()->SetFocusedControl(m_pButtonBorder);
            return;
        }

        if (eEvent == eZW_MISC1)
        {
            m_pDataInterface->SelectItem(iId + m_iIdTop);
            return;
        }

        if (iId == m_iNumOfButtonsUsed - 1
            && m_iIdTop < m_pDataInterface->GetNumOfItems() - m_iNumOfButtonsUsed
            && eEvent == eZW_DOWN
            && m_iLastButtonFocus == iId)
        {
            ZSDOwner& rOwner = g_pGameData->GetAudioOSDInterface();
            rOwner.PlaySound(4);
            rOwner.AllowSounds();

            ++m_iIdTop;
            SetupTextOnButtons();
            return;
        }

        if (!iId)
        {
            if (m_iIdTop > 0 && eEvent == eZW_UP && !m_iLastButtonFocus)
            {
                ZSDOwner& rOwner = g_pGameData->GetAudioOSDInterface();
                rOwner.PlaySound(4);
                rOwner.AllowSounds();

                --m_iIdTop;
                SetupTextOnButtons();
                return;
            }

            m_pDataInterface->Click(iId + m_iIdTop, eEvent);
            return;
        }

        if (iId == 3334)
        {
            if (eEvent == eZW_SELECT)
            {
                if (m_iIdTop < m_pDataInterface->GetNumOfItems() - m_iNumOfButtonsUsed)
                {
                    ++m_iIdTop;
                    SetupTextOnButtons();
                }

                return;
            }

            m_pDataInterface->Click(iId + m_iIdTop, eEvent);
            return;
        }

        if (iId == 3333 && eEvent == eZW_SELECT)
        {
            if (m_iIdTop > 0)
            {
                --m_iIdTop;
                SetupTextOnButtons();
            }

            return;
        }

        m_pDataInterface->Click(iId + m_iIdTop, eEvent);
    }

    ZGUIElementLink ZGUIList::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        m_pResourceManager = pResourceManager;
        m_pParent = pGroup;
        m_v2DefaultPos.x = pfPos[0];
        m_v2DefaultPos.y = pfPos[1];

        const char* pszInterface = m_szInterface[0] ? m_szInterface : GetName();
        m_pDataInterface = g_pGameData->m_pMenuElements->GetListDataInterface(pszInterface);

        m_pDataInterface->SetWindow(m_sWindow);
        m_pDataInterface->Begin();

        const ZVector2 vPos { pfPos[0], pfPos[1] };

        if (m_szBorderName[0])
            m_pWinBorderGraphic = pResourceManager->GetGraphic(vPos, m_pColorSet, pGroup, m_szBorderName, ELEFT, -1);

        if (!m_bInvalidate)
        {
            if (m_pBorderGraphic)
                m_pButtonBorder = pResourceManager->GetButton(vPos, m_pColorSet, pGroup, GenerateSubId(MAX_NUM_OF_VISIBLE_BUTTONS),
                    nullptr, nullptr, ELEFT, m_pBorderGraphic, 9, FT_MENU, true, false);

            if (m_paBorderGraphicArrows[0])
                m_paArrowButtons[0] = pResourceManager->GetButton(vPos, m_pColorSet, pGroup, GenerateSubId(3333),
                    nullptr, nullptr, ELEFT, m_paBorderGraphicArrows[0], 9, FT_MENU, true, false);

            if (m_paBorderGraphicArrows[1])
                m_paArrowButtons[1] = pResourceManager->GetButton(vPos, m_pColorSet, pGroup, GenerateSubId(3334),
                    nullptr, nullptr, ELEFT, m_paBorderGraphicArrows[1], 9, FT_MENU, true, false);
        }

        float fX = pfPos[0];
        float fY = pfPos[1];

        int32_t iNumOfButtons = m_pDataInterface->GetNumOfItems();
        int32_t iMaxNumberOfButtons = m_pDataInterface->GetMaxNumOfButtons();

        if (m_iNumOfVisibleButtons != -1)
            iMaxNumberOfButtons = m_iNumOfVisibleButtons;

        ZASSERT(iMaxNumberOfButtons <= MAX_NUM_OF_VISIBLE_BUTTONS);

        if (iMaxNumberOfButtons < iNumOfButtons)
            iNumOfButtons = iMaxNumberOfButtons;

        m_iNumOfButtonsUsed = 0;
        const int32_t iVisibleButtons = iNumOfButtons;

        IWindowInterface* pTopWindow = g_pGameData->m_pMenuElements->m_pXMLGUISystem->GetTopWindow();

        int32_t iLineDistance = m_iLineDistance;
        if (iLineDistance == -1)
            iLineDistance = pTopWindow->m_iLineSpace + pResourceManager->m_pFonts[m_eFontType]->GetMaxHeight();

        ZVector2 vArrowSize { 0.0f, 0.0f };
        if (m_paBorderGraphicArrows[0] && !m_bPlaceArrowsButtonRight)
        {
            m_paBorderGraphicArrows[0]->GetButtonSize(&vArrowSize);
            fY += static_cast<float>(iLineDistance) + vArrowSize.y;
        }

        uint32_t iMask = 9;
        if (m_bDisabled)
            iMask = 32;

        int32_t iLine = 0;

        while (m_iNumOfButtonsUsed < iVisibleButtons)
        {
            if (!m_pDataInterface->IgnoreLineInGUI(iLine))
            {
                const int32_t iId = GenerateSubId(m_iNumOfButtonsUsed);

                if (m_bUseTemplate)
                {
                    ZVector2 vTemplateSize;
                    m_ListTemplate.GetSize(vTemplateSize);
                    iLineDistance = static_cast<int32_t>(vTemplateSize.y);

                    ZVector2 vButtonPos;
                    vButtonPos.x = pfPos[0];
                    vButtonPos.y = pfPos[1] + static_cast<float>(iLineDistance / 2 + iLineDistance * iLine);

                    m_paButtons[m_iNumOfButtonsUsed] = pResourceManager->GetButton(vButtonPos, m_pColorSet, pGroup, iId,
                        nullptr, nullptr, m_eAlignment, m_pButtonGraphic, 0, FT_MENU, false, false);

                    m_paButtons[m_iNumOfButtonsUsed]->SetAvailibleStates(iMask);

                    ZASSERT(static_cast<int32_t>(m_ListTemplate.GetGroup().size()) < NUM_OF_COLUMNS);

                    for (uint32_t i = 0; i < m_ListTemplate.GetGroup().size(); ++i)
                    {
                        IGUIElement* pElement = m_ListTemplate.GetGroup()[i];

                        ZVector2 vElementPos;
                        pElement->GetPos(&vElementPos);

                        pResourceManager->AddAdditionalLineObjs(&m_VecLineObjects[m_iNumOfButtonsUsed][i], vElementPos,
                            pElement->m_pColorSet, m_paButtons[m_iNumOfButtonsUsed], pElement->m_eAlignment, iMask,
                            pElement->m_eFontType, pElement->m_bShadow);
                    }
                }
                else
                {
                    const ZVector2 vButtonPos { fX, fY };
                    m_paButtons[m_iNumOfButtonsUsed] = pResourceManager->GetButton(vButtonPos, m_pColorSet, pGroup, iId,
                        &m_VecCheckedWinObjs[m_iNumOfButtonsUsed], &m_VecUncheckedWinObjs[m_iNumOfButtonsUsed],
                        m_eAlignment, m_pButtonGraphic, 9, FT_MENU, true, false);

                    fY += static_cast<float>(iLineDistance);
                }

                if (m_iNumOfButtonsUsed > 0)
                    LinkVertical(m_paButtons[m_iNumOfButtonsUsed - 1], m_paButtons[m_iNumOfButtonsUsed]);

                if (m_bDisabled)
                    m_paButtons[m_iNumOfButtonsUsed]->SetState(32);

                ++m_iNumOfButtonsUsed;
            }

            ++iLine;
        }

        if (!m_bInvalidate)
        {
            m_iIdTop = 0;
            m_iLastFocusId = -1;
        }

        if (!m_bInvalidate && m_paArrowButtons[1] && !m_bPlaceArrowsButtonRight)
        {
            m_paArrowButtons[1]->SetPos(ZVector3(fX, fY, 0.0f));
        }

        if (m_bPlaceArrowsButtonRight && (m_bInvalidate || m_paArrowButtons[1]))
        {
            if (m_paArrowButtons[0] && m_paArrowButtons[1])
            {
                const ZVector3 vArrow0Pos(m_vArrowsPos.x + m_v2DefaultPos.x, m_vArrowsPos.y + m_v2DefaultPos.y, 0.0f);
                m_paArrowButtons[0]->SetPos(vArrow0Pos);

                if (m_bUseDoubleArrowPos)
                {
                    m_paArrowButtons[1]->SetPos(ZVector3(m_vArrowsPos2.x + m_v2DefaultPos.x,
                        m_vArrowsPos2.y + m_v2DefaultPos.y, 0.0f));
                }
                else
                {
                    m_paBorderGraphicArrows[0]->GetButtonSize(&vArrowSize);
                    m_paArrowButtons[1]->SetPos(ZVector3(vArrow0Pos.x, vArrow0Pos.y + vArrowSize.y, 0.0f));
                }
            }
        }

        SetupTextOnButtons();

        float aSize[2] = { 0.0f, 0.0f };

        if (m_pWinBorderGraphic)
        {
            GetSize(m_pWinBorderGraphic, aSize);
            aSize[1] += 10.0f;
        }
        else if (m_pButtonBorder)
        {
            GetSize(m_pButtonBorder, aSize);
            aSize[1] += 10.0f;
        }
        else
        {
            aSize[0] = 0.0f;
            aSize[1] = static_cast<float>(iLineDistance * m_iNumOfButtonsUsed);

            if (m_paArrowButtons[0])
                aSize[1] += static_cast<float>(iLineDistance) + vArrowSize.y;

            if (m_paArrowButtons[1])
                aSize[1] += static_cast<float>(iLineDistance) + vArrowSize.y;
        }

        if (m_bLinkOut)
            return ZGUIElementLink(aSize[0], aSize[1], nullptr, nullptr, m_paButtons[0], m_paButtons[m_iNumOfButtonsUsed - 1]);

        return ZGUIElementLink(aSize[0], aSize[1], m_pButtonBorder, m_pButtonBorder, m_pButtonBorder, m_pButtonBorder);
    }

    void ZGUIList::ReleaseResources(ZResourceManager* pResourceManager)
    {
        for (int32_t i = 0; i < MAX_NUM_OF_VISIBLE_BUTTONS; ++i)
        {
            pResourceManager->ReleaseButton(m_paButtons[i]);
            m_paButtons[i] = nullptr;

            for (int32_t j = 0; j < NUM_OF_COLUMNS; ++j)
                m_VecLineObjects[i][j].clear();
        }

        pResourceManager->ReleaseGraphic(m_pWinBorderGraphic);
        m_pDataInterface->Close();

        if (!m_bInvalidate)
        {
            pResourceManager->ReleaseButton(m_paArrowButtons[0]);
            pResourceManager->ReleaseButton(m_paArrowButtons[1]);
            pResourceManager->ReleaseButton(m_pButtonBorder);

            m_pButtonBorder = nullptr;
            m_paArrowButtons[0] = nullptr;
            m_paArrowButtons[1] = nullptr;
        }
    }

    bool ZGUIList::SetFocus(bool)
    {
        if (m_bDisabled)
            return false;

        if (m_pButtonBorder && !m_bListFocus)
        {
            m_pButtonBorder->GetSystem()->SetFocusedControl(m_pButtonBorder);
            return true;
        }

        const int32_t iFocus = m_pDataInterface->GetFocus();
        const int32_t iLineTop = m_iIdTop;
        m_iLastFocusId = iFocus;

        if (iFocus < iLineTop || iFocus >= iLineTop + m_iNumOfButtonsUsed)
        {
            m_iIdTop = iFocus;

            if (m_iIdTop > m_pDataInterface->GetNumOfItems() - m_iNumOfButtonsUsed)
                m_iIdTop = m_pDataInterface->GetNumOfItems() - m_iNumOfButtonsUsed;

            SetupTextOnButtons();
        }

        const int32_t iButton = m_iLastFocusId - m_iIdTop;
        m_iLastButtonFocus = iButton;

        if (!m_paButtons[iButton])
            return false;

        m_paButtons[iButton]->GetSystem()->SetFocusedControl(m_paButtons[m_iLastButtonFocus]);
        return true;
    }

    void ZGUIList::Cancel()
    {
        m_pDataInterface->Cancel();
    }

    void ZGUIList::Invalidate()
    {
        ZWINDOWS* pWindows = GetMenuZWindows();
        ZWINGROUP* pFocused = pWindows ? pWindows->GetFocusedControl() : nullptr;

        const int32_t iLastFocusId = m_iLastFocusId;
        int32_t iButton = 0;

        while (iButton < m_iNumOfButtonsUsed && pFocused != m_paButtons[iButton])
            ++iButton;

        m_bInvalidate = true;
        ReleaseResources(m_pResourceManager);

        float aPos[2] = { m_v2DefaultPos.x, m_v2DefaultPos.y };
        Setup(aPos, m_pResourceManager, m_pParent);

        m_bInvalidate = false;

        if (m_iNumOfButtonsUsed + m_iIdTop > m_pDataInterface->GetNumOfItems())
        {
            iButton = 0;
            m_iIdTop = 0;
        }

        SetupTextOnButtons();

        if (m_iNumOfButtonsUsed && !m_bDisabled)
        {
            if (iButton >= m_iNumOfButtonsUsed)
            {
                iButton = m_iNumOfButtonsUsed - 1;
                m_iLastFocusId = iLastFocusId;
                m_iLastButtonFocus = m_iNumOfButtonsUsed - 1;
            }

            g_pGameData->m_pMenuElements->m_pXMLGUISystem->SetFocus();

            if (pWindows)
                pWindows->SetFocusedControl(m_paButtons[iButton]);
        }
    }

    void ZGUIList::SetupTextOnButtons()
    {
        if (m_bUseTemplate)
        {
            const uint32_t iNumOfLines = m_ListTemplate.GetGroup().size();

            for (int32_t i = 0; i < m_iNumOfButtonsUsed; ++i)
            {
                ZBUTTON* pButton = m_paButtons[i];

                const char* pColorSetName = m_pDataInterface->GetColorSet(i + m_iIdTop);
                if (pColorSetName)
                {
                    ZColorSet* pColorSet = g_pGameData->m_pMenuElements->GetColorSet(pColorSetName);
                    if (pColorSet)
                        ChangeColorSet(pButton, pColorSet);
                }

                m_pResourceManager->ReleaseButtonGraphic(pButton);

                if (m_pButtonGraphic)
                    m_pResourceManager->AddButtonGraphic(pButton, m_pButtonGraphic, m_pIconColorSet, ELEFT, nullptr, nullptr);

                int32_t iColumn = 0;
                for (const char* pszGraphic = m_pDataInterface->GetButtonGraphicMultiple(i + m_iIdTop, 0);
                     pszGraphic != nullptr;
                     pszGraphic = m_pDataInterface->GetButtonGraphicMultiple(i + m_iIdTop, ++iColumn))
                {
                    ZButtonGraphic* pGraphic = g_pGameData->m_pMenuElements->GetButtonGraphic(pszGraphic);
                    if (pGraphic)
                        m_pResourceManager->AddButtonGraphic(pButton, pGraphic, m_pIconColorSet, ELEFT, nullptr, nullptr);
                }

                for (uint32_t k = 0; k < iNumOfLines; ++k)
                {
                    zstring sText;
                    m_pDataInterface->GetCellText(i + m_iIdTop, k, sText);

                    ZStaticVector<ZLINEOBJ*, 8>& rLineObjs = m_VecLineObjects[i][k];
                    for (uint32_t m = 0; m < rLineObjs.size(); ++m)
                        rLineObjs[m]->SetText(sText.c_str());
                }
            }
        }
        else
        {
            for (int32_t i = 0; i < m_iNumOfButtonsUsed; ++i)
            {
                ZBUTTON* pButton = m_paButtons[i];
                const int32_t iIndex = i + m_iIdTop;

                zstring sText;
                m_pDataInterface->GetCellText(iIndex, 0, sText);
                pButton->SetText(sText.c_str());

                const bool bChecked = m_pDataInterface->GetChecked(iIndex,
                    &m_VecCheckedWinObjs[i], &m_VecUncheckedWinObjs[i]);
                SetChecked(bChecked, m_VecUncheckedWinObjs[i], m_VecCheckedWinObjs[i]);

                ZColorSet* pColorSet = m_pColorSet;
                const char* pColorSetName = m_pDataInterface->GetColorSet(iIndex);
                if (pColorSetName)
                    pColorSet = g_pGameData->m_pMenuElements->GetColorSet(pColorSetName);

                ChangeColorSet(pButton, pColorSet);
            }
        }
    }
}
