#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/GUI/Frame/ZFRAME.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t WINDOW_ALIGN_H_LEFT = 0x01u;
        constexpr uint32_t WINDOW_ALIGN_H_RIGHT = 0x02u;
        constexpr uint32_t WINDOW_ALIGN_V_TOP = 0x10u;
        constexpr uint32_t WINDOW_ALIGN_V_BOTTOM = 0x20u;
        constexpr uint32_t WINDOW_ALIGN_MODE_BORDER = 0x100u;

        constexpr float GUI_ABSOLUTE_POS = -5000.0f;
        constexpr float GUI_HORIZONTAL_SPACING = 5.0f;
    }

    ZGUIElementLink::ZGUIElementLink()
        : m_fSizeX(0.0f)
        , m_fSizeY(0.0f)
        , m_bUseLinks(false)
    {
        m_apFocusIn[0] = nullptr;
        m_apFocusIn[1] = nullptr;
        m_apFocusIn[2] = nullptr;
        m_apFocusIn[3] = nullptr;
    }

    ZGUIElementLink::ZGUIElementLink(float fSizeX, float fSizeY)
        : m_fSizeX(fSizeX)
        , m_fSizeY(fSizeY)
        , m_bUseLinks(false)
    {
        m_apFocusIn[0] = nullptr;
        m_apFocusIn[1] = nullptr;
        m_apFocusIn[2] = nullptr;
        m_apFocusIn[3] = nullptr;
    }

    ZGUIElementLink::ZGUIElementLink(float fSizeX, float fSizeY, ZCONTROL* pFocus)
        : m_fSizeX(fSizeX)
        , m_fSizeY(fSizeY)
        , m_bUseLinks(pFocus != nullptr)
    {
        m_apFocusIn[0] = pFocus;
        m_apFocusIn[1] = pFocus;
        m_apFocusIn[2] = pFocus;
        m_apFocusIn[3] = pFocus;
    }

    ZGUIElementLink::ZGUIElementLink(float fSizeX, float fSizeY, ZCONTROL* pFocus0, ZCONTROL* pFocus1, ZCONTROL* pFocus2, ZCONTROL* pFocus3)
        : m_fSizeX(fSizeX)
        , m_fSizeY(fSizeY)
        , m_bUseLinks(pFocus0 != nullptr || pFocus1 != nullptr || pFocus2 != nullptr || pFocus3 != nullptr)
    {
        m_apFocusIn[0] = pFocus0;
        m_apFocusIn[1] = pFocus1;
        m_apFocusIn[2] = pFocus2;
        m_apFocusIn[3] = pFocus3;
    }

    ZGUIElementLink& ZGUIElementLink::operator=(const ZGUIElementLink& other)
    {
        m_fSizeX = other.m_fSizeX;
        m_fSizeY = other.m_fSizeY;
        m_apFocusIn[0] = other.m_apFocusIn[0];
        m_apFocusIn[1] = other.m_apFocusIn[1];
        m_apFocusIn[2] = other.m_apFocusIn[2];
        m_apFocusIn[3] = other.m_apFocusIn[3];
        m_bUseLinks = other.m_bUseLinks;
        return *this;
    }

    bool ZGUIElementLink::UseLinks() const
    {
        return m_bUseLinks;
    }

    IWindowInterface::ZElementExtraInfo::ZElementExtraInfo()
        : m_pGUIElement(nullptr)
        , m_iAlignmentOverride(0)
    {
    }

    IWindowInterface::ZElementExtraInfo::ZElementExtraInfo(IGUIElement* pGUIElement, ENavigation eNavigation, int32_t iAlignmentId)
        : m_pGUIElement(pGUIElement)
        , m_iAlignmentOverride(static_cast<int32_t>(eNavigation) | (iAlignmentId << 16))
    {
    }

    IWindowInterface::ZElementExtraInfo& IWindowInterface::ZElementExtraInfo::operator=(const ZElementExtraInfo& other)
    {
        m_pGUIElement = other.m_pGUIElement;
        m_iAlignmentOverride = other.m_iAlignmentOverride;
        return *this;
    }

    ENavigation IWindowInterface::ZElementExtraInfo::GetNavigation()
    {
        return static_cast<ENavigation>(m_iAlignmentOverride & 0xFFFF);
    }

    int32_t IWindowInterface::ZElementExtraInfo::GetNavigationId()
    {
        return m_iAlignmentOverride;
    }

    IWindowInterface::IWindowInterface(ZMenuElements*)
    {
        m_pResourceManager = nullptr;
        m_bPopOnBack = true;
        m_pWinGroupBackGround = nullptr;
        m_pFrameBackground = nullptr;
        m_v2WindowSize.x = 640.0f;
        m_v2WindowSize.y = 480.0f;
        m_bUseBackgroundIngame = false;
        m_bBackgroundIsFrame = false;
        m_iWindowAlignment = WINDOW_ALIGN_MODE_BORDER;
        m_iLineSpace = 3;
        m_eNavigation = EVERTICAL;
        m_bOpen = false;
        m_bRollBackMark = false;
        m_bPauseEngine = true;
    }

    void IWindowInterface::readParams(const char** ppParams, ZMenuElements* pElems)
    {
        ZGUIBase::readParams(ppParams, pElems);

        GuiOption::readBool(&m_bPopOnBack, ppParams, "PopOnBack", false);
        GuiOption::readString(m_sBackground, "Background", ppParams);
        GuiOption::readString(m_sRollBackWindow, "RollBackWindow", ppParams);
        ReadAlignment(m_iWindowAlignment, ppParams);
        GuiOption::readInt2(m_v2WindowSize.x, ppParams, "W");
        GuiOption::readInt2(m_v2WindowSize.y, ppParams, "H");
        GuiOption::readBool(&m_bUseBackgroundIngame, ppParams, "UseBackgroundIngame", false);
        GuiOption::readInt(m_iLineSpace, ppParams, "LineSpace");
        ReadNavigation(m_eNavigation, ppParams);
        GuiOption::readBool(&m_bBackgroundIsFrame, ppParams, "Frame", false);
        GuiOption::readBool(&m_bRollBackMark, ppParams, "RollBackMark", false);
        GuiOption::readBool(&m_bPauseEngine, ppParams, "PauseEngine", false);
    }

    void IWindowInterface::OpenWindow(ZResourceManager* pResourceManager, bool, ZWINGROUP* pWinGroup, bool)
    {
        m_bOpen = true;

        ZVector2 vPos {};
        ZColorSet colorSet;

        ZWINGROUP* pParent = pWinGroup;
        if (!m_sBackground.empty() && !IsDialog() && !IsTRC())
        {
            ZVector3 vGroupPos;
            pWinGroup->GetPos(vGroupPos);
            vPos.x = vGroupPos.x;
            vPos.y = vGroupPos.y;
            pParent = pResourceManager->m_pWinGroupBackground;
        }

        if (m_bBackgroundIsFrame)
        {
            m_pFrameBackground = pResourceManager->GetFrame(
                vPos, &colorSet, pParent, m_v2WindowSize, m_sBackground.c_str(), ELEFT);
        }
        else
        {
            m_pWinGroupBackGround = pResourceManager->GetGraphic(
                vPos, &colorSet, pParent, m_sBackground.c_str(), ELEFT, -1);
        }
    }

    void IWindowInterface::CloseWindow(ZResourceManager* pResourceManager, bool)
    {
        m_bOpen = false;
        pResourceManager->ReleaseGraphic(m_pWinGroupBackGround);
        pResourceManager->ReleaseFrame(m_pFrameBackground);
        m_pWinGroupBackGround = nullptr;
        m_pFrameBackground = nullptr;
    }

    void IWindowInterface::PushSubWindow(const char*, IGUIElement*, bool, bool)
    {
    }

    bool IWindowInterface::PopSubWindow()
    {
        return false;
    }

    bool IWindowInterface::Update(int)
    {
        return false;
    }

    ZWINDOW* IWindowInterface::GetTopSubWindow()
    {
        return nullptr;
    }

    ZWINDOW* IWindowInterface::GetButtomSubWindow()
    {
        return nullptr;
    }

    const ZStaticVector<IGUIElement*, 44>* IWindowInterface::GetCurrentElements()
    {
        return nullptr;
    }

    void IWindowInterface::Cancel()
    {
    }

    void IWindowInterface::Invalidate()
    {
    }

    void IWindowInterface::GrapFocus()
    {
    }

    bool IWindowInterface::IsDialog()
    {
        return false;
    }

    bool IWindowInterface::IsTRC()
    {
        return false;
    }

    int IWindowInterface::GetTRCPriority()
    {
        return 0;
    }

    void IWindowInterface::Click(IGUIElement*, eZWUserEvents, uint32_t)
    {
    }

    void IWindowInterface::GetTopLeftPos(ZVector2& rResult, ZVector2& rViewport)
    {
        const uint32_t iAlignment = m_iWindowAlignment;
        const float fWidth = m_v2WindowSize.x;
        const float fHeight = m_v2WindowSize.y;

        if (iAlignment & WINDOW_ALIGN_MODE_BORDER)
        {
            if (iAlignment & WINDOW_ALIGN_H_LEFT)
            {
                rResult.x = m_v2Pos.x;
            }
            else if (iAlignment & WINDOW_ALIGN_H_RIGHT)
            {
                rResult.x = rViewport.x - m_v2Pos.x - fWidth;
            }
            else
            {
                rResult.x = rViewport.x * 0.5f + m_v2Pos.x - fWidth * 0.5f;
            }

            if (iAlignment & WINDOW_ALIGN_V_TOP)
            {
                rResult.y = m_v2Pos.y;
            }
            else if (iAlignment & WINDOW_ALIGN_V_BOTTOM)
            {
                rResult.y = rViewport.y - m_v2Pos.y - fHeight;
            }
            else
            {
                rResult.y = rViewport.y * 0.5f + m_v2Pos.y - fHeight * 0.5f;
            }
        }
        else
        {
            if (iAlignment & WINDOW_ALIGN_H_LEFT)
            {
                rResult.x = m_v2Pos.x;
            }
            else if (iAlignment & WINDOW_ALIGN_H_RIGHT)
            {
                rResult.x = m_v2Pos.x - fWidth;
            }
            else
            {
                rResult.x = m_v2Pos.x - fWidth * 0.5f;
            }

            if (iAlignment & WINDOW_ALIGN_V_TOP)
            {
                rResult.y = m_v2Pos.y;
            }
            else if (iAlignment & WINDOW_ALIGN_V_BOTTOM)
            {
                rResult.y = m_v2Pos.y - fHeight;
            }
            else
            {
                rResult.y = m_v2Pos.y - fHeight * 0.5f;
            }
        }
    }

    void IWindowInterface::SetFirstFocus(ZStaticVector<IGUIElement*, 44>& rElements)
    {
        for (uint32_t i = 0; i < rElements.size(); ++i)
        {
            if (rElements[i]->SetFocus(false))
                break;
        }
    }

    void IWindowInterface::CallCancel(ZStaticVector<IGUIElement*, 44>& rElements)
    {
        for (uint32_t i = 0; i < rElements.size(); ++i)
            rElements[i]->Cancel();
    }

    ZCONTROL* IWindowInterface::GetUpLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements)
    {
        for (int i = iIndex - 1; i >= 0; --i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[3];
        }

        for (int i = static_cast<int>(rElements.size()) - 1; i >= iIndex; --i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[3];
        }

        return nullptr;
    }

    ZCONTROL* IWindowInterface::GetDownLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements)
    {
        for (int i = iIndex + 1; i < static_cast<int>(rElements.size()); ++i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[2];
        }

        for (int i = 0; i < iIndex; ++i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[2];
        }

        return nullptr;
    }

    ZCONTROL* IWindowInterface::GetLeftLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements)
    {
        for (int i = iIndex - 1; i >= 0; --i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[1];
        }

        for (int i = static_cast<int>(rElements.size()) - 1; i >= iIndex; --i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[1];
        }

        return nullptr;
    }

    ZCONTROL* IWindowInterface::GetRightLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements)
    {
        for (int i = iIndex + 1; i < static_cast<int>(rElements.size()); ++i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[0];
        }

        for (int i = 0; i < iIndex; ++i)
        {
            if (rElements[i].UseLinks())
                return rElements[i].m_apFocusIn[0];
        }

        return nullptr;
    }

    ZGUIElementLink IWindowInterface::SetupGUIElements(float* pfPos, ENavigation eNavigation,
        ZStaticVector<IGUIElement*, 44>& rElements, ZResourceManager* pResourceManager, ZWINGROUP* pParent,
        ZGUIElementLink* pPrevElementLink)
    {
        const bool bHorizontal = (eNavigation == EHORIZONTAL);

        float fX = pfPos[0];
        float fY = pfPos[1];

        ZStaticVector<ZGUIElementLink, 44> aElementLinks;

        for (uint32_t i = 0; i < rElements.size(); ++i)
        {
            IGUIElement* pElement = rElements[i];

            if (!bHorizontal)
            {
                const EAlignment eAlignment = pElement->m_eAlignment;

                if (eAlignment == ELEFT)
                    fX = pfPos[0];
                else if (eAlignment == ECENTER)
                    fX = m_v2WindowSize.x * 0.5f;
                else if (eAlignment == ERIGHT)
                    fX = m_v2WindowSize.x;
            }

            ZGUIElementLink link;

            if (pElement->HasAbsolutePos())
            {
                ZVector2 vElementPos;
                pElement->GetPos(&vElementPos);

                float fLinkX = vElementPos.x;
                float fLinkY = vElementPos.y;

                if (fLinkX == GUI_ABSOLUTE_POS)
                    fLinkX = fX;

                if (fLinkY <= GUI_ABSOLUTE_POS)
                    fLinkY = fY;

                float aLinkPos[2] = { fLinkX, fLinkY };
                link = pElement->Setup(aLinkPos, pResourceManager, pParent);

                fX = fLinkX;
                fY = fLinkY;
            }
            else
            {
                float aLinkPos[2] = { fX, fY };
                link = pElement->Setup(aLinkPos, pResourceManager, pParent);
            }

            if (bHorizontal)
            {
                fX += link.m_fSizeX + GUI_HORIZONTAL_SPACING;
            }
            else
            {
                fY += static_cast<float>(m_iLineSpace) + link.m_fSizeY;
                pfPos[1] = fY;
            }

            aElementLinks.push_back(link);
        }

        if (bHorizontal)
        {
            if (!aElementLinks.empty())
                pfPos[1] = pfPos[1] + static_cast<float>(m_iLineSpace) + aElementLinks[0].m_fSizeY;

            for (uint32_t i = 0; i < aElementLinks.size(); ++i)
            {
                ZGUIElementLink& rLink = aElementLinks[i];

                if (!rLink.UseLinks())
                    continue;

                rLink.m_apFocusIn[0]->SetNextFocus(GetUpLink(static_cast<int>(i), aElementLinks), Left);
                rLink.m_apFocusIn[1]->SetNextFocus(GetDownLink(static_cast<int>(i), aElementLinks), Right);
            }

            if (pPrevElementLink)
            {
                bool bFirstElement = true;

                for (uint32_t i = 0; i < aElementLinks.size(); ++i)
                {
                    ZGUIElementLink& rLink = aElementLinks[i];

                    if (!rLink.UseLinks())
                        continue;

                    if (bFirstElement)
                    {
                        if (pPrevElementLink->m_apFocusIn[3])
                        {
                            pPrevElementLink->m_apFocusIn[3]->SetNextFocus(rLink.m_apFocusIn[2], Down);
                            rLink.m_apFocusIn[2]->SetNextFocus(pPrevElementLink->m_apFocusIn[3], Up);
                        }

                        if (pPrevElementLink->m_apFocusIn[2])
                        {
                            pPrevElementLink->m_apFocusIn[2]->SetNextFocus(rLink.m_apFocusIn[3], Up);
                            rLink.m_apFocusIn[3]->SetNextFocus(pPrevElementLink->m_apFocusIn[2], Down);
                        }

                        bFirstElement = false;
                    }
                    else
                    {
                        rLink.m_apFocusIn[2]->SetNextFocus(pPrevElementLink->m_apFocusIn[3], Up);
                    }
                }
            }
        }
        else
        {
            bool bUseLinks = false;

            for (uint32_t i = 0; i < aElementLinks.size() && !bUseLinks; ++i)
                bUseLinks = aElementLinks[i].UseLinks();

            if (!bUseLinks)
            {
                if (pPrevElementLink)
                    return *pPrevElementLink;

                return ZGUIElementLink();
            }

            if (pPrevElementLink)
            {
                aElementLinks.push_back(ZGUIElementLink());

                for (uint32_t i = aElementLinks.size() - 1; i > 0; --i)
                    aElementLinks[i] = aElementLinks[i - 1];

                aElementLinks[0] = *pPrevElementLink;
            }

            for (uint32_t i = 0; i < aElementLinks.size(); ++i)
            {
                ZGUIElementLink& rLink = aElementLinks[i];

                if (!rLink.UseLinks())
                    continue;

                rLink.m_apFocusIn[2]->SetNextFocus(GetUpLink(static_cast<int>(i), aElementLinks), Up);
                rLink.m_apFocusIn[3]->SetNextFocus(GetDownLink(static_cast<int>(i), aElementLinks), Down);
            }
        }

        ZCONTROL* pFirstLink = nullptr;
        ZCONTROL* pLastLink = nullptr;

        if (bHorizontal)
        {
            for (uint32_t i = 0; i < aElementLinks.size(); ++i)
            {
                if (aElementLinks[i].UseLinks())
                {
                    pFirstLink = aElementLinks[i].m_apFocusIn[2];
                    pLastLink = aElementLinks[i].m_apFocusIn[3];
                    break;
                }
            }
        }
        else
        {
            pFirstLink = GetDownLink(static_cast<int>(aElementLinks.size()), aElementLinks);
            pLastLink = GetUpLink(0, aElementLinks);
        }

        if (pPrevElementLink && pPrevElementLink->m_apFocusIn[2])
            pFirstLink = pPrevElementLink->m_apFocusIn[2];

        return ZGUIElementLink(0.0f, 0.0f, nullptr, nullptr, pFirstLink, pLastLink);
    }
}
