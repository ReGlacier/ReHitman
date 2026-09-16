#include <Glacier/GUI/XMLInterface/Elements/ZToggleText.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggleTextItem.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    ZToggleText::ZToggleText()
    {
        m_bSingleResource = false;
        m_bFirstTimeInit = true;
    }

    ZToggleText::~ZToggleText()
    {
    }

    void ZToggleText::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bSingleResource, ppParams, "SingleResource", false);

        if (!m_bSingleResource)
            return;

        char szText[64];
        ZGUI::ReadText(szText, "Text", ppParams);

        zstring* pText = static_cast<zstring*>(ZUniMemory::Allocate(sizeof(zstring)));
        if (pText)
            new (pText) zstring(szText);

        m_vecResourceTextPtr.push_back(pText);
    }

    void ZToggleText::addElement(const char* pName, ZGUIBase* pEntry)
    {
        if (strcmp(pName, "ToggleTextItem") != 0)
        {
            ZGUIBase::addElement(pName, pEntry);
            return;
        }

        zstring* pText = static_cast<zstring*>(ZUniMemory::Allocate(sizeof(zstring)));
        if (pText)
            new (pText) zstring(static_cast<ZToggleTextItem*>(pEntry)->GetText());

        m_vecResourceTextPtr.push_back(pText);
    }

    void ZToggleText::AddText(const char* pText)
    {
        zstring* pStr = static_cast<zstring*>(ZUniMemory::Allocate(sizeof(zstring)));
        if (pStr)
            new (pStr) zstring(pText);

        m_vecTextPtr.push_back(pStr);

        if (m_vecTextPtr.size() != 0)
            SetUpperBound(static_cast<int32_t>(m_vecTextPtr.size()) - 1);
    }

    void ZToggleText::AddText(const char* pText, uint32_t dwLen)
    {
        ZASSERT(dwLen < 0xFF);

        char szText[256];
        strncpy(szText, pText, dwLen);
        szText[dwLen] = 0;

        AddText(szText);
    }

    void ZToggleText::OptionChanged()
    {
        if (m_bFirstTimeInit)
        {
            m_bFirstTimeInit = false;
            ParseResourceText();
        }

        if (m_vecTextPtr.size() == 0)
            return;

        if (m_pZButton)
            m_pZButton->SetText(m_vecTextPtr[GetOptionValue()]->c_str());
    }

    void ZToggleText::ParseResourceText()
    {
        for (uint32_t i = 0; i < m_vecResourceTextPtr.size(); ++i)
        {
            const char* pszText = g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_vecResourceTextPtr[i]->c_str());

            if (!m_bSingleResource)
            {
                AddText(pszText);
                continue;
            }

            uint32_t iLastNewLine = 0;
            uint32_t iLineStart = 0;
            uint32_t iChar = 0;

            while (pszText[iChar])
            {
                if (pszText[iChar] == '\r' || pszText[iChar] == '\n')
                {
                    iLastNewLine = iChar;
                    AddText(pszText + iLineStart, iChar - iLineStart);
                    iLineStart = iChar + 1;
                }

                ++iChar;
            }

            if (iLastNewLine != iChar - 1)
                AddText(pszText + iLineStart, iChar - iLineStart);
        }
    }
}
