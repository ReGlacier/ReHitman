#include <Glacier/GUI/XMLInterface/Elements/ZGUIEdit.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>

#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t kAllowedChars[] =
        {
            0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
            0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
            0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
            0xe0, 0xe1, 0xe2, 0xe4, 0xe5, 0xe3, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed,
            0xee, 0xef, 0xf1, 0xf2, 0xf3, 0xf4, 0xf6, 0xf5, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xdf,
            0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e,
            0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,
            0xc0, 0xc1, 0xc2, 0xc4, 0xc5, 0xc3, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd,
            0xce, 0xcf, 0xd1, 0xd2, 0xd3, 0xd4, 0xd6, 0xd5, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdf,
            0x5f, 0x2d, 0x5b, 0x5d, 0x24, 0x3d, 0x23, 0x26, 0x28, 0x29, 0x21, 0x20, 0xbf, 0xa1
        };

        bool IsAllowedChar(uint8_t uChar)
        {
            for (uint32_t i = 0; i < sizeof(kAllowedChars) / sizeof(kAllowedChars[0]); ++i)
            {
                if (kAllowedChars[i] == uChar)
                    return true;
            }

            return false;
        }

        bool IsPasswordChar(uint8_t uChar)
        {
            return (uChar >= '0' && uChar <= '9')
                || (uChar >= 'A' && uChar <= 'Z')
                || (uChar >= 'a' && uChar <= 'z')
                || uChar == '-' || uChar == '_' || uChar == '&' || uChar == '@' || uChar == '#'
                || uChar == '$' || uChar == '!' || uChar == '?' || uChar == ':' || uChar == '.'
                || uChar == '~' || uChar == '*' || uChar == '=' || uChar == '[' || uChar == ']'
                || uChar == ',' || uChar == '\\' || uChar == ';' || uChar == '\'' || uChar == '"'
                || uChar == '%' || uChar == '/' || uChar == '(' || uChar == ')' || uChar == '+';
        }
    }

    ZGUIEdit::ZGUIEdit()
    {
        m_fCursorTime = 0.0f;
        m_bCursor = false;
        m_pWinGroup_Cursor = nullptr;
        m_pButton_Text = nullptr;
        m_sText[0] = 0;
        m_dwLen = 0;
        m_szCursor[0] = 0;
        m_bStateActive = false;
        m_dwTextLimit = 32;
        m_iTextPixelLimit = 0;
        m_bPassword = false;
        m_bDisableAnimateAlpha = true;
        m_bUseCharList = false;
    }

    ZGUIEdit::~ZGUIEdit()
    {
    }

    void ZGUIEdit::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        IGUIElement::readParams(ppParams, pElems);

        ZGUI::ReadText(m_szCursor, "Cursor", ppParams);
        GuiOption::readInt(m_dwTextLimit, ppParams, "TextLimit");
        GuiOption::readInt(m_iTextPixelLimit, ppParams, "PixelLimit");

        zstring sPassword;
        GuiOption::readString(sPassword, "Password", ppParams);
        if (sPassword == "True")
            m_bPassword = true;

        GuiOption::readString(m_szInfoText, "InfoText", ppParams);
        GuiOption::readBool(&m_bDisableAnimateAlpha, ppParams, "DisableAnimateAlpha", false);
        GuiOption::readBool(&m_bUseCharList, ppParams, "UseCharList", false);
    }

    ZGUIElementLink ZGUIEdit::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        const ZVector2 vPos { pfPos[0], pfPos[1] };

        m_pButton_Text = pResourceManager->GetButton(vPos, m_pColorSet, pGroup, m_iIndex, nullptr, nullptr,
            m_eAlignment, m_pButtonGraphic, 9, m_eFontType, m_bShadow, m_bDisableAnimateAlpha);

        float fCursorX = pfPos[0];
        float fCursorY = pfPos[1];

        if (!m_szInfoText.empty())
        {
            const char* pszInfoText =
                g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_szInfoText.c_str());
            m_pButton_Text->SetText(pszInfoText);

            Vector3 vTextRight;
            GetRightPosOfTextGroup(m_pButton_Text, &vTextRight, 5);

            const ZVector2 vOffset { vTextRight.x - pfPos[0], vTextRight.y - pfPos[1] };
            pResourceManager->AddAdditionalLineObjs(&m_LineObjs, vOffset, m_pColorSet, m_pButton_Text,
                m_eAlignment, 9, m_eFontType, true);

            for (uint32_t i = 0; i < m_LineObjs.size(); ++i)
                m_LineObjs[i]->m_bAnimateAlpha = !m_bDisableAnimateAlpha;

            fCursorX = vTextRight.x;
            fCursorY = vTextRight.y;
        }

        m_pWinGroup_Cursor = pResourceManager->GetGraphic(vPos, m_pColorSet, pGroup, m_szCursor, ELEFT, -1);
        m_pWinGroup_Cursor->SetPos(Vector3(fCursorX, fCursorY, 0.0f));
        m_pWinGroup_Cursor->Hide(false);

        UpdateText();

        float fSizeX = 100.0f;
        float fSizeY = 20.0f;
        if (m_pButtonGraphic)
        {
            Vector2 vButtonSize;
            m_pButtonGraphic->GetButtonSize(&vButtonSize);
            fSizeX = vButtonSize.x;
            fSizeY = vButtonSize.y;
        }

        return ZGUIElementLink(fSizeX, fSizeY, m_pButton_Text, m_pButton_Text, m_pButton_Text, m_pButton_Text);
    }

    void ZGUIEdit::ReleaseResources(ZResourceManager* pResourceManager)
    {
        pResourceManager->ReleaseGraphic(m_pWinGroup_Cursor);
        pResourceManager->ReleaseButton(m_pButton_Text);
        m_LineObjs.clear();
    }

    void ZGUIEdit::Click(eZWUserEvents eEvent, int, ZXMLGUISystem*)
    {
        if (static_cast<int32_t>(eEvent) == 12 && m_dwLen != 0)
        {
            --m_dwLen;
            m_sText[m_dwLen] = 0;
            UpdateText();
            return;
        }

        if (m_dwLen < static_cast<uint32_t>(m_dwTextLimit))
        {
            const uint8_t uChar = static_cast<uint8_t>(eEvent);

            if (((uChar > 0x1F) && (uChar < 0x7B)) || m_bUseCharList)
            {
                if (m_bUseCharList && !IsAllowedChar(uChar))
                    return;

                if ((m_dwLen != 0 || uChar != ' ') && (!m_bPassword || IsPasswordChar(uChar)))
                {
                    m_sText[m_dwLen] = static_cast<char>(uChar);
                    ++m_dwLen;
                    m_sText[m_dwLen] = 0;
                }
            }
        }

        UpdateText();
    }

    void ZGUIEdit::Update(bool)
    {
        IGUIElement* pElementInFocus = g_pGameData->m_pMenuElements->m_pXMLGUISystem->GetElementInFocus();

        if (pElementInFocus == this)
        {
            m_fCursorTime -= g_pSysInterface->m_fActualTimeDelta;

            if (m_fCursorTime <= 0.0f)
            {
                m_bCursor = !m_bCursor;
                m_pWinGroup_Cursor->Hide(m_bCursor);

                if (m_bCursor)
                {
                    m_bStateActive = true;
                    m_fCursorTime = 0.1f;
                    return;
                }

                m_fCursorTime = 0.3f;
            }

            m_bStateActive = true;
        }
        else
        {
            m_pWinGroup_Cursor->Hide(true);

            if (m_bStateActive)
                Done();

            m_bStateActive = false;
        }
    }

    bool ZGUIEdit::SetFocus(bool)
    {
        ZWINDOWS* pWindows = m_pButton_Text->GetSystem();
        pWindows->SetFocusedControl(m_pButton_Text);
        return true;
    }

    void ZGUIEdit::Done()
    {
        ZASSERT(false);
    }

    void ZGUIEdit::SetText(zstring strText)
    {
        strncpy(m_sText, strText.c_str(), sizeof(m_sText));
        m_sText[sizeof(m_sText) - 1] = 0;
        m_dwLen = static_cast<uint32_t>(strlen(m_sText));
        UpdateText();
    }

    zstring ZGUIEdit::GetText() const
    {
        return zstring(m_sText);
    }

    void ZGUIEdit::UpdateText()
    {
        char szDisplay[256];
        szDisplay[0] = 0;

        if (m_sText[0] != 0)
        {
            if (m_iTextPixelLimit != 0)
            {
                const zstring sText(m_sText);
                const uint32_t iLen = sText.length();

                char szReversed[512];
                for (uint32_t i = 0; i < iLen; ++i)
                    szReversed[i] = m_sText[iLen - 1 - i];

                uint32_t iVisible = 0;
                if (m_iTextPixelLimit > 0)
                {
                    do
                    {
                        szReversed[iVisible + 256] = szReversed[iVisible];
                        szReversed[iVisible + 257] = 0;

                        ZLINEOBJ* pLineObj;
                        if (m_LineObjs.size())
                        {
                            pLineObj = m_LineObjs[0];
                        }
                        else
                        {
                            char szUtf8[256];
                            zstring::CStrToUTF8(szUtf8, m_sText, static_cast<int>(sizeof(szUtf8)));
                            m_pButton_Text->SetText(szUtf8);

                            pLineObj = nullptr;
                            for (ZBaseGeom* pBaseGeom = m_pButton_Text->m_pGroupFirst;
                                 pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
                            {
                                ZGEOM* pGeom = pBaseGeom->GetGeom();
                                if (pGeom && pGeom->IsDerivedFrom<ZLINEOBJ>())
                                {
                                    pLineObj = static_cast<ZLINEOBJ*>(pGeom);
                                    break;
                                }
                            }
                        }

                        ZASSERT(pLineObj && pLineObj->IsDerivedFrom<ZLINEOBJ>());

                        const int32_t iPixelPos = pLineObj->GetCharPosition(static_cast<int>(++iVisible));

                        if (iLen == iVisible || iPixelPos >= m_iTextPixelLimit)
                            break;
                    }
                    while (true);
                }

                uint32_t iOut = 0;
                for (uint32_t i = iLen - iVisible; i < iLen; ++i)
                    szDisplay[iOut++] = m_sText[i];

                szDisplay[iOut] = 0;
            }
            else
            {
                strcpy(szDisplay, m_sText);
            }
        }

        if (m_bPassword && szDisplay[0])
        {
            for (char* p = szDisplay; *p; ++p)
                *p = '*';
        }

        char szText[256];
        zstring::CStrToUTF8(szText, szDisplay, static_cast<int>(sizeof(szText)));

        if (m_LineObjs.size())
        {
            for (uint32_t i = 0; i < m_LineObjs.size(); ++i)
                m_LineObjs[i]->SetText(szText);
        }
        else
        {
            m_pButton_Text->SetText(szText);
        }

        Vector3 vTextRight;
        GetRightPosOfTextGroup(m_pButton_Text, &vTextRight, 0);

        if (m_pWinGroup_Cursor)
            m_pWinGroup_Cursor->SetPos(Vector3(vTextRight.x - 11.0f, vTextRight.y - 20.0f, 0.0f));
    }
}
