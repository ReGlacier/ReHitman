#include <Glacier/GUI/XMLInterface/Elements/ZGUITable.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIDataInterfaces.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/Font/ZTTFONT.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>


namespace Glacier
{
    ZGUITable::ZGUITable()
    {
        m_pTableDataInterface = nullptr;

        for (int32_t i = 0; i < 7; ++i)
            m_paButtons[i] = nullptr;

        for (int32_t i = 0; i < MAX_NUM_OF_CELLS; ++i)
            m_paCells[i] = nullptr;

        m_iNumOfRows = 0;
        m_iNumOfColumns = 0;
        m_iNumOfRowsDisplayed = 0;
        m_iTopRow = 0;

        for (int32_t i = 0; i < MAX_NUM_OF_COLUMNS; ++i)
            m_iColumnSize[i] = 80;

        m_iLineDistance = -1;
    }

    void ZGUITable::Click(eZWUserEvents eEvent, int iIndex, ZXMLGUISystem*)
    {
        if (static_cast<int32_t>(eEvent) != eZW_SELECT)
            return;

        if (iIndex >= m_iNumOfColumns)
        {
            if (iIndex == m_iNumOfColumns + 1 && m_iTopRow < m_iNumOfRows - m_iNumOfRowsDisplayed)
            {
                ++m_iTopRow;
                SetTextOnTable();
            }
            else if (iIndex == m_iNumOfColumns && m_iTopRow > 0)
            {
                --m_iTopRow;
                SetTextOnTable();
            }
        }
        else
        {
            m_pTableDataInterface->SelectRow(iIndex + m_iTopRow);
            SetTextOnTable();
        }
    }

    ZGUIElementLink ZGUITable::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        m_pTableDataInterface = g_pGameData->m_pMenuElements->GetTableDataInterface(GetName());
        m_pTableDataInterface->Setup();
        m_pTableDataInterface->GetSize(m_iNumOfRows, m_iNumOfColumns);

        ZASSERT(m_iNumOfColumns <= MAX_NUM_OF_COLUMNS);

        m_iNumOfRowsDisplayed = m_iNumOfRows < 6 ? m_iNumOfRows : 5;
        m_iTopRow = 0;

        int32_t iLineDistance = m_iLineDistance;
        if (iLineDistance == -1)
        {
            IWindowInterface* pTopWindow = g_pGameData->m_pMenuElements->m_pXMLGUISystem->GetTopWindow();
            iLineDistance = pTopWindow->m_iLineSpace + pResourceManager->m_pFonts[m_eFontType]->GetMaxHeight();
        }

        const float fLineDistance = static_cast<float>(iLineDistance);
        float fX = pfPos[0];
        float fY = pfPos[1];

        for (int32_t i = 0; i < m_iNumOfColumns; ++i)
        {
            const ZVector2 vButtonPos { fX, fY };
            m_paButtons[i] = pResourceManager->GetButton(vButtonPos, m_pColorSet, pGroup, GenerateSubId(i),
                nullptr, nullptr, ELEFT, nullptr, 9, FT_MENU, true, false);

            fX += static_cast<float>(m_iColumnSize[i]);

            if (i > 0)
            {
                m_paButtons[i]->SetNextFocus(m_paButtons[i - 1], Left);
                m_paButtons[i - 1]->SetNextFocus(m_paButtons[i], Right);
            }
        }

        m_paButtons[0]->SetNextFocus(m_paButtons[m_iNumOfColumns - 1], Left);
        m_paButtons[m_iNumOfColumns - 1]->SetNextFocus(m_paButtons[0], Right);

        fY += fLineDistance;

        for (int32_t i = 0; i < m_iNumOfRowsDisplayed; ++i)
        {
            fX = pfPos[0];

            for (int32_t j = 0; j < m_iNumOfColumns; ++j)
            {
                const ZVector2 vCellPos { fX, fY };
                m_paCells[j + i * m_iNumOfColumns] = pResourceManager->GetTextGroup(vCellPos, m_pColorSet, pGroup,
                    1, FT_MENU, true, ELEFT);

                fX += static_cast<float>(m_iColumnSize[j]);
            }

            fY += fLineDistance;
        }

