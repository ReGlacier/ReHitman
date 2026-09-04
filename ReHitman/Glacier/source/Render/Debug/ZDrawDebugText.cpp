#include <Glacier/Render/Debug/ZDrawDebugRender.h>
#include <Glacier/Render/Debug/ZDrawDebugText.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZDrawDebugText::ZMenu::ZMenu()
    {
        m_pName = nullptr;
        m_CallBack = nullptr;
        m_pData = nullptr;
        m_pMenu = nullptr;
    }

    ZDrawDebugText::ZMenu::ZMenu(const ZMenu& copy)
    {
        m_pName = copy.m_pName;
        m_CallBack = copy.m_CallBack;
        m_pData = copy.m_pData;
        m_pMenu = copy.m_pMenu;
    }

    ZDrawDebugText::ZMenu& ZDrawDebugText::ZMenu::operator=(const ZDrawDebugText::ZMenu& rhs)
    {
        m_pName = rhs.m_pName;
        m_CallBack = rhs.m_CallBack;
        m_pData = rhs.m_pData;
        m_pMenu = rhs.m_pMenu;
        return *this;
    }

    ZDrawDebugText::ZDrawDebugText()
        : ZDebugFrame(nullptr)
        , m_Menus()
        , m_bInitialized(false)
        , m_bSelectorMenu(false)
        , m_bLocked(false)
    {
        m_Menus.Clear();
        g_pDrawDebugText = this;
    }

    void ZDrawDebugText::Update(ZDrawDebugRender* pRender)
    {
        if (g_pDrawDebugText == this)
        {
            // TODO: Finish me
        }

        m_pRender->xReset();
        Update();
        // TODO: Finish me
    }

    void ZDrawDebugText::TextPlot(uint32_t x, uint32_t y, const char* pText, uint32_t dwTextColor)
    {
        DrawText(x, y, pText, dwTextColor);
    }

    void ZDrawDebugText::TextPlotInvers(uint32_t x, uint32_t y, const char* pText, uint32_t dwTextColor, uint32_t dwBgColor)
    {
        const auto lLen = strlen(pText);

        DrawRect(x, y, 9 * lLen, 15, dwBgColor);
        DrawText(x, y, pText, dwTextColor);
    }

    void ZDrawDebugText::AddMenu(const char* pName, void(*CallBack)(const void*, bool), const void* pData)
    {
        for (int i = 0; i < m_Menus.Count(); ++i)
        {
            auto* ptr = m_Menus.Get(i);

            if (!strcasecmp(ptr->m_pName, pName))
            {
                ptr->m_pMenu = nullptr;
                ptr->m_CallBack = CallBack;
                ptr->m_pData = pData;
                return;
            }
        }

        ZDrawDebugText::ZMenu sNewMenu {};
        sNewMenu.m_CallBack = CallBack;
        sNewMenu.m_pName = pName;
        sNewMenu.m_pMenu = nullptr;
        m_Menus.Add(&sNewMenu);
    }

    void ZDrawDebugText::AddMenu(const char* pName, ZSimpleDebugMenu* pSimpleMenu)
    {
        for (int i = 0; i < m_Menus.Count(); ++i)
        {
            auto* ptr = m_Menus.Get(i);

            if (!strcasecmp(ptr->m_pName, pName))
            {
                ptr->m_pMenu = pSimpleMenu;
                ptr->m_CallBack = nullptr;
                ptr->m_pData = nullptr;
                return;
            }
        }

        ZDrawDebugText::ZMenu sNewMenu {};
        sNewMenu.m_pName = pName;
        sNewMenu.m_pMenu = pSimpleMenu;
        m_Menus.Add(&sNewMenu);
    }

    void ZDrawDebugText::Init()
    {
        if (m_bInitialized)
            return;

        m_bInitialized = true;
        m_pActiveMenu = nullptr;
        m_bSelectorMenu = false;
        m_bLocked = false;

        ZDrawDebugRegion::Set(0, 0, g_pSysInterface->m_lResolution[0], g_pSysInterface->m_lResolution[1]);
        g_RenderDebugMenu.Init();

        // TODO: Finish me
        // TODO: Other debug menus here
    }

    void ZDrawDebugText::End()
    {
        // Do nothing
    }

    void ZDrawDebugText::Update()
    {
        // TODO: Finish me
        DrawInfo();

        for (int i = 0; i < m_Menus.Count(); ++i)
        {
            if (auto* pMenu = m_Menus.Get(i); pMenu && pMenu->m_pMenu)
            {
                pMenu->m_pMenu->DrawAlways();
            }
        }
        // TODO: Finish me
    }

    void ZDrawDebugText::Lock()
    {
        m_bLocked = true;
    }

    void ZDrawDebugText::Unlock()
    {
        m_bLocked = false;
    }

    void ZDrawDebugText::DrawInfo()
    {
        if (!g_bInfoDisplayEnabled)
            return;

        auto* pFrame = AddFrame();

        SetPosSizeText(1, 1, 20, 20);
        Fill(0x80202080u);
        DrawFocus();

        // TODO: Finish me
    }

    void ZDrawDebugText::DrawText(uint32_t x, uint32_t y, const char* pszText, uint32_t lColor)
    {
        ZASSERT(m_pRender);

        auto lPosY = m_pRender->m_Viewport.y - y;
        // TODO: Finish me
    }

    void ZDrawDebugText::DrawRect(uint32_t lPosX, int lPosY, int lSizeX, int lSizeY, uint32_t lColor)
    {
        ZASSERT(m_pRender);

        auto vp = m_pRender->Viewport();
        vp.x = vp.x + 12 - lPosY;

        m_pRender->xSetTexture(0u);
        m_pRender->xSetDrawMode(0x60001u);
        m_pRender->xColor(lColor);
        m_pRender->xBegin(ZDrawDebugRender::PRIMTYPE::PT_TRIANGLES);
        // TODO: Finish me
        m_pRender->xEnd();
    }

    void ZDrawDebugText::DrawLine(float fStartX, float fStartY, float fEndX, float fEndY, uint32_t lColor)
    {
        ZASSERT(m_pRender);

        m_pRender->xSetTexture(0u);
        m_pRender->xSetDrawMode(0x60001u);
        m_pRender->xColor(lColor);
        m_pRender->xBegin(ZDrawDebugRender::PRIMTYPE::PT_LINES);
        // TODO: Finish me
        m_pRender->xEnd();
    }
}
