#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/GUI/Frame/ZFRAME.h>
#include <cstring>


namespace Glacier
{
    IGUIElement::IGUIElement()
    {
        m_pColorSet = nullptr;
        m_iIndex = 0;
        m_iControlPrompt = 1;
        m_eFontType = FT_MENU;
        m_eAlignment = ELEFT;
        m_pButtonGraphic = nullptr;
        m_bShadow = true;
    }

    void IGUIElement::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUIBase::readParams(ppParams, pElems);

        const char* pColorSetName = GUI::GetAttr(ppParams, "ColorSet", false);
        if (!pColorSetName)
            pColorSetName = "Default";

        m_pColorSet = pElems->GetColorSet(pColorSetName);

        const char* pFont = GUI::GetAttr(ppParams, "Font", false);
        if (pFont)
        {
            static const struct
            {
                const char* pName;
                EFontType eValue;
            } s_aFonts[] = {
                { "Header", FT_HEADER },
                { "Menu", FT_MENU },
                { "Text", FT_TEXT },
            };

            for (const auto& sFont : s_aFonts)
            {
                if (strcmp(sFont.pName, pFont) == 0)
                {
                    m_eFontType = sFont.eValue;
                    break;
                }
            }
        }

        const char* pAlignment = GUI::GetAttr(ppParams, "Alignment", false);
        if (pAlignment)
        {
            static const struct
            {
                const char* pName;
                EAlignment eValue;
            } s_aAlignments[] = {
                { "Left", ELEFT },
                { "Right", ERIGHT },
                { "Center", ECENTER },
            };

            for (const auto& sAlignment : s_aAlignments)
            {
                if (strcmp(sAlignment.pName, pAlignment) == 0)
                {
                    m_eAlignment = sAlignment.eValue;
                    break;
                }
            }
        }

        const char* pButtonGraphic = GUI::GetAttr(ppParams, "ButtonGraphic", false);
        if (pButtonGraphic)
        {
            m_pButtonGraphic = pElems->GetButtonGraphic(pButtonGraphic);
            ZASSERT(m_pButtonGraphic != nullptr);
        }

