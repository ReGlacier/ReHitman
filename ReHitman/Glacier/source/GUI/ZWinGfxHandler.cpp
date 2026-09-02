#include <Glacier/GUI/ZWinGfxHandler.h>
#include <Glacier/GUI/ZBUTTON.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/GUI/ZCONTROL.h>
#include <Glacier/GUI/ZFONT.h>
#include <Glacier/GUI/ZFRAME.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    // ZWinGfxHandler
    ZWinGfxHandler::ZWinGfxHandler(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINGROUP(psName, pBaseGeom)
    {
    }

    ZWinGfxHandler::~ZWinGfxHandler() = default;

    const RTP::ZPropertyInfo& ZWinGfxHandler::GetProperties() const
    {
        return ZWinGfxHandler::Info;
    }

    uint32_t ZWinGfxHandler::GetObjectId() const
    {
        return ZWinGfxHandler::m_Id;
    }

    void ZWinGfxHandler::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWinGfxHandler::m_Id;
        mask = ZWinGfxHandler::m_Mask;
    }

    ZGEOMCLASSINFO* ZWinGfxHandler::GetOldClassInfo() const
    {
        return ZWinGfxHandler::m_OldClassInfo;
    }

    void ZWinGfxHandler::ClassInit()
    {
        ZWINGROUP::ClassInit();
        Hide(true);
    }

    void ZWinGfxHandler::CopyData(const ZGEOM* pSource)
    {
        ZWINGROUP::CopyData(pSource);
        const auto* pHandler = static_cast<const ZWinGfxHandler*>(pSource);
        m_rHoriScrollBar = pHandler->m_rHoriScrollBar;
        m_rVertScrollBar = pHandler->m_rVertScrollBar;
        m_rCheckButton = pHandler->m_rCheckButton;
        m_rFrame0 = pHandler->m_rFrame0;
        m_rFrame1 = pHandler->m_rFrame1;
        m_rClickSound = pHandler->m_rClickSound;
        m_rHoverSound = pHandler->m_rHoverSound;
        m_rDefaultFont = pHandler->m_rDefaultFont;
        m_dwNormalColor = pHandler->m_dwNormalColor;
        m_dwHoverColor = pHandler->m_dwHoverColor;
        m_dwFocusColor = pHandler->m_dwFocusColor;
        m_dwPushColor = pHandler->m_dwPushColor;
        m_dwDisabledColor = pHandler->m_dwDisabledColor;
        m_dwCheckColor = pHandler->m_dwCheckColor;
        m_szYes = pHandler->m_szYes;
        m_szNo = pHandler->m_szNo;
    }

    ZCHAROBJ* ZWinGfxHandler::CreateText(ZWINGROUP* pGroup, const char* psText, uint32_t lResourceId)
    {
        auto* pFont = reinterpret_cast<ZFONT*>(ZGEOM::RefToPtr(m_rDefaultFont));
        if (!pFont)
            return nullptr;

        auto* pText = static_cast<ZCHAROBJ*>(pGroup->CreateResourceGeom("Text", 0x43484F42u, lResourceId, true));
        if (!pText)
            return nullptr;

        pText->SetFont(pFont);
        pText->SetColor(m_dwNormalColor);
        pText->SetText(*psText == '%' ? psText + 1 : psText);
        return pText->DoInit() ? pText : nullptr;
    }

    ZLINEOBJ* ZWinGfxHandler::CreateMultiLineText(ZWINGROUP* pGroup, const char* psText, uint32_t lWidth)
    {
        auto* pFont = reinterpret_cast<ZFONT*>(ZGEOM::RefToPtr(m_rDefaultFont));
        if (!pFont)
            return nullptr;

        auto* pText = static_cast<ZLINEOBJ*>(pGroup->CreateGeom("MultiLineText", 0x200038u, true));
        if (!pText)
            return nullptr;

        pText->SetFont(pFont);
        pText->SetColor(m_dwNormalColor);
        pText->SetWidth(static_cast<int>(lWidth));
        pText->SetText(*psText == '%' ? psText + 1 : psText);
        return pText->DoInit() ? pText : nullptr;
    }

    ZBUTTON* ZWinGfxHandler::CreateTextButton(
        ZWINGROUP* pGroup, const char* psText, uint32_t lControlId, uint32_t lColor)
    {
        auto* pButton = static_cast<ZBUTTON*>(pGroup->CreateGeom("TextButton", 0x100033u, true));
        if (!pButton)
            return nullptr;

        pButton->m_dwNormalColor = m_dwNormalColor;
        pButton->m_dwHoverColor = m_dwHoverColor;
        pButton->m_dwFocusColor = m_dwFocusColor;
        pButton->m_dwPushColor = m_dwPushColor;
        pButton->m_dwDisabledColor = m_dwDisabledColor;
        pButton->m_dwCheckColor = m_dwCheckColor;
        pButton->m_bTextControl = true;

        ZCHAROBJ* pText = CreateText(pButton, psText, 0x20002Du);
        if (!pText)
        {
            pButton->Delete();
            return nullptr;
        }

        pText->SetType(47);
        pButton->SetControlId(static_cast<int>(lControlId));
        if (!pButton->DoInit())
            return nullptr;

        pButton->Activate(true);
        pText->SetColor(lColor);
        return pButton;
    }

    ZFRAME* ZWinGfxHandler::CreateFrame(
        ZWINGROUP* pGroup, uint32_t lWidth, uint32_t lHeight, uint32_t lFrame)
    {
        ZGEOM* pTemplate = ZGEOM::RefToPtr(lFrame ? m_rFrame1 : m_rFrame0);
        if (!pTemplate)
            return nullptr;

        ZMat3x3 mat{};
        mreset(mat.data);
        ZVector3 pos{};
        auto* pFrame = static_cast<ZFRAME*>(pTemplate->DuplicateInit(pGroup, &mat, &pos, nullptr, false));
        if (pFrame)
            pFrame->SetSize(static_cast<int>(lWidth), static_cast<int>(lHeight));
        return pFrame;
    }

    ZPANEL* ZWinGfxHandler::CreateSimpleDialog(
        ZWINGROUP* pGroup, ZCONTROL*, const char* psText, uint32_t lControlId, uint32_t lColor, uint32_t lWidth)
    {
        auto* pDialog = static_cast<ZWINGROUP*>(pGroup->CreateGeom("Dialog", 0x100037u, true));
        if (!pDialog)
            return nullptr;

        ZLINEOBJ* pText = CreateMultiLineText(pDialog, psText, lWidth);
        if (!pText)
        {
            pDialog->Delete();
            return nullptr;
        }

        const uint32_t textHeight = static_cast<uint32_t>(pText->Size().y * 2.0f);
        ZFRAME* pFrame = CreateFrame(pDialog, lWidth + 20, textHeight + 40, 0);
        if (!pFrame)
        {
            pDialog->Delete();
            return nullptr;
        }

        pFrame->SetPos(0.0f, 0.0f, 100.0f);
        const uint32_t frameWidth = static_cast<uint32_t>(pFrame->Size().x * 2.0f);
        const uint32_t frameHeight = static_cast<uint32_t>(pFrame->Size().y * 2.0f);
        pText->SetPos(static_cast<float>(frameWidth / 2), static_cast<float>(frameHeight / 2 - 8), 80.0f);
        pText->SetAlignment(68);

        ZBUTTON* pYes = CreateTextButton(pDialog, m_szYes, lControlId, 33);
        ZBUTTON* pNo = CreateTextButton(pDialog, m_szNo, lControlId, 34);
        if (!pYes || !pNo)
        {
            pDialog->Delete();
            return nullptr;
        }

        pYes->SetPos(static_cast<float>(frameWidth / 2 - 8), static_cast<float>(frameHeight - 12), 80.0f);
        pNo->SetPos(static_cast<float>(frameWidth / 2 + 8), static_cast<float>(frameHeight - 12), 80.0f);
        pDialog->SetPos(
            (static_cast<float>(g_pSysInterface->m_lResolution[0]) - pGroup->Pos().x - static_cast<float>(frameWidth)) * 0.5f,
            (static_cast<float>(g_pSysInterface->m_lResolution[1]) - pGroup->Pos().y - static_cast<float>(frameHeight)) * 0.5f,
            0.0f);
        return reinterpret_cast<ZPANEL*>(pDialog);
    }


#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(ZWinGfxHandler, ZWINGROUP, 0x009A29D0, "ZWinGfxHandler", 0x0077D274, nullptr, 0x008079C0, 0x009A2980, 0x009A2984);
#   pragma endregion
}
