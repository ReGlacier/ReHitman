#include <Glacier/GUI/XMLInterface/Elements/ZGUIVirtualKeyboard.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ResourceCollection.h>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t kNormalKeys[80] =
        {
            '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
            'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
            'u', 'v', 'w', 'x', 'y', 'z', '@', '.', '_', '-',
            '[', ']', '$', '=', '?', '!', '#', '&', '*', '~',
            ':', ',', 0xE0, 0xE1, 0xE2, 0xE4, 0xE5, 0xE3, 0xE6, 0xE7,
            0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF1, 0xF2,
            0xF3, 0xF4, 0xF6, 0xF5, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xDF
        };

        constexpr uint32_t kCapsKeys[80] =
        {
            '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X', 'Y', 'Z', '@', '.', '_', '-',
            '[', ']', '$', '=', '?', '!', '#', '&', '*', '~',
            ':', ',', 0xC0, 0xC1, 0xC2, 0xC4, 0xC5, 0xC3, 0xC6, 0xC7,
            0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD1, 0xD2,
            0xD3, 0xD4, 0xD6, 0xD5, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDF
        };

        int32_t Utf32ToUtf8(char* pDest, uint32_t uChar)
        {
            if (uChar > 0x7F)
            {
                if (uChar > 0x7FF)
                {
                    if (uChar > 0xFFFF)
                    {
                        pDest[0] = static_cast<char>((uChar >> 18) | 0xF0);
                        pDest[1] = static_cast<char>(((uChar >> 12) & 0x3F) | 0x80);
                        pDest[3] = static_cast<char>((uChar & 0x3F) | 0x80);
                        pDest[2] = static_cast<char>(((uChar >> 6) & 0x3F) | 0x80);
                        pDest[4] = 0;
                        return 4;
                    }

                    pDest[0] = static_cast<char>((uChar >> 12) | 0xE0);
                    pDest[2] = static_cast<char>((uChar & 0x3F) | 0x80);
                    pDest[1] = static_cast<char>(((uChar >> 6) & 0x3F) | 0x80);
                    pDest[3] = 0;
                    return 3;
                }

                pDest[1] = static_cast<char>((uChar & 0x3F) | 0x80);
                pDest[0] = static_cast<char>((uChar >> 6) | 0xC0);
                pDest[2] = 0;
                return 2;
            }

            *reinterpret_cast<uint16_t*>(pDest) = static_cast<uint8_t>(uChar);
            return 1;
        }

        uint32_t Utf8ToUtf32(const char** ppText)
        {
            const uint8_t uFirst = static_cast<uint8_t>(**ppText);

            if (uFirst < 0x80)
            {
                ++(*ppText);
                return uFirst;
            }

            uint32_t uResult = 0;
            uint32_t uRemaining = 0;

            if ((uFirst & 0xE0) == 0xC0)
            {
                uResult = uFirst & 0x1F;
                uRemaining = 1;
            }
            else if ((uFirst & 0xF0) == 0xE0)
            {
                uResult = uFirst & 0x0F;
                uRemaining = 2;
            }
            else if ((uFirst & 0xF8) == 0xF0)
            {
                uResult = uFirst & 0x07;
                uRemaining = 3;
            }
            else
            {
                ++(*ppText);
                return uFirst;
            }

            ++(*ppText);

            for (uint32_t i = 0; i < uRemaining; ++i)
            {
                uResult = (uResult << 6) | (static_cast<uint8_t>(**ppText) & 0x3F);
                ++(*ppText);
            }

            return uResult;
        }

        void BuildUtf8(const UTC4CHAR* pText, char* pDest)
        {
            int32_t iOffset = 0;

            for (const UTC4CHAR* pChar = pText; *pChar != 0; ++pChar)
                iOffset += Utf32ToUtf8(&pDest[iOffset], *pChar);

            pDest[iOffset] = 0;
        }

        ZLINEOBJ* FindLineObj(ZWINGROUP* pGroup)
        {
            for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
            {
                ZGEOM* pGeom = pBaseGeom->GetGeom();
                if (pGeom && pGeom->IsDerivedFrom<ZLINEOBJ>())
                    return static_cast<ZLINEOBJ*>(pGeom);
            }

            return nullptr;
        }
    }

    ZGUIVirtualKeyboard::ZGUIVirtualKeyboard()
    {
        m_pParent = nullptr;
        m_pWinGroup_BackgroundPassword = nullptr;
        m_pWinGroup_BackgroundNormal = nullptr;
        m_pWinGroup_BackgroundCaps = nullptr;
        m_pWinGroup_ButtonSmall = nullptr;
        m_pWinGroup_ButtonBig = nullptr;
        m_pWinGroup_ButtonBigHighLightCaps = nullptr;
        m_pWinGroup_ButtonBigHighLight = nullptr;
        m_pWinGroup_ButtonSmallHighLight = nullptr;
        m_pWinGroup_Cursor = nullptr;
        m_pWinGroup_Text = nullptr;
        m_pWinGroup_Title = nullptr;

        m_dwPosSmallX = 0;
        m_dwPosSmallY = 0;
        m_dwPosBigX = 0;
        m_dwPosBigY = 0;
        m_eKeyboard = KEYBOARD_NORMAL;
        m_eMode = MODE_ALPHABET;
        m_bCaps = false;
        m_fSmallButtonHighLight = 0.0f;
        m_fBigButtonHighLight = 0.0f;
        m_utc4Text[0] = 0;
        m_dwTextLen = 0;
        m_bCursor = true;
        m_fCursorTime = 0.0f;
        m_dwTextLimit = 16;
        m_iTextPixelLimit = 0;
    }

    ZGUIVirtualKeyboard::~ZGUIVirtualKeyboard()
    {
    }

    void ZGUIVirtualKeyboard::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        m_dwTextLimit = 16;
        m_iTextPixelLimit = 0;

        IGUIElement::readParams(ppParams, pElems);

        GuiOption::readString(m_sTitle, "Title", ppParams);
        GuiOption::readInt(m_dwTextLimit, ppParams, "TextLimit");
        GuiOption::readInt(m_iTextPixelLimit, ppParams, "PixelLimit");
    }

    ZGUIElementLink ZGUIVirtualKeyboard::Setup(float* pfPos, ZResourceManager* pResourceManager, ZWINGROUP* pGroup)
    {
        m_pWinGroup_Text = nullptr;
        m_bCursor = true;
        m_fCursorTime = 0.0f;
        m_fBigButtonHighLight = 0.0f;
        m_fSmallButtonHighLight = 0.0f;
        m_dwTextLen = 0;
        m_bCaps = false;
        m_eKeyboard = KEYBOARD_NORMAL;
        m_eMode = MODE_ALPHABET;
        m_dwPosSmallX = 0;
        m_dwPosSmallY = 0;
        m_dwPosBigX = 0;
        m_dwPosBigY = 0;
        m_pParent = nullptr;

        m_pParent = pResourceManager->GetWingroup(pGroup);
        m_pParent->SetPos(Vector3(pfPos[0], pfPos[1], 0.0f));

        const ZVector2 vZero { 0.0f, 0.0f };
        const ZVector2 vSmall { 70.0f, 42.0f };
        const ZVector2 vBig { 14.0f, 40.0f };
        const ZVector2 vText { 28.0f, 35.0f };
        const ZVector2 vTitle { 80.0f, 88.0f };

        m_pWinGroup_BackgroundPassword = pResourceManager->GetGraphic(vZero, m_pColorSet, m_pParent,
            "OnlineBackgroundKeyboardPassword", ELEFT, -1);
        m_pWinGroup_BackgroundNormal = pResourceManager->GetGraphic(vZero, m_pColorSet, m_pParent,
            "OnlineBackgroundKeyboardNormal", ELEFT, -1);
        m_pWinGroup_BackgroundCaps = pResourceManager->GetGraphic(vZero, m_pColorSet, m_pParent,
            "OnlineBackgroundKeyboardCaps", ELEFT, -1);
        m_pWinGroup_ButtonSmall = pResourceManager->GetGraphic(vSmall, m_pColorSet, m_pParent,
            "OnlineKeyboardButtonSmall", ELEFT, -1);
        m_pWinGroup_ButtonBig = pResourceManager->GetGraphic(vBig, m_pColorSet, m_pParent,
            "OnlineKeyboardButtonBig", ELEFT, -1);
        m_pWinGroup_ButtonBigHighLightCaps = pResourceManager->GetGraphic(vBig, m_pColorSet, m_pParent,
            "OnlineKeyboardButtonBigHighLightCaps", ELEFT, -1);
        m_pWinGroup_ButtonBigHighLight = pResourceManager->GetGraphic(vBig, m_pColorSet, m_pParent,
            "OnlineKeyboardButtonBigHighLight", ELEFT, -1);
        m_pWinGroup_ButtonSmallHighLight = pResourceManager->GetGraphic(vBig, m_pColorSet, m_pParent,
            "OnlineKeyboardButtonSmallHighLight", ELEFT, -1);
        m_pWinGroup_Cursor = pResourceManager->GetGraphic(vBig, m_pColorSet, m_pParent,
            "OnlineKeyboardCursor00", ELEFT, -1);

        m_pWinGroup_Text = pResourceManager->GetTextGroup(vText, m_pColorSet, m_pParent, 1,
            m_eFontType, m_bShadow, m_eAlignment);
        m_pWinGroup_Title = pResourceManager->GetTextGroup(vTitle, m_pColorSet, m_pParent, 1,
            m_eFontType, m_bShadow, m_eAlignment);

        m_pWinGroup_Title->SetText(
            g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface", m_sTitle.c_str()));

        m_pWinGroup_ButtonSmall->Hide(false);
        m_pWinGroup_ButtonBig->Hide(true);
        m_pWinGroup_BackgroundCaps->Hide(true);
        m_pWinGroup_ButtonBigHighLightCaps->Hide(true);
        m_pWinGroup_ButtonBigHighLight->Hide(true);
        m_pWinGroup_BackgroundPassword->Hide(true);
        m_pWinGroup_ButtonSmallHighLight->Hide(true);

        g_pGameData->m_pMenuElements->m_pXMLGUISystem->AddRecieveAllInput(this);

        if (m_eKeyboard == KEYBOARD_PASSWORD)
        {
            m_pWinGroup_BackgroundNormal->Hide(true);
            m_pWinGroup_BackgroundPassword->Hide(false);
        }
        else
        {
            m_pWinGroup_BackgroundNormal->Hide(false);
            m_pWinGroup_BackgroundPassword->Hide(true);
        }

        m_pWinGroup_Cursor->SetPos(Vector3(16.0f, 16.0f, 0.0f));

        UpdateText();

        return ZGUIElementLink();
    }

    void ZGUIVirtualKeyboard::ReleaseResources(ZResourceManager* pResourceManager)
    {
        pResourceManager->ReleaseGraphic(m_pWinGroup_BackgroundPassword);
        pResourceManager->ReleaseGraphic(m_pWinGroup_BackgroundNormal);
        pResourceManager->ReleaseGraphic(m_pWinGroup_BackgroundCaps);
        pResourceManager->ReleaseGraphic(m_pWinGroup_ButtonSmall);
        pResourceManager->ReleaseGraphic(m_pWinGroup_ButtonBig);
        pResourceManager->ReleaseGraphic(m_pWinGroup_ButtonBigHighLightCaps);
        pResourceManager->ReleaseGraphic(m_pWinGroup_ButtonBigHighLight);
        pResourceManager->ReleaseGraphic(m_pWinGroup_ButtonSmallHighLight);
        pResourceManager->ReleaseGraphic(m_pWinGroup_Cursor);
        pResourceManager->ReleaseTextGroup(m_pWinGroup_Text);
        pResourceManager->ReleaseTextGroup(m_pWinGroup_Title);
        pResourceManager->ReleaseWinGroup(m_pParent);

        g_pGameData->m_pMenuElements->m_pXMLGUISystem->RemoveRecieveAllInput(this);
    }

    void ZGUIVirtualKeyboard::Update(bool)
    {
        if (m_fSmallButtonHighLight > 0.0f)
        {
            m_fSmallButtonHighLight -= g_pSysInterface->m_fActualTimeDelta;

            if (m_fSmallButtonHighLight <= 0.0f)
                m_pWinGroup_ButtonSmallHighLight->Hide(true);
        }

        if (m_fBigButtonHighLight > 0.0f)
        {
            m_fBigButtonHighLight -= g_pSysInterface->m_fActualTimeDelta;

            if (m_fBigButtonHighLight <= 0.0f)
                m_pWinGroup_ButtonBigHighLight->Hide(true);
        }

        m_fCursorTime -= g_pSysInterface->m_fActualTimeDelta;

        if (m_fCursorTime <= 0.0f)
        {
            m_bCursor = !m_bCursor;
            m_pWinGroup_Cursor->Hide(m_bCursor);

            if (m_bCursor)
                m_fCursorTime = 0.1f;
            else
                m_fCursorTime = 0.3f;
        }
    }

    void ZGUIVirtualKeyboard::Click(eZWUserEvents eEvent, int, ZXMLGUISystem* pGUISystem)
    {
        const int32_t iEvent = static_cast<int32_t>(eEvent);

        if (iEvent == eZW_SELECT)
        {
            if (m_eMode == MODE_CONTROL)
            {
                if (m_dwPosBigY != 0)
                {
                    m_fBigButtonHighLight = 0.2f;
                    m_pWinGroup_ButtonBigHighLight->Hide(false);
                }

                const uint32_t iBigY = m_dwPosBigY;

                if (iBigY == 0)
                    m_bCaps = !m_bCaps;

                if (iBigY == 1 && m_dwTextLen != 0)
                    AddChar(' ');

                if (iBigY == 2)
                    DelChar();

                if (iBigY == 3)
                    Done(pGUISystem);
            }
            else if (m_eMode == MODE_ALPHABET)
            {
                m_fSmallButtonHighLight = 0.2f;
                m_pWinGroup_ButtonSmallHighLight->Hide(false);

                const uint32_t iIndex = m_dwPosSmallX + 10 * m_dwPosSmallY;
                AddChar(m_bCaps ? kCapsKeys[iIndex] : kNormalKeys[iIndex]);
            }
        }

        if (m_eMode == MODE_ALPHABET)
        {
            switch (iEvent)
            {
                case eZW_DOWN:
                    if (m_dwPosSmallY >= 7)
                        m_dwPosSmallY = 0;
                    else
                        ++m_dwPosSmallY;
                    break;

                case eZW_UP:
                    if (m_dwPosSmallY != 0)
                        --m_dwPosSmallY;
                    else
                        m_dwPosSmallY = 7;
                    break;

                case eZW_RIGHT:
                    ++m_dwPosSmallX;

                    if (m_dwPosSmallX >= 10)
                    {
                        m_dwPosSmallX = 0;
                        m_dwPosBigY = m_dwPosSmallY >> 1;
                        m_eMode = MODE_CONTROL;
                    }
                    break;

                case eZW_LEFT:
                    if (m_dwPosSmallX != 0)
                    {
                        --m_dwPosSmallX;
                    }
                    else
                    {
                        m_dwPosSmallX = 9;
                        m_dwPosBigY = m_dwPosSmallY >> 1;
                        m_eMode = MODE_CONTROL;
                    }
                    break;
            }
        }
        else if (m_eMode == MODE_CONTROL)
        {
            switch (iEvent)
            {
                case eZW_DOWN:
                    if (m_dwPosBigY >= 3)
                        m_dwPosBigY = 0;
                    else
                        ++m_dwPosBigY;
                    break;

                case eZW_UP:
                    if (m_dwPosBigY != 0)
                        --m_dwPosBigY;
                    else
                        m_dwPosBigY = 3;
                    break;

                case eZW_RIGHT:
                {
                    const uint32_t iPrev = m_dwPosSmallX;
                    uint32_t iNew = 2 * m_dwPosBigY;
                    m_eMode = MODE_ALPHABET;
                    m_dwPosSmallY = 0;

                    if (iNew == iPrev || iNew + 1 == iPrev)
                        iNew = iPrev;

                    m_dwPosSmallX = iNew;
                    break;
                }

                case eZW_LEFT:
                {
                    const uint32_t iPrev = m_dwPosSmallX;
                    uint32_t iNew = 2 * m_dwPosBigY;
                    m_eMode = MODE_ALPHABET;
                    m_dwPosSmallY = 9;

                    if (iNew == iPrev || iNew + 1 == iPrev)
                        iNew = iPrev;

                    m_dwPosSmallX = iNew;
                    break;
                }
            }
        }

        if (m_eMode == MODE_CONTROL)
        {
            m_pWinGroup_ButtonSmall->Hide(true);
            m_pWinGroup_ButtonBig->Hide(false);

            const float fY = static_cast<float>(54 * m_dwPosBigY + 40);
            m_pWinGroup_ButtonBig->SetPos(Vector3(14.0f, fY, 0.0f));
            m_pWinGroup_ButtonBigHighLight->SetPos(Vector3(14.0f, fY, 0.0f));
        }
        else if (m_eMode == MODE_ALPHABET)
        {
            m_pWinGroup_ButtonSmall->Hide(false);
            m_pWinGroup_ButtonBig->Hide(true);

            const float fX = static_cast<float>(27 * m_dwPosSmallX + 69);
            const float fY = static_cast<float>(27 * m_dwPosSmallY + 42);
            m_pWinGroup_ButtonSmallHighLight->SetPos(Vector3(fX, fY, 0.0f));
            m_pWinGroup_ButtonSmall->SetPos(Vector3(fX, fY, 0.0f));
        }

        if (m_bCaps)
        {
            m_pWinGroup_BackgroundNormal->Hide(true);
            m_pWinGroup_BackgroundCaps->Hide(false);
            m_pWinGroup_ButtonBigHighLightCaps->Hide(false);
        }
        else
        {
            m_pWinGroup_BackgroundNormal->Hide(false);
            m_pWinGroup_BackgroundCaps->Hide(true);
            m_pWinGroup_ButtonBigHighLightCaps->Hide(true);
        }
    }

    void ZGUIVirtualKeyboard::Done(ZXMLGUISystem*)
    {
        ZASSERT(false);
    }

    void ZGUIVirtualKeyboard::UpdateText()
    {
        UTC4CHAR aTemp[256];
        UTC4CHAR aText[256];
        aText[0] = 0;

        if (m_dwTextLen != 0)
        {
            if (m_iTextPixelLimit != 0)
            {
                for (uint32_t i = 0; i < m_dwTextLen; ++i)
                    aTemp[i] = m_utc4Text[m_dwTextLen - 1 - i];

                uint32_t iVisible = 0;

                do
                {
                    aText[iVisible] = aTemp[iVisible];
                    aText[iVisible + 1] = 0;

                    char szUtf8[1280];
                    BuildUtf8(aText, szUtf8);
                    m_pWinGroup_Text->SetText(szUtf8);

                    ZLINEOBJ* pLineObj = FindLineObj(m_pWinGroup_Text);
                    ZASSERT(pLineObj != nullptr);

                    const int32_t iPixelPos = pLineObj->GetCharPosition(static_cast<int>(++iVisible));

                    if (m_dwTextLen == iVisible || iPixelPos >= m_iTextPixelLimit)
                        break;
                }
                while (true);

                for (uint32_t i = 0; i < iVisible; ++i)
                    aText[i] = m_utc4Text[m_dwTextLen - iVisible + i];

                aText[iVisible] = 0;
            }
            else
            {
                uint32_t i = 0;

                while (i < m_dwTextLen && m_utc4Text[i] != 0)
                {
                    aText[i] = m_utc4Text[i];
                    ++i;
                }

                aText[i] = 0;
            }
        }

        char szUtf8[1280];
        BuildUtf8(aText, szUtf8);
        m_pWinGroup_Text->SetText(szUtf8);

        Vector3 vTextRight;
        GetRightPosOfTextGroup(m_pWinGroup_Text, &vTextRight, 0);
        m_pWinGroup_Cursor->SetPos(Vector3(vTextRight.x - 12.0f, 16.0f, 0.0f));
    }

    void ZGUIVirtualKeyboard::AddChar(uint32_t uChar)
    {
        if (m_dwTextLen < static_cast<uint32_t>(m_dwTextLimit))
        {
            m_utc4Text[m_dwTextLen] = uChar;
            ++m_dwTextLen;
            m_utc4Text[m_dwTextLen] = 0;
            UpdateText();
        }
    }

    void ZGUIVirtualKeyboard::DelChar()
    {
        if (m_dwTextLen != 0)
            --m_dwTextLen;
        else
            m_dwTextLen = 0;

        UpdateText();
    }

    void ZGUIVirtualKeyboard::SetTextUC(const UTC4CHAR* pText)
    {
        uint32_t i = 0;

        if (pText != nullptr)
        {
            while (i < static_cast<uint32_t>(m_dwTextLimit) && i < 255 && pText[i] != 0)
            {
                m_utc4Text[i] = pText[i];
                ++i;
            }
        }

        m_utc4Text[i] = 0;
        m_dwTextLen = i;
        UpdateText();
    }

    void ZGUIVirtualKeyboard::GetTextUC(UTC4CHAR* pText) const
    {
        if (pText == nullptr)
            return;

        for (uint32_t i = 0; i <= m_dwTextLen; ++i)
            pText[i] = m_utc4Text[i];
    }

    void ZGUIVirtualKeyboard::SetText(zstring strText)
    {
        uint32_t i = 0;
        const char* pText = strText.c_str();

        while (pText != nullptr && *pText != 0 && i < static_cast<uint32_t>(m_dwTextLimit) && i < 255)
        {
            m_utc4Text[i] = Utf8ToUtf32(&pText);
            ++i;
        }

        m_utc4Text[i] = 0;
        m_dwTextLen = i;
        UpdateText();
    }

    zstring ZGUIVirtualKeyboard::GetText() const
    {
        char szUtf8[1280];
        int32_t iOffset = 0;

        for (uint32_t i = 0; i < m_dwTextLen; ++i)
            iOffset += Utf32ToUtf8(&szUtf8[iOffset], m_utc4Text[i]);

        szUtf8[iOffset] = 0;
        return zstring(szUtf8);
    }
}