        fX = pfPos[0];
        const ZVector2 vScrollUpPos { fX, fY };
        m_paButtons[m_iNumOfColumns] = pResourceManager->GetButton(vScrollUpPos, m_pColorSet, pGroup,
            GenerateSubId(m_iNumOfColumns), nullptr, nullptr, ELEFT, nullptr, 9, FT_MENU, true, false);

        fX += static_cast<float>(m_iColumnSize[0]);
        const ZVector2 vScrollDownPos { fX, fY };
        m_paButtons[m_iNumOfColumns + 1] = pResourceManager->GetButton(vScrollDownPos, m_pColorSet, pGroup,
            GenerateSubId(m_iNumOfColumns + 1), nullptr, nullptr, ELEFT, nullptr, 9, FT_MENU, true, false);

        m_paButtons[m_iNumOfColumns]->SetNextFocus(m_paButtons[m_iNumOfColumns + 1], Left);
        m_paButtons[m_iNumOfColumns]->SetNextFocus(m_paButtons[m_iNumOfColumns + 1], Right);
        m_paButtons[m_iNumOfColumns + 1]->SetNextFocus(m_paButtons[m_iNumOfColumns], Left);
        m_paButtons[m_iNumOfColumns + 1]->SetNextFocus(m_paButtons[m_iNumOfColumns], Right);

        m_paButtons[m_iNumOfColumns]->SetNextFocus(m_paButtons[0], Up);
        m_paButtons[m_iNumOfColumns + 1]->SetNextFocus(m_paButtons[0], Up);

        for (int32_t i = 0; i < m_iNumOfColumns; ++i)
            m_paButtons[i]->SetNextFocus(m_paButtons[m_iNumOfColumns], Down);

        fY += fLineDistance;

        float fSizeX = 0.0f;
        for (int32_t i = 0; i < m_iNumOfColumns; ++i)
            fSizeX += static_cast<float>(m_iColumnSize[i]);

        SetTextOnTable();

        ZGUIElementLink result;
        result.m_fSizeX = fSizeX;
        result.m_fSizeY = fY - pfPos[1];
        result.m_apFocusIn[0] = nullptr;
        result.m_apFocusIn[1] = nullptr;
        result.m_apFocusIn[2] = m_paButtons[0];
        result.m_apFocusIn[3] = m_paButtons[m_iNumOfColumns];
        result.m_bUseLinks = result.m_apFocusIn[2] != nullptr || result.m_apFocusIn[3] != nullptr;
        return result;
    }

    void ZGUITable::ReleaseResources(ZResourceManager* pResourceManager)
    {
        for (int32_t i = 0; i < 7; ++i)
        {
            pResourceManager->ReleaseButton(m_paButtons[i]);
            m_paButtons[i] = nullptr;
        }

        for (int32_t i = 0; i < MAX_NUM_OF_CELLS; ++i)
        {
            pResourceManager->ReleaseTextGroup(m_paCells[i]);
            m_paCells[i] = nullptr;
        }
    }

    bool ZGUITable::SetFocus(bool)
    {
        ZWINDOWS* pWindows = m_paButtons[0]->GetSystem();
        pWindows->SetFocusedControl(m_paButtons[0]);
        return true;
    }

    void ZGUITable::SetTextOnTable()
    {
        for (int32_t i = 0; i < m_iNumOfColumns; ++i)
        {
            const zstring sTitle = m_pTableDataInterface->GetColumnTitle(i);
            m_paButtons[i]->SetText(sTitle.c_str());
        }

        m_pTableDataInterface->Begin();

        for (int32_t i = 0; i < m_iNumOfRowsDisplayed; ++i)
        {
            for (int32_t j = 0; j < m_iNumOfColumns; ++j)
            {
                const zstring sText = m_pTableDataInterface->GetCellText(i + m_iTopRow, j);
                m_paCells[j + i * m_iNumOfColumns]->SetText(sText.c_str());
            }
        }

        m_pTableDataInterface->End();

        const char* pszUp = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", "Up");
        const char* pszDown = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", "Down");

        m_paButtons[m_iNumOfColumns]->SetText(pszUp);
        m_paButtons[m_iNumOfColumns + 1]->SetText(pszDown);
    }
}
