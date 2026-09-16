#include <Glacier/Render/Debug/ZDrawDebugRender.h>
#include <Glacier/Render/Debug/ZDrawDebugText.h>
#include <Glacier/Render/Debug/ZDrawDebugTimer.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/View/IView.h>
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
        , m_lSelectedIndex(0)
        , m_pActiveMenu(nullptr)
        , m_pRender(nullptr)
    {
        m_Menus.Clear();
        g_pDrawDebugText = this;
    }

    void ZDrawDebugText::Update(ZDrawDebugRender* pRender)
    {
        if (g_pDrawDebugText != this)
            return;

        m_pRender = pRender;
        if (!m_pRender)
            return;

        m_pRender->xReset();
        Update();
        m_pRender->xFlush();
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
        sNewMenu.m_pData = pData;
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
    }

    void ZDrawDebugText::End()
    {
        // Do nothing
    }

    void ZDrawDebugText::Update()
    {
        DrawInfo();

        for (int i = 0; i < m_Menus.Count(); ++i)
        {
            if (auto* pMenu = m_Menus.Get(i); pMenu && pMenu->m_pMenu)
            {
                pMenu->m_pMenu->DrawAlways();
            }
        }

        if (m_pActiveMenu && m_pActiveMenu->m_pMenu)
            m_pActiveMenu->m_pMenu->Update();
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

        SetPosSizeText(1, 1, 42, 8);
        Fill(0x80202080u);
        DrawFocus();

        ZRender* pRender = nullptr;
        if (m_pRender->m_pIView)
            pRender = const_cast<IView*>(m_pRender->m_pIView)->Render();

        if (!pRender || !g_pSysInterface)
            return;

        ZDebugFrame sFrame { pFrame };
        sFrame.Plot(0, 0, "FPS: %d  Frame: %.2f ms", pRender->m_FPS,
            g_pSysInterface->DeltaFrameTime * 1000.0f);
        sFrame.Plot(0, 1, "Triangles: %u  Primitives: %u", pRender->m_lTriangleCount,
            pRender->m_lPrimitiveCount);
        sFrame.Plot(0, 2, "Textures: %u  Bones: %u", pRender->m_lTextureCount,
            pRender->m_lBoneCount);
        sFrame.Plot(0, 3, "Sprites: %u  SubPrims: %u", pRender->m_lSpriteCount,
            pRender->m_lSubPrimitiveCount);
        sFrame.Plot(0, 4, "Tex memory: %u  Draw mode: %u", pRender->m_lTextureSize,
            g_RenderDebugMenu.m_lDrawModeIndex);
        sFrame.Plot(0, 5, "Camera mode: %u  Timer: %u", g_RenderDebugMenu.m_lCameraModeIndex,
            g_pDrawDebugTimer ? g_pDrawDebugTimer->m_lTimerType : TIMER_NONE);
    }

    void ZDrawDebugText::DrawText(uint32_t x, uint32_t y, const char* pszText, uint32_t lColor)
    {
        ZASSERT(m_pRender);

        const auto viewport = m_pRender->Viewport();
        const ZVector3 position {
            static_cast<float>(viewport.x + x),
            static_cast<float>(viewport.y + viewport.h - y),
            0.0f,
        };
        m_pRender->xDrawText(position, pszText, lColor, 0);
    }

    void ZDrawDebugText::DrawRect(uint32_t lPosX, int lPosY, int lSizeX, int lSizeY, uint32_t lColor)
    {
        ZASSERT(m_pRender);

        m_pRender->xSetTexture(0u);
        m_pRender->xSetDrawMode(0x60001u);
        m_pRender->xColor(lColor);
        m_pRender->xBegin(ZDrawDebugRender::PRIMTYPE::PT_TRIANGLES);
        m_pRender->xVertex2(static_cast<float>(lPosX), static_cast<float>(lPosY));
        m_pRender->xVertex2(static_cast<float>(lPosX + lSizeX), static_cast<float>(lPosY));
        m_pRender->xVertex2(static_cast<float>(lPosX + lSizeX), static_cast<float>(lPosY + lSizeY));
        m_pRender->xVertex2(static_cast<float>(lPosX), static_cast<float>(lPosY));
        m_pRender->xVertex2(static_cast<float>(lPosX + lSizeX), static_cast<float>(lPosY + lSizeY));
        m_pRender->xVertex2(static_cast<float>(lPosX), static_cast<float>(lPosY + lSizeY));
        m_pRender->xEnd();
    }

    void ZDrawDebugText::DrawLine(float fStartX, float fStartY, float fEndX, float fEndY, uint32_t lColor)
    {
        ZASSERT(m_pRender);

        m_pRender->xSetTexture(0u);
        m_pRender->xSetDrawMode(0x60001u);
        m_pRender->xColor(lColor);
        m_pRender->xBegin(ZDrawDebugRender::PRIMTYPE::PT_LINES);
        m_pRender->xVertex2(fStartX, fStartY);
        m_pRender->xVertex2(fEndX, fEndY);
        m_pRender->xEnd();
    }
}