        GuiOption::readBool(&m_bShadow, ppParams, "Shadow", false);
    }

    void IGUIElement::Update(bool)
    {
    }

    void IGUIElement::Click(eZWUserEvents, int, ZXMLGUISystem*)
    {
    }

    void IGUIElement::OpenSubwindow()
    {
    }

    void IGUIElement::SetValue(int)
    {
    }

    bool IGUIElement::Use()
    {
        return true;
    }

    void IGUIElement::FocusUpdate()
    {
    }

    ZGUIElementLink IGUIElement::Setup(float*, ZResourceManager*, ZWINGROUP*)
    {
        return ZGUIElementLink();
    }

    void IGUIElement::ReleaseResources(ZResourceManager*)
    {
    }

    bool IGUIElement::SetFocus(bool)
    {
        return false;
    }

    void IGUIElement::Cancel()
    {
    }

    void IGUIElement::Invalidate()
    {
    }

    const char* IGUIElement::GetWindow()
    {
        return nullptr;
    }

    int32_t IGUIElement::GenerateSubId(int iId)
    {
        return m_iIndex + (iId << 16);
    }

    int32_t IGUIElement::GetId(int iSubId)
    {
        return iSubId >> 16;
    }

    int32_t IGUIElement::GetIndexFromId(int iSubId)
    {
        return static_cast<uint16_t>(iSubId);
    }

    void IGUIElement::CloseWindow(bool bCloseAll)
    {
        g_pGameData->m_pMenuElements->m_pXMLGUISystem->CloseWindow(bCloseAll);
    }

    void IGUIElement::RollBackIfOnStack(const char* pszWindowName, bool bPushWindow, bool bResetStack)
    {
        ZXMLGUISystem* pGUISystem = g_pGameData->m_pMenuElements->m_pXMLGUISystem;

        if (pGUISystem->IsOnStack(pszWindowName))
            pGUISystem->RollBack(pszWindowName, nullptr, false, false);
        else
            pGUISystem->OpenWindow(pszWindowName, bPushWindow, bResetStack);
    }

    void IGUIElement::ChangeColorSet(ZWINGROUP* pGroup, ZColorSet* pColorSet)
    {
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZWINOBJ>() || pGeom->IsDerivedFrom<ZFRAME>())
                continue;

            ZWINOBJ* pWinObj = static_cast<ZWINOBJ*>(pGeom);
            const uint8_t dwType = pWinObj->m_dwType;

            if (dwType & 0x01)
                pWinObj->SetColor(pColorSet->GetColor(static_cast<ZColorSet::EColorIndex>(0)));

            if (dwType & 0x08)
                pWinObj->SetColor(pColorSet->GetColor(static_cast<ZColorSet::EColorIndex>(1)));

            if (dwType & 0x20)
                pWinObj->SetColor(pColorSet->GetColor(static_cast<ZColorSet::EColorIndex>(2)));
        }
    }

    void IGUIElement::ChangeColor(ZWINGROUP* pGroup, ZColorSet* pColorSet, ZColorSet::EColorIndex eColorIndex)
    {
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZWINOBJ>() || pGeom->IsDerivedFrom<ZFRAME>())
                continue;

            static_cast<ZWINOBJ*>(pGeom)->SetColor(pColorSet->GetColor(eColorIndex));
        }
    }

    void IGUIElement::SetColor(uint32_t rgba, ZWINGROUP* pGroup, uint32_t mask)
    {
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupLast; ForNotGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZWINOBJ>())
                continue;

            ZWINOBJ* pWinObj = static_cast<ZWINOBJ*>(pGeom);
            if ((mask & pWinObj->m_dwType) != 0)
                pWinObj->SetColor(rgba);
        }
    }

    void IGUIElement::GetRightPosOfTextGroup(ZWINGROUP* pGroup, Glacier::Vector3* pPos, int iOffsetByX)
    {
        pGroup->GetPos(*pPos);

        int32_t iMaxRight = 0;
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupLast; ForNotGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZLINEOBJ>())
                continue;

            ZLINEOBJ* pLineObj = static_cast<ZLINEOBJ*>(pGeom);
            const char* pText = pLineObj->GetText();

            ZVector3 vLinePos;
            pGeom->GetPos(vLinePos);

            if (pText)
            {
                const int32_t iWidth = pLineObj->GetStringWidth(pText, static_cast<int>(strlen(pText)));
                if (iWidth + static_cast<int32_t>(vLinePos.x) > iMaxRight)
                    iMaxRight = iWidth + static_cast<int32_t>(vLinePos.x);
            }
        }

        pPos->x = static_cast<float>(iMaxRight + iOffsetByX) + pPos->x;
    }

    void IGUIElement::SetLineSpace(ZWINGROUP* pGroup, int iSpace)
    {
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZLINEOBJ>())
                continue;

            static_cast<ZLINEOBJ*>(pGeom)->SetLineSpacing(static_cast<int8_t>(iSpace));
        }
    }

    void IGUIElement::SetLineWidth(ZWINGROUP* pGroup, int iWidth)
    {
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZLINEOBJ>())
                continue;

            static_cast<ZLINEOBJ*>(pGeom)->SetWidth(iWidth);
        }
    }

    void IGUIElement::ChangeType(ZWINGROUP* pGroup, uint8_t uType)
    {
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZWINOBJ>())
                static_cast<ZWINOBJ*>(pGeom)->SetType(uType);
        }
    }

    void IGUIElement::SetAnimateAlpha(ZWINGROUP* pGroup, bool bAnimate)
    {
        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupFirst; pBaseGeom != nullptr; pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZWINOBJ>())
                static_cast<ZWINOBJ*>(pGeom)->m_bAnimateAlpha = bAnimate;
        }
    }

    void IGUIElement::SetAnimateAlphaRecur(ZWINGROUP* pGroup, bool bAnimate)
    {
        ZBaseGeom* pBaseGeom = pGroup->BaseGeom();
        while (pBaseGeom)
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZWINOBJ>())
                static_cast<ZWINOBJ*>(pGeom)->m_bAnimateAlpha = bAnimate;

            pGroup->RecurGetNext(&pBaseGeom);
        }
    }

    void IGUIElement::FadeRecur(ZWINGROUP* pGroup, uint8_t alpha)
    {
        ZBaseGeom* pBaseGeom = pGroup->BaseGeom();
        while (pBaseGeom)
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZWINOBJ>())
                static_cast<ZWINOBJ*>(pGeom)->SetAlpha(alpha);

            pGroup->RecurGetNext(&pBaseGeom);
        }
    }
}
