#include <Glacier/GUI/XMLInterface/Windows/ZStandardWindow.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIGroup.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/Data/ZGameData.h>
#include <cstring>


namespace Glacier
{
    ZStandardWindow::ZStandardWindow(ZMenuElements* pMenuElements)
        : IWindowInterface(pMenuElements)
    {
        m_iAlignmentId = 0;
        m_bDialogWindow = false;
        m_bTRCWindow = false;
        m_iTRCPriority = 0;
    }

    void ZStandardWindow::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IWindowInterface::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bDialogWindow, ppParams, "Dialog", false);
        GuiOption::readBool(&m_bTRCWindow, ppParams, "TRC", false);
        GuiOption::readInt(m_iTRCPriority, ppParams, "TRCPrioritry");
    }

    void ZStandardWindow::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "GuiElement") == 0)
        {
            IWindowInterface::ZElementExtraInfo extraInfo;
            extraInfo.m_pGUIElement = static_cast<IGUIElement*>(pEntry);
            extraInfo.m_iAlignmentOverride = (m_iAlignmentId << 16) | static_cast<int32_t>(m_eNavigation);
            m_vecGUIElementExtraInfo.push_back(extraInfo);
        }
        else if (strcmp(pName, "Navigation") == 0)
        {
            ++m_iAlignmentId;

            ZGUIGroup* pGroup = static_cast<ZGUIGroup*>(pEntry);
            const ENavigation eNavigation = pGroup->GetNavigation();
            const ZStaticVector<IGUIElement*, 16>& rGroup = pGroup->GetGroup();
            for (uint32_t i = 0; i < rGroup.size(); ++i)
            {
                IWindowInterface::ZElementExtraInfo extraInfo;
                extraInfo.m_pGUIElement = rGroup[i];
                extraInfo.m_iAlignmentOverride = (m_iAlignmentId << 16) | static_cast<int32_t>(eNavigation);
                m_vecGUIElementExtraInfo.push_back(extraInfo);
            }
        }
    }

    void ZStandardWindow::OpenWindow(ZResourceManager* pResourceManager, bool bReset, ZWINGROUP* pWinGroup, bool bFocus)
    {
        IWindowInterface::OpenWindow(pResourceManager, bReset, pWinGroup, bFocus);

        m_vecCurrentElements.clear();

        float aGroupPos[2] = { 0.0f, 0.0f };
        ZGUIElementLink prevElementLink;

        for (uint32_t i = 0; i < m_vecGUIElementExtraInfo.size(); )
        {
            const int32_t iNavigationId = m_vecGUIElementExtraInfo[i].GetNavigationId();
            const ENavigation eNavigation = m_vecGUIElementExtraInfo[i].GetNavigation();

            ZStaticVector<IGUIElement*, 44> aGroupElements;

            do
            {
                IGUIElement* pElement = m_vecGUIElementExtraInfo[i].m_pGUIElement;
                if (pElement->Use())
                {
                    aGroupElements.push_back(pElement);
                    m_vecCurrentElements.push_back(pElement);
                }

                ++i;
            }
            while (i < m_vecGUIElementExtraInfo.size() &&
                   m_vecGUIElementExtraInfo[i].GetNavigationId() == iNavigationId);

            prevElementLink = SetupGUIElements(aGroupPos, eNavigation, aGroupElements, pResourceManager, pWinGroup, &prevElementLink);
        }

        if (bFocus)
        {
            g_pGameData->m_pMenuElements->m_pXMLGUISystem->SetFocus();
            SetFirstFocus(m_vecCurrentElements);
        }
    }

    void ZStandardWindow::CloseWindow(ZResourceManager* pResourceManager, bool bReset)
    {
        IWindowInterface::CloseWindow(pResourceManager, bReset);

        for (uint32_t i = 0; i < m_vecCurrentElements.size(); ++i)
            m_vecCurrentElements[i]->ReleaseResources(pResourceManager);

        m_vecCurrentElements.clear();
    }

    bool ZStandardWindow::Update(int iIndex)
    {
        bool bFound = false;

        for (uint32_t i = 0; i < m_vecCurrentElements.size(); ++i)
        {
            IGUIElement* pElement = m_vecCurrentElements[i];
            pElement->Update(false);

            if (pElement->m_iIndex == iIndex)
                bFound = true;
        }

        return bFound;
    }

    const ZStaticVector<IGUIElement*, 44>* ZStandardWindow::GetCurrentElements()
    {
        return &m_vecCurrentElements;
    }

    void ZStandardWindow::Cancel()
    {
        CallCancel(m_vecCurrentElements);
    }

    void ZStandardWindow::Invalidate()
    {
        for (uint32_t i = 0; i < m_vecCurrentElements.size(); ++i)
            m_vecCurrentElements[i]->Invalidate();
    }

    void ZStandardWindow::GrapFocus()
    {
        SetFirstFocus(m_vecCurrentElements);
    }

    bool ZStandardWindow::IsDialog()
    {
        return m_bDialogWindow;
    }

    bool ZStandardWindow::IsTRC()
    {
        return m_bTRCWindow;
    }

    int ZStandardWindow::GetTRCPriority()
    {
        return m_iTRCPriority;
    }
}
