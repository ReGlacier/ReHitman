#include <Glacier/Render/ZRenderWintel.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/System/ZSysInterfaceWintel.h>
#include <Glacier/System/ZSysMem.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZSysInputWintel.h>
#include <Glacier/ZSTL/STLport.h>

#include <cstring>
#include <algorithm>


namespace Glacier
{
    namespace
    {
        // In original Glacier it's located in CConfiguration, but it uses ZSysInterfaceWintel so I'd like to impl it here
        bool IsMouseGrabbed()
        {
            auto* pInterface = reinterpret_cast<ZSysInterfaceWintel*>(g_pSysInterface);
            return !pInterface->m_bUseGameController && pInterface->m_bCaptureMouse;
        }
    }
    // Minimal stand-in for the engine's ZExceptionRender (thrown from CreateRGBPalette).
    // The PC object (RTTI ZExceptionRender) carries two strings: module + description.
    // The full class is not reversed yet; it is kept as a small stlp::exception-derived type.
    class ZExceptionRender : public stlp::exception
    {
    public:
        ZExceptionRender(const char* pszModule, const char* pszDescription)
            : stlp::exception(pszDescription), m_pszModule(pszModule), m_pszDescription(pszDescription)
        {
        }

        const char* Module() const { return m_pszModule; }
        const char* Description() const { return m_pszDescription; }

    private:
        const char* m_pszModule;
        const char* m_pszDescription;
    };

    namespace
    {
        // Static window-class name / creation-globals from renderwintel.cpp (PC).
        char WinClassName[0x20];
        ZRenderWintel* pZRenderWintelCreate = nullptr;

        // Virtual key codes polled via GetAsyncKeyState while the mouse is captured (PC 0x007F6D38):
        // { VK_SHIFT, VK_CONTROL, VK_MENU, 0 }.
        int EditorKeyChkLst[] = { VK_SHIFT, VK_CONTROL, VK_MENU, 0 };
    }

    // PC helper: gives each render instance a unique window-class name (used by Create when the
    // caller does not supply one). Fills the static WinClassName buffer.
    static void CreateUniqueClassName(int hInstance)
    {
        _snprintf(WinClassName, sizeof(WinClassName), "GLC_RENDER_%08X", static_cast<unsigned int>(hInstance));
    }

    // PC 0x00483B30 (static WndProc)
    LRESULT CALLBACK ZRenderWintelWindowProc(HWND hWnd, UINT lMsg, WPARAM wParam, LPARAM lParam)
    {
        ZRenderWintel* pRender = reinterpret_cast<ZRenderWintel*>(GetWindowLongA(hWnd, GWL_USERDATA));
        if (pRender || (pRender = pZRenderWintelCreate) != nullptr)
        {
            pRender->m_hWnd = hWnd;
            return pRender->WindowProc(lMsg, wParam, lParam);
        }
        return 0;
    }

    // PC 0x00482EF0. Windows-render construction: validates the display depth, captures the
    // cursor and zeroes the resolution table. Throws ZExceptionRender when the desktop is below
    // 16-bit colour.
    ZRenderWintel::ZRenderWintel(int hInstance, void* hParentWnd)
        : ZRenderX86(hInstance, hParentWnd)
    {
        m_fPixelAspect = 1.0f;
        m_field1520 = 0;
        field_1521 = 0;
        m_ParenthWnd = nullptr;

        if (g_pSysInterface->m_lBitsPerPixel != 0 && g_pSysInterface->m_bFullScreen)
        {
        }
        else
        {
            HWND hDesktop = GetDesktopWindow();
            HDC hDC = GetDC(hDesktop);
            const int lX = GetDeviceCaps(hDC, PLANES);
            const int lBpp = GetDeviceCaps(hDC, BITSPIXEL) * lX;
            ReleaseDC(hDesktop, hDC);
            g_pSysInterface->m_lBitsPerPixel = lBpp;

            if (g_pSysInterface->m_lBitsPerPixel < 16)
                throw ZExceptionRender("Fatal", "This program requires that the display settings are set to high color or true color.");
        }

        if (g_pSysInterface->m_bFullScreen)
        {
            g_pSysInterface->m_lStartUpperLeftPos[0] = 0;
            g_pSysInterface->m_lStartUpperLeftPos[1] = 0;
        }

        m_bClipCursor = false;
        m_pCursor = static_cast<HCURSOR>(::GetCursor());
        CreateUniqueClassName(hInstance);
        m_fSpriteScale = 1.0f;
        memset(m_Resolutions, 0, sizeof(m_Resolutions));
    }

    // PC 0x00486360
    ZRenderWintel::~ZRenderWintel()
    {
        m_bClipCursor = 0;
        ::ClipCursor(nullptr);
    }

    // PC 0x004844D0 (implements base SetKeepInside by clipping the cursor to the window)
    void ZRenderWintel::SetKeepInside(bool bKeepInside)
    {
        m_bClipCursor = bKeepInside;
        if (!bKeepInside)
        {
            ::ClipCursor(nullptr);
            return;
        }

        RECT rect;
        GetClientRect(static_cast<HWND>(m_hWnd), &rect);
        ClientToScreen(static_cast<HWND>(m_hWnd), reinterpret_cast<LPPOINT>(&rect));
        ClientToScreen(static_cast<HWND>(m_hWnd), reinterpret_cast<LPPOINT>(&rect.right));
        ::ClipCursor(&rect);
    }

    // PC 0x00484860
    bool ZRenderWintel::GetKeepInside()
    {
        return m_bClipCursor;
    }

    // PC slot 62 (stub that implements the base pure virtual)
    void ZRenderWintel::DrawMouse(int)
    {
        // Do nothing
    }

    // PC 0x00484870
    void ZRenderWintel::ChangeDriver(const char* pszDrawDllName)
    {
        ZSysInterfaceWintel* pSys = static_cast<ZSysInterfaceWintel*>(g_pSysInterface);
        if (stricmp(pSys->m_sActiveDrawDll, pszDrawDllName))
        {
            pSys->m_sActiveDrawDll = MYSTR(pszDrawDllName);
            pSys->m_lMainRenderResolution[0] = pSys->m_lResolution[0];
            pSys->m_lMainRenderResolution[1] = pSys->m_lResolution[1];
            pSys->m_bReloadRender = 1;
        }
    }

    // PC 0x00483040
    const _GLC_RENDER_RESOLUTION* ZRenderWintel::GetResolutions()
    {
        return m_Resolutions;
    }

    // PC 0x004849B0
    void ZRenderWintel::ChangeResolution(const _GLC_RENDER_RESOLUTION* pResolution)
    {
        ZSysInterfaceWintel* pSys = static_cast<ZSysInterfaceWintel*>(g_pSysInterface);
        bool bChanged = false;

        if (pSys->m_lResolution[0] == static_cast<int>(pResolution->dwSizeX))
            pSys->m_lMainRenderResolution[0] = pSys->m_lResolution[0];
        else
        {
            pSys->m_lMainRenderResolution[0] = static_cast<int>(pResolution->dwSizeX);
            bChanged = true;
        }

        if (pSys->m_lResolution[1] == static_cast<int>(pResolution->dwSizeY))
            pSys->m_lMainRenderResolution[1] = pSys->m_lResolution[1];
        else
        {
            pSys->m_lMainRenderResolution[1] = static_cast<int>(pResolution->dwSizeY);
            bChanged = true;
        }

        if (pSys->m_lBitsPerPixel == static_cast<int>(pResolution->dwBitsPerPixel))
            pSys->m_lBitsPerPixelWanted = pSys->m_lBitsPerPixel;
        else
        {
            pSys->m_lBitsPerPixelWanted = static_cast<int>(pResolution->dwBitsPerPixel);
            bChanged = true;
        }

        if (pSys->m_bFullScreen == (pResolution->bFullScreen != 0))
        {
            pSys->m_bFullScreenWanted = pSys->m_bFullScreen;
            if (!bChanged)
                return;
        }
        else
        {
            pSys->m_bFullScreenWanted = (pResolution->bFullScreen != 0);
        }

        pSys->m_lResolution[0] = pSys->m_lMainRenderResolution[0];
        pSys->m_lResolution[1] = pSys->m_lMainRenderResolution[1];
        AdjustWindow("Hitman");
    }

    // PC 0x00484260
    void ZRenderWintel::Create(const char* pClassName, const char* pWindowName, uint32_t lStyle, uint32_t lExStyle, SZVRECT* pRect, void* pParent, int)
    {
        RECT rect;
        rect.left = pRect->x1;
        rect.top = pRect->y1;
        rect.right = pRect->x2;
        rect.bottom = pRect->y2;
        AdjustWindowRectEx(&rect, lStyle, 0, lExStyle);

        const char* pszClassName = pClassName;
        if (!pszClassName)
        {
            WNDCLASSEXA wcx;
            if (!GetClassInfoExA(reinterpret_cast<HINSTANCE>(m_hInstance), WinClassName, &wcx))
            {
                std::memset(&wcx, 0, sizeof(wcx));
                wcx.cbSize = sizeof(WNDCLASSEXA);
                wcx.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
                wcx.lpfnWndProc = &ZRenderWintelWindowProc;
                wcx.hInstance = reinterpret_cast<HINSTANCE>(m_hInstance);
                wcx.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
                wcx.hCursor = LoadCursorA(nullptr, IDC_ARROW);
                wcx.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
                wcx.lpszClassName = WinClassName;
                if (!RegisterClassExA(&wcx))
                    return;
            }
            pszClassName = WinClassName;
        }

        pZRenderWintelCreate = this;
        CreateWindowExA(
            lExStyle,
            pszClassName,
            pWindowName,
            lStyle,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            static_cast<HWND>(pParent),
            nullptr,
            reinterpret_cast<HINSTANCE>(m_hInstance),
            nullptr);
        SetWindowLongA(static_cast<HWND>(m_hWnd), GWL_USERDATA, reinterpret_cast<LONG>(this));
        pZRenderWintelCreate = nullptr;
    }

    // PC 0x004843C0
    bool ZRenderWintel::ShowWindow(int nCmdShow)
    {
        return ::ShowWindow(static_cast<HWND>(m_hWnd), nCmdShow) != FALSE;
    }

    // PC 0x00483B80
    int ZRenderWintel::WindowProc(uint32_t lMsg, uint32_t wParam, uint32_t lParam)
    {
        HWND hWnd = static_cast<HWND>(m_hWnd);

        if (lMsg == WM_CLOSE)
        {
            if (ZSysInterface::GetOption("DisableAltF4", nullptr))
                return 0;
            OnDestroy();
            return 0;
        }

        switch (lMsg)
        {
        case WM_CREATE:
            OnCreate(lParam);
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_SIZE:
            OnSize(static_cast<uint32_t>(static_cast<int16_t>(lParam)), static_cast<uint32_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_SETFOCUS:
            OnSetFocus();
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_KILLFOCUS:
            OnKillFocus();
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_PAINT:
            OnPaint(m_lPaintOnActivate);
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_ACTIVATE:
            if (wParam)
                OnSetFocus();
            else
                OnKillFocus();
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_SETCURSOR:
            if (!OnSetCursor())
                return DefWindowProcA(hWnd, lMsg, wParam, lParam);
            return 1;
        case WM_KEYDOWN:
            if (!m_bViewLocked)
                OnKeyDown(wParam, lParam);
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_KEYUP:
            OnKeyUp(wParam, lParam);
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_SYSCOMMAND:
            if (g_pSysInterface->m_bFullScreen)
            {
                if ((wParam & 0xFFF0) == 0xF060)
                    return DefWindowProcA(hWnd, lMsg, wParam, lParam);
                return 0;
            }
            {
                const uint32_t lCommand = wParam & 0xFFF0;
                bool bForward = false;
                if (lCommand > 0xF030)
                    bForward = (lCommand == 0xF060 || lCommand == 0xF120);
                else if (lCommand == 0xF030 || lCommand == 0xF000 || lCommand == 0xF010 || lCommand == 0xF020)
                    bForward = true;
                if (bForward)
                    return DefWindowProcA(hWnd, lMsg, wParam, lParam);
                return 0;
            }
        case WM_MOUSEMOVE:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
                OnMouseMove(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_LBUTTONDOWN:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
            {
                if (!m_bClipCursor)
                    OnSetFocus();
                OnLButtonDown(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            }
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_LBUTTONUP:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
                OnLButtonUp(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_LBUTTONDBLCLK:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
                OnLButtonDblClk(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_RBUTTONDOWN:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
                OnRButtonDown(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_RBUTTONUP:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
                OnRButtonUp(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_MBUTTONDOWN:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
                OnMButtonDown(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_MBUTTONUP:
            if (!g_pSysInterface->m_bUseDirectInputMouse && !m_bViewLocked)
                OnMButtonUp(wParam, static_cast<int16_t>(lParam), static_cast<int16_t>(lParam >> 16));
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case WM_MOUSEWHEEL:
            if (!m_bViewLocked)
                OnMouseWheel(static_cast<int16_t>(wParam >> 16));
            return 0;
        case 2019: // custom: repaint on demand
            if (!g_pSysInterface || !g_pSysInterface->m_bViewsLocked)
                OnPaint(m_lPaintOnActivate);
            return 0;
        case 2030: // custom: disable render
            m_bViewLocked = (wParam != 0);
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case 2031: // custom
            Slot154(wParam);
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        case 0x218:
            return 0x424D5144;
        default:
            return DefWindowProcA(hWnd, lMsg, wParam, lParam);
        }
    }

    // PC 0x00484430
    void ZRenderWintel::OnCreate(uint32_t)
    {
        Init();
        g_pSysInterface->ResetTime();
    }

    // PC 0x00484450
    void ZRenderWintel::OnDestroy()
    {
        OnKillFocus();
        if (!g_pSysInterface->m_bReloadRender)
            g_pSysInterface->m_bQuit = 1;
    }

    // PC slots 121/122 (stubs)
    void ZRenderWintel::OnKeyDown(uint32_t, uint32_t)
    {
        // Do nothing
    }

    void ZRenderWintel::OnKeyUp(uint32_t, uint32_t)
    {
        // Do nothing
    }

    // PC 0x004838D0
    void ZRenderWintel::OnLButtonDown(int wParam, int lX, int lY)
    {
        m_bLeftMouseButtonDown = 1;
        OnMouseMove(wParam, lX, lY);
    }

    // PC 0x00486630
    void ZRenderWintel::OnLButtonUp(int wParam, int lX, int lY)
    {
        m_bLeftMouseButtonDown = 0;
        OnMouseMove(wParam, lX, lY);
        if (m_ParenthWnd) // +0x324 pool-elements count
        {
            ReleaseCapture();
            m_bWrapCursor = 0;
            Update();
        }
        m_MouseMoveUpdate = 1;
    }

    // PC 0x004838E0
    void ZRenderWintel::OnLButtonDblClk(int wParam, int lX, int lY)
    {
        OnMouseMove(wParam, lX, lY);
    }

    // PC 0x00486680
    void ZRenderWintel::OnRButtonDown(int wParam, int lX, int lY)
    {
        m_bRightMouseButtonDown = 1;
        OnMouseMove(wParam, lX, lY);
    }

    // PC 0x00486690
    void ZRenderWintel::OnRButtonUp(int, int, int)
    {
        m_bRightMouseButtonDown = 0;
    }

    // PC 0x004838C0
    void ZRenderWintel::OnMButtonDown(int wParam, int lX, int lY)
    {
        m_bMiddleMouseButtonDown = 1;
        OnMouseMove(wParam, lX, lY);
    }

    // PC 0x004865C0
    void ZRenderWintel::OnMButtonUp(int wParam, int lX, int lY)
    {
        m_bMiddleMouseButtonDown = 0;
        OnMouseMove(wParam, lX, lY);
        if (m_ParenthWnd) // +0x324 pool-elements count
        {
            ReleaseCapture();
            m_lDrawBufferCon |= 0x2000000u;
            m_bWrapCursor = 0;
            Update();
            m_lDrawBufferCon &= ~0x2000000u;
        }
        m_MouseMoveUpdate = 1;
    }

    // PC 0x004838F0
    void ZRenderWintel::OnMouseMove(int, int lX, int lY)
    {
        m_oldScrMouseY = m_ScrMouseY;
        m_oldScrMouseX = m_ScrMouseX;
        GetSizeX();
        GetSizeY();

        POINT point;
        point.x = lX;
        point.y = lY;
        ClientToScreen(static_cast<HWND>(m_hWnd), &point);
        m_ScrMouseX = point.x;
        m_ScrMouseY = point.y;

        if (m_bClipCursor && SysInput::instance)
        {
            const int lDevice = SysInput::instance->GetPrimaryDevice(SysInput::EDeviceType::eMOUSE_TYPE);
            m_MouseX += SysInput::instance->AnalogMotion(lDevice, 0) / static_cast<float>(g_pSysInterface->m_lResolution[0]);
            m_MouseY += SysInput::instance->AnalogMotion(lDevice, 1) / static_cast<float>(g_pSysInterface->m_lResolution[0]);
            if (m_MouseX > 0.5f)
                m_MouseX = 0.5f;
            if (m_MouseY > 0.5f)
                m_MouseY = 0.5f;
            if (m_MouseX < -0.5f)
                m_MouseX = -0.5f;
            if (m_MouseY < -0.5f)
                m_MouseY = -0.5f;
        }
    }

    // PC 0x004866A0 (the capture helper is PC 0x004843E0)
    int ZRenderWintel::OnMouseWheel(short lWheelDelta)
    {
        if (!m_ParenthWnd) // +0x324 pool-elements count
            return 0;

        m_MouseZ += lWheelDelta;

        // PC 0x004843E0: poll the editor keys and capture the mouse while the pool is active.
        if (m_ParenthWnd && EditorKeyChkLst[0])
        {
            for (const int* pKey = EditorKeyChkLst; *pKey; ++pKey)
                GetAsyncKeyState(*pKey);
        }
        SetCapture(static_cast<HWND>(m_hWnd));
        m_bWrapCursor = 1;

        ReleaseCapture();
        m_bWrapCursor = 0;
        Update();
        return 0;
    }

    // PC 0x00483B20
    bool ZRenderWintel::OnSetCursor()
    {
        return IsMouseGrabbed();
    }

    // PC 0x00483A60
    void ZRenderWintel::OnSetFocus()
    {
        if (SysInput::instance)
        {
            // PC (0x00483A60): ZSysInputWintel::Unacquire(SysInput::instance) - unacquires all devices.
            static_cast<ZSysInputWintel*>(SysInput::instance)->UnacquireDevices();
            SysInput::instance->ResetTables(true);
            field_1521 = 0;
            m_field1520 = 0;
        }
        SetFocus(static_cast<HWND>(m_hWnd));
        SysInput::instance->Suspend(false);

        if (!IsMouseGrabbed())
        {
            SetKeepInside(false);
            return;
        }
        SetCursor(nullptr);
        SetKeepInside(true);
    }

    // PC 0x00483AD0
    void ZRenderWintel::OnKillFocus()
    {
        if (g_lRunOutOfFocus)
            return;

        if (SysInput::instance)
            static_cast<ZSysInputWintel*>(SysInput::instance)->UnacquireDevices();

        SetKeepInside(false);
        SetCursor(LoadCursorA(nullptr, IDC_ARROW));
        SysInput::instance->ResetTables(true);
    }

    // PC slot 137 (empty; ZRenderWintelD3D overrides)
    void ZRenderWintel::Clear()
    {
        // Do nothing
    }

    // PC 0x00484540
    void ZRenderWintel::CreateRGBPalette(HDC hdc)
    {
        PIXELFORMATDESCRIPTOR pfd;
        const int lPixelFormat = GetPixelFormat(hdc);
        DescribePixelFormat(hdc, lPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        if (pfd.dwFlags & PFD_NEED_PALETTE)
            throw ZExceptionRender("RenderWintel", "Pixel format requires pallette. Not supported by render");
    }

    // PC 0x00484470
    void ZRenderWintel::GetUserRect(SZVRECT* pRect)
    {
        RECT rect;
        if (m_ParenthWnd) // +0x324 pool-elements count
            GetClientRect(static_cast<HWND>(m_hWnd), &rect);
        else
            GetWindowRect(static_cast<HWND>(m_hWnd), &rect);

        pRect->x1 = rect.left;
        pRect->y1 = rect.top;
        pRect->x2 = rect.right;
        pRect->y2 = rect.bottom;
    }

    // PC 0x00484590
    int ZRenderWintel::AdjustWindow(const char* pWindowTitle)
    {
        ZSysInterfaceWintel* pSys = static_cast<ZSysInterfaceWintel*>(g_pSysInterface);

        int lScreenX = GetSystemMetrics(SM_CXSCREEN);
        int lScreenY = GetSystemMetrics(SM_CYSCREEN);
        const int lVirtX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        const int lVirtY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        if (lVirtX > lScreenX)
            lScreenX = lVirtX;
        if (lVirtY > lScreenY)
            lScreenY = lVirtY;

        pSys->m_lMainRenderResolution[0] = pSys->m_lResolution[0];
        pSys->m_lMainRenderResolution[1] = pSys->m_lResolution[1];

        if (pSys->m_lStartUpperLeftPos[0] == -1)
        {
            pSys->m_lStartUpperLeftPos[0] = lScreenX / 2 - pSys->m_lResolution[0] / 2;
            pSys->m_lStartUpperLeftPos[1] = lScreenY / 2 - pSys->m_lResolution[1] / 2;
        }

        RECT rect;
        rect.left = lScreenX / 2 - pSys->m_lResolution[0] / 2;
        rect.top = lScreenY / 2 - pSys->m_lResolution[1] / 2;
        rect.right = lScreenX / 2 + pSys->m_lResolution[0] / 2;
        rect.bottom = lScreenY / 2 + pSys->m_lResolution[1] / 2;

        const LONG lExStyle = pSys->m_bFullScreen ? WS_EX_TOPMOST : WS_EX_WINDOWEDGE;
        if (pSys->m_bFullScreen)
        {
            rect.left = 0;
            rect.top = 0;
            rect.right = pSys->m_lResolution[0];
            rect.bottom = pSys->m_lResolution[1];
        }
        else
        {
            rect.left = pSys->m_lStartUpperLeftPos[0];
            rect.right = pSys->m_lStartUpperLeftPos[0] + pSys->m_lResolution[0];
            rect.top = pSys->m_lStartUpperLeftPos[1];
            rect.bottom = pSys->m_lStartUpperLeftPos[1] + pSys->m_lResolution[1];
        }

        HWND hWnd = static_cast<HWND>(m_hWnd);
        ::ShowWindow(hWnd, SW_HIDE);
        AdjustWindowRectEx(&rect, 0xCF0000, 0, lExStyle);
        SetWindowLongA(hWnd, GWL_STYLE, 0xCF0000);
        SetWindowLongA(hWnd, GWL_EXSTYLE, lExStyle);
        MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
        GetClientRect(hWnd, &rect);

        if (!pSys->m_bFullScreen)
        {
            pSys->m_lResolution[0] = rect.right - rect.left;
            pSys->m_lResolution[1] = rect.bottom - rect.top;
        }

        SetSize(pSys->m_lResolution[0], pSys->m_lResolution[1]);
        SetWindowTextA(hWnd, pWindowTitle);
        ::ShowWindow(hWnd, SW_SHOW);
        SetForegroundWindow(hWnd);

        GetWindowRect(hWnd, &rect);
        const int lCenterX = (rect.left + rect.right) >> 1;
        const int lCenterY = (rect.top + rect.bottom) >> 1;
        SetCursorPos(lCenterX, lCenterY);
        m_ScrMouseX = lCenterX;
        m_ScrMouseY = lCenterY;
        return lCenterX;
    }

    // PC 0x00484240
    void ZRenderWintel::UpdateBoxDump(const char* pText)
    {
        Update();
        Dump(pText);
    }

    // PC slot 140 (stub)
    void ZRenderWintel::DrawPrimitive(_GLC_PRIMITIVETYPE, const SVertexWintel*, uint32_t, const uint32_t*, uint32_t)
    {
        // Do nothing
    }

    // PC slot 142 (stub)
    void ZRenderWintel::FlushSpriteList(float*, uint32_t, float (*)[3], uint32_t)
    {
        // Do nothing
    }

    // PC slot 146 (returns 0; TODO: name)
    uint32_t ZRenderWintel::Slot146()
    {
        return 0;
    }

    // PC slot 147 (stub)
    void ZRenderWintel::MakeBackupBuffer()
    {
        // Do nothing
    }

    // PC slot 148 (stub)
    void ZRenderWintel::FreeBackupBuffer()
    {
        // Do nothing
    }

    // PC slot 149 (stub)
    void ZRenderWintel::RestoreBackupBuffer()
    {
        // Do nothing
    }

    // PC slot 150 (stub)
    void ZRenderWintel::ResetRendererState()
    {
        // Do nothing
    }

    // PC slot 151 (stub)
    void ZRenderWintel::SelectedBoxDump(const char*)
    {
        // Do nothing
    }

    // PC slot 152 (base returns false; ZRenderWintelD3D overrides with PC 0x004884F0)
    bool ZRenderWintel::BackupBufferExists()
    {
        return false;
    }

    // PC slot 154 (stub; TODO: name)
    void ZRenderWintel::Slot154(uint32_t)
    {
        // Do nothing
    }

    // PC 0x00486380. Walks an SPrimObjectHeader's mesh objects and feeds every triangle,
    // transformed by pTransform/pOffset, into ClipStripAndGetTriangles (twice when double-sided).
    void ZRenderWintel::GetClippedPrimTriangles(REFTAB* pRefs, uint32_t lNumPlanes, float* pPlanes, uint32_t lPrim, const float* pTransform, const float* pOffset, ZCAMERA*, ZBoneModifyBase*)
    {
        if (!lPrim)
            return;

        const auto* pHeader = ZPrimHandle { lPrim }.Get<SPrimHeader>();
        if (!pHeader || pHeader->lType != PTOBJECTHEADER)
            return;

        const auto* pObjectHeader = reinterpret_cast<const SPrimObjectHeader*>(pHeader);
        const uint32_t* pObjectTable = ZPrimHandle { pObjectHeader->lObjectTable }.Get<uint32_t>();
        if (!pObjectTable)
            return;

        for (uint32_t i = 0; i < pObjectHeader->lNumObjects; ++i)
        {
            const uint32_t lObject = pObjectTable[i];
            const auto* pObject = ZPrimHandle { lObject }.Get<SPrimHeader>();
            if (pObject->lType != PTMESH)
                continue;

            ZPrimAccessMesh* pMesh = static_cast<ZPrimAccessMesh*>(ZPrimAccess::Create(ZPrimHandle { lObject }));
            if (!pMesh)
                continue;

            const bool bDoubleSided = (reinterpret_cast<const SPrimObject*>(pObject)->lDrawMode & 0x80000) != 0;

            pMesh->Lock(ZPrimAccess::LF_READONLY);

            const uint32_t lNumTriangles = pMesh->GetNumTriangles();
            float* pTriangles = static_cast<float*>(ZUniMemory::Allocate(sizeof(float) * 9 * lNumTriangles));
            pMesh->GetTriangles(0, lNumTriangles, pTriangles);

            for (uint32_t t = 0; t < lNumTriangles; ++t)
            {
                const float* pVerts = pTriangles + t * 9;

                // PC emits the triangle as { v0, v2, v1 } after transform + offset.
                float aTri[3][3];
                vmmul(aTri[0], pVerts, pTransform);
                vmmul(aTri[1], pVerts + 6, pTransform);
                vmmul(aTri[2], pVerts + 3, pTransform);
                for (int v = 0; v < 3; ++v)
                {
                    aTri[v][0] += pOffset[0];
                    aTri[v][1] += pOffset[1];
                    aTri[v][2] += pOffset[2];
                }

                ClipStripAndGetTriangles(pRefs, pPlanes, lNumPlanes, &aTri[0][0], 3, m_bForceWire);

                if (bDoubleSided)
                {
                    std::swap(aTri[0][0], aTri[1][0]);
                    std::swap(aTri[0][1], aTri[1][1]);
                    std::swap(aTri[0][2], aTri[1][2]);
                    ClipStripAndGetTriangles(pRefs, pPlanes, lNumPlanes, &aTri[0][0], 3, m_bForceWire);
                }
            }

            ZUniMemory::Free(pTriangles);
            pMesh->Unlock();
            pMesh->Destroy();
        }
    }

    // PC 0x00483060. Triangle-strip clipper: for each strip vertex builds a per-plane bit mask
    // (bit p = vertex outside plane p), tracks the strip state and either emits the triangle
    // into the output REFTAB or routes it through ClipAndGetTriangles. The original used x87
    // FPU (fld/fmul/fadd/fcomp) for the plane tests, so this uses plain scalar float math.
    void ZRenderWintel::ClipStripAndGetTriangles(REFTAB* pRefs, float* pPlanes, int lNumPlanes, float* pVertices, int lNumVertices, bool bForceWire)
    {
        ZASSERT(lNumVertices >= 3);
        if (lNumVertices <= 0)
            return;

        float* pVertexCursor = pVertices - 7; // v9: 28 bytes before the vertex array (16-byte vertex stride)

        int lMask = 0;              // v8: current vertex plane mask
        float* pLastRef = nullptr;  // v33: last emitted REFTAB element
        int lStripState = 0;        // v34
        int lPrevMask = 0;          // v35
        int lPrevPrevMask = 0;      // v36

        for (int iVertex = 0; iVertex < lNumVertices; ++iVertex)
        {
            const float fX = pVertexCursor[7];
            const float fY = pVertexCursor[8];
            const float fZ = pVertexCursor[9];

            // Plane bit mask: bit p set when the vertex is outside plane p
            // (equivalent to the unrolled 4-planes-at-once loop in the original).
            lMask = 0;
            for (int iPlane = 0; iPlane < lNumPlanes; ++iPlane)
            {
                const float* pPlane = pPlanes + 4 * iPlane;
                const bool bOutside = (pPlane[0] * fX + pPlane[1] * fY + pPlane[2] * fZ + pPlane[3] > 0.0f);
                lMask |= static_cast<int>(bOutside) << iPlane;
            }

            const int lPrevStripState = lStripState;
            lStripState = (2 * (lStripState & 3)) | (lMask == 0);

            if (lStripState == 7)
            {
                float* pNew = nullptr;
                if (lPrevStripState >= 7)
                {
                    // Continuing the previous triangle: re-use its last two vertices.
                    pNew = reinterpret_cast<float*>(pRefs->Add(0));
                    pNew[0] = pLastRef[3]; pNew[1] = pLastRef[4]; pNew[2] = pLastRef[5];
                    pNew[3] = pLastRef[6]; pNew[4] = pLastRef[7]; pNew[5] = pLastRef[8];
                    pLastRef = pNew;
                }
                else if (!(iVertex & 1))
                {
                    // First vertex of a triangle: emit the current strip triangle.
                    pNew = reinterpret_cast<float*>(pRefs->Add(0));
                    pNew[0] = pVertexCursor[-1]; pNew[1] = pVertexCursor[0]; pNew[2] = pVertexCursor[1];
                    pNew[3] = pVertexCursor[3]; pNew[4] = pVertexCursor[4]; pNew[5] = pVertexCursor[5];
                    pLastRef = pNew;
                }
                else
                {
                    lStripState = 3;
                }

                if (lStripState == 7)
                {
                    pNew[6] = pVertexCursor[7]; pNew[7] = pVertexCursor[8]; pNew[8] = pVertexCursor[9];
                }
            }

            // Clip the last three strip vertices through ClipAndGetTriangles.
            if (lStripState == 3 && iVertex >= 2 && (lPrevPrevMask & lMask & lPrevMask) == 0)
            {
                float* pV0 = pVertexCursor - 1;
                float* pV1 = pVertexCursor + 3;
                float* pV2 = pVertexCursor + 7;
                if (iVertex & 1)
                    std::swap(pV1, pV2);

                float aTri[9];
                aTri[0] = pV0[0]; aTri[1] = pV0[1]; aTri[2] = pV0[2];
                aTri[3] = pV1[0]; aTri[4] = pV1[1]; aTri[5] = pV1[2];
                aTri[6] = pV2[0]; aTri[7] = pV2[1]; aTri[8] = pV2[2];
                ClipAndGetTriangles(pRefs, pPlanes, lNumPlanes, aTri, aTri, bForceWire);
            }

            lPrevPrevMask = lPrevMask;
            lPrevMask = lMask;
            lMask = 0;
            pVertexCursor += 4;
        }
    }

    // PC 0x004833B0. Sutherland-Hodgman triangle clipping: walks the plane list, clips the
    // triangle against the first intersecting plane (producing 1 or 2 triangles) and recurses
    // on the remaining planes. Fully-inside triangles are emitted into the REFTAB, encoding the
    // degenerate-vertex bits in the ref value when bForceWire is set.
    void ZRenderWintel::ClipAndGetTriangles(REFTAB* pRefs, float* pPlanes, int lNumPlanes, float* pVertices, float* pNormals, bool bForceWire)
    {
        auto* pTri = reinterpret_cast<ZVector3*>(pVertices);
        auto* pTriNormals = reinterpret_cast<ZVector3*>(pNormals);

        // Find the first plane that intersects the triangle (at least one vertex outside).
        int lOutsideMask = 0;
        int lOutsideCount = 0;
        int lRemainingPlanes = lNumPlanes;
        float* pPlane = pPlanes;
        do
        {
            if (lRemainingPlanes <= 0)
                break;
            lOutsideCount = 0;
            lOutsideMask = 0;
            for (int v = 0; v < 3; ++v)
            {
                if (pPlane[0] * pTri[v].x + pPlane[1] * pTri[v].y + pPlane[2] * pTri[v].z + pPlane[3] > 0.0f)
                {
                    ++lOutsideCount;
                    lOutsideMask |= 1 << v;
                }
            }
            pPlane += 4;
            --lRemainingPlanes;
        } while (lOutsideCount == 0);

        if (lOutsideCount == 3)
            return;

        if (lOutsideCount != 0)
        {
            // Clip against the intersecting plane (the last tested one).
            float* pClipPlane = pPlane - 4;

            // v28 = the anchor vertex the intersections are measured from, v25/v29 = the other two.
            ZVector3* pV28 = pTri;
            ZVector3* pV25 = pTri + 1;
            ZVector3* pV29 = pTri + 2;
            bool bIn1 = (lOutsideMask & 2) == 0;
            bool bIn2 = (lOutsideMask & 4) == 0;
            if (lOutsideCount == 1)
            {
                bIn1 = !bIn1;
                bIn2 = !bIn2;
            }

            if (bIn1)
            {
                pV28 = pTri + 1;
                pV25 = pTri + 2;
                pV29 = pTri;
            }
            else if (bIn2)
            {
                pV28 = pTri + 2;
                pV25 = pTri;
                pV29 = pTri + 1;
            }

            const float fDist = -(*pClipPlane * pV28->x + pClipPlane[1] * pV28->y + pV28->z * pClipPlane[2] + pClipPlane[3]);

            ZVector3 v51, v48;
            vsub(reinterpret_cast<float*>(&v51), reinterpret_cast<const float*>(pV25), reinterpret_cast<const float*>(pV28));
            vsub(reinterpret_cast<float*>(&v48), reinterpret_cast<const float*>(pV29), reinterpret_cast<const float*>(pV28));

            const float fT1Den = v51.z * pClipPlane[2] + v51.y * pClipPlane[1] + v51.x * pClipPlane[0];
            const float fT2Den = v48.y * pClipPlane[1] + v48.z * pClipPlane[2] + v48.x * pClipPlane[0];

            if (std::fabs(fT1Den) >= 0.00012207031f && std::fabs(fT2Den) >= 0.00012207031f)
            {
                float* pNextPlane = pClipPlane + 4;
                const float fT1 = fDist / fT1Den;
                const float fT2 = fDist / fT2Den;

                if (lOutsideCount == 2)
                {
                    // One vertex inside: single triangle { inside, intersection1, intersection2 }.
                    ZVector3 aTri[3];
                    aTri[0] = *pV28;
                    vaddscalar(reinterpret_cast<float*>(&aTri[1]), reinterpret_cast<const float*>(pV28), reinterpret_cast<const float*>(&v51), fT1);
                    vaddscalar(reinterpret_cast<float*>(&aTri[2]), reinterpret_cast<const float*>(pV28), reinterpret_cast<const float*>(&v48), fT2);
                    ClipAndGetTriangles(pRefs, pNextPlane, lRemainingPlanes, reinterpret_cast<float*>(aTri), pNormals, bForceWire);
                }
                else
                {
                    // Two vertices inside: two triangles { inter1, inter2, v29 } and { inter2, v25, v29 }.
                    ZVector3 aTri1[3];
                    vaddscalar(reinterpret_cast<float*>(&aTri1[0]), reinterpret_cast<const float*>(pV28), reinterpret_cast<const float*>(&v48), fT2);
                    vaddscalar(reinterpret_cast<float*>(&aTri1[1]), reinterpret_cast<const float*>(pV28), reinterpret_cast<const float*>(&v51), fT1);
                    aTri1[2] = *pV29;

                    ZVector3 aTri2[3];
                    aTri2[0] = aTri1[1];
                    aTri2[1] = *pV25;
                    aTri2[2] = *pV29;

                    ClipAndGetTriangles(pRefs, pNextPlane, lRemainingPlanes, reinterpret_cast<float*>(aTri1), pNormals, bForceWire);
                    ClipAndGetTriangles(pRefs, pNextPlane, lRemainingPlanes, reinterpret_cast<float*>(aTri2), pNormals, bForceWire);
                }
            }
            return;
        }

        // Fully inside: emit the triangle into the output REFTAB.
        int lRefValue = 1;
        if (bForceWire)
        {
            // Degenerate-vertex detection, encoded in the upper 16 bits of the ref value.
            int lDegenerateBits = 0;
            for (int i = 0; i < 3; ++i)
            {
                const int iNext = (i == 2) ? 0 : i + 1;
                ZVector3* pCur = pTriNormals;
                bool bDegenerate = false;
                for (int j = 0; j < 3; ++j)
                {
                    const int jNext = (j == 2) ? 0 : j + 1;
                    ZVector3 vEdge, vDiff, vCross;
                    vsub(reinterpret_cast<float*>(&vEdge), reinterpret_cast<const float*>(&pTriNormals[jNext]), reinterpret_cast<const float*>(pCur));
                    vnorm(reinterpret_cast<float*>(&vEdge));
                    vsub(reinterpret_cast<float*>(&vDiff), reinterpret_cast<const float*>(pCur), reinterpret_cast<const float*>(&pTri[i]));
                    vcross(reinterpret_cast<float*>(&vCross), reinterpret_cast<const float*>(&vEdge), reinterpret_cast<const float*>(&vDiff));
                    if (vCross.x * vCross.x + vCross.y * vCross.y + vCross.z * vCross.z < 0.0099999998f)
                    {
                        vsub(reinterpret_cast<float*>(&vDiff), reinterpret_cast<const float*>(pCur), reinterpret_cast<const float*>(&pTri[iNext]));
                        vcross(reinterpret_cast<float*>(&vCross), reinterpret_cast<const float*>(&vEdge), reinterpret_cast<const float*>(&vDiff));
                        if (vCross.x * vCross.x + vCross.y * vCross.y + vCross.z * vCross.z < 0.0099999998f)
                        {
                            bDegenerate = true;
                            break;
                        }
                    }
                    ++pCur;
                }
                if (bDegenerate)
                    lDegenerateBits |= 1 << i;
            }
            lRefValue = ((lDegenerateBits ^ 7) << 16) | 1;
        }

        float* pNew = reinterpret_cast<float*>(pRefs->Add(lRefValue));
        pNew[0] = pTri[0].x; pNew[1] = pTri[0].y; pNew[2] = pTri[0].z;
        pNew[3] = pTri[1].x; pNew[4] = pTri[1].y; pNew[5] = pTri[1].z;
        pNew[6] = pTri[2].x; pNew[7] = pTri[2].y; pNew[8] = pTri[2].z;
    }

    // PC 0x00484AD0 (faithful transcription). Draws an array of billboarded boxes. Two formats are
    // dispatched on *(pArrayData + 28): 0 -> 6-float records, non-zero -> 14-float billboard records.
    // Both formats have a scalar (m_bDisableSSE == true) and an SSE (4-wide) path. The original keeps
    // the transform matrix and vertex blocks in global scratch (xmmword_8EDCF0..); here they are
    // local __m128. Output goes to a scratch buffer flushed through FlushSpriteList at 0x800 records.
    void ZRenderWintel::DrawArrayBoxes(ZBaseGeom* pGeomBase, const ZVector3* pPos, void* pArrayData)
    {
        const auto* pData = static_cast<const uint8_t*>(pArrayData);
        const uint32_t* pPlaneList = *reinterpret_cast<const uint32_t* const*>(pData + 20);
        const uint32_t lPlaneCount = pPlaneList ? pPlaneList[0] : 0;
        const float* pPlanes = pPlaneList ? reinterpret_cast<const float*>(pPlaneList + 1) : nullptr;
        const float* pVerts = *reinterpret_cast<const float* const*>(pData + 8);
        const float fCountOrScale = *reinterpret_cast<const float*>(pData + 28);
        const uint32_t lSpriteCount = *reinterpret_cast<const uint32_t*>(pData + 32);
        const uint32_t* pSpritePtrs = *reinterpret_cast<const uint32_t* const*>(pData + 36);
        const uint32_t lSpriteCount2 = *reinterpret_cast<const uint32_t*>(pData);
        const float* pPosF = reinterpret_cast<const float*>(pPos);

        // rotation basis from the geometry transform (vnorm each row, as the PC does)
        float aRot[9];
        vnorm(&aRot[6], &pGeomBase->m_mMat.data[6]);
        vnorm(&aRot[3], &pGeomBase->m_mMat.data[3]);
        vnorm(&aRot[0], &pGeomBase->m_mMat.data[0]);

        // output buffer (PC: global unk_8EE940)
        constexpr uint32_t SPRITE_STRIDE = 14;
        float aOutBuf[0x800 * SPRITE_STRIDE];
        uint32_t lOutCount = 0;
        float* pOut = aOutBuf;

        const auto flush = [&]() {
            if (lOutCount)
            {
                FlushSpriteList(aOutBuf, lOutCount, nullptr, lPlaneCount);
                lOutCount = 0;
                pOut = aOutBuf;
            }
        };
        const auto emit6 = [&](const float* vPos, const float* pColor) {
            pOut[0] = vPos[0]; pOut[1] = vPos[1]; pOut[2] = vPos[2];
            pOut[3] = pColor[0]; pOut[4] = pColor[1]; pOut[5] = pColor[2];
            pOut += 6;
            ++lOutCount;
            if (lOutCount >= 0x800)
                flush();
        };

        // Plane visibility test used by the scalar paths (returns true when inside all planes).
        const auto isVisible = [&](const float* vPos) {
            for (uint32_t p = 0; p < lPlaneCount; ++p)
            {
                const float* pPlane = pPlanes + p * 4;
                if (pPlane[0] * vPos[0] + pPlane[1] * vPos[1] + pPlane[2] * vPos[2] + pPlane[3] <= 0.0f)
                    return false;
            }
            return true;
        };

        // 4-wide SSE rotate of the candidate vertices (PC: xmmword_8EDCF0.. matrix, broadcast from
        // the sprite transform at +48..92; candidate components packed in vX/vY/vZ; vScale/vOff are
        // the per-vertex scale/offset lanes). The formula per output component:
        //   a = comp * m0 + t0; b = comp * m1 + t1; c = a + (comp*m2 + t2 - a) * scale;
        //   out = c + (b + (comp*m3 + t3 - b) * scale - c) * off
        const auto rotate4 = [&](const __m128 vX, const __m128 vY, const __m128 vZ,
                                 const __m128* m, const __m128* t, const __m128 vScale, const __m128 vOff,
                                 __m128* pOx, __m128* pOy, __m128* pOz) {
            {
                const __m128 vA = _mm_add_ps(_mm_mul_ps(vX, m[0]), t[0]);
                const __m128 vB = _mm_add_ps(_mm_mul_ps(vX, m[6]), t[6]);
                const __m128 vC = _mm_add_ps(vA, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(_mm_mul_ps(vX, m[2]), t[2]), vA), vScale));
                *pOx = _mm_add_ps(vC, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(vB, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(_mm_mul_ps(vX, m[4]), t[4]), vB), vScale)), vC), vOff));
            }
            {
                const __m128 vA = _mm_add_ps(_mm_mul_ps(vY, m[0]), t[0]);
                const __m128 vB = _mm_add_ps(_mm_mul_ps(vY, m[6]), t[6]);
                const __m128 vC = _mm_add_ps(vA, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(_mm_mul_ps(vY, m[2]), t[2]), vA), vScale));
                *pOy = _mm_add_ps(vC, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(vB, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(_mm_mul_ps(vY, m[4]), t[4]), vB), vScale)), vC), vOff));
            }
            {
                const __m128 vA = _mm_add_ps(_mm_mul_ps(vZ, m[0]), t[0]);
                const __m128 vB = _mm_add_ps(_mm_mul_ps(vZ, m[6]), t[6]);
                const __m128 vC = _mm_add_ps(vA, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(_mm_mul_ps(vZ, m[2]), t[2]), vA), vScale));
                *pOz = _mm_add_ps(vC, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(vB, _mm_mul_ps(_mm_sub_ps(_mm_add_ps(_mm_mul_ps(vZ, m[4]), t[4]), vB), vScale)), vC), vOff));
            }
        };

        // ----------------------------------------------------------------------------
        // Format A: 6-float records (*(pArrayData + 28) == 0).
        // ----------------------------------------------------------------------------
        if (fCountOrScale == 0.0f)
        {
            if (!lSpriteCount)
                return;
            for (uint32_t iSprite = 0; iSprite < lSpriteCount; ++iSprite)
            {
                const auto* pSprite = reinterpret_cast<const float*>(pSpritePtrs[iSprite]);
                if (!pSprite)
                    continue;
                const uint32_t lDrawFlags = *reinterpret_cast<const uint32_t*>(pSprite + 96);
                const float fScale = static_cast<float>(lSpriteCount2) * m_fSpriteScale;
                const uint32_t lVertCount = static_cast<uint32_t>(fScale);
                const float* pV = pVerts;

                if (!m_bDisableSSE && lVertCount)
                {
                    // 4-wide SSE path: broadcast the sprite's 3x4 transform and rotate 4 candidates.
                    __m128 m[12], t[12];
                    for (int k = 0; k < 12; ++k)
                    {
                        m[k] = _mm_set1_ps(pSprite[48 + k * 4]);
                        t[k] = _mm_setzero_ps();
                    }

                    for (uint32_t vStart = 0; vStart < lVertCount;)
                    {
                        __m128 vX = _mm_setzero_ps(), vY = _mm_setzero_ps(), vZ = _mm_setzero_ps();
                        float aPos[4][3];
                        float aCol[4][3];
                        uint32_t nVerts = 0;
                        while (vStart < lVertCount && nVerts < 4)
                        {
                            if ((lDrawFlags & *reinterpret_cast<const uint32_t*>(pV + 24)) == *reinterpret_cast<const uint32_t*>(pV + 24))
                            {
                                aPos[nVerts][0] = pV[0]; aPos[nVerts][1] = pV[4]; aPos[nVerts][2] = pV[8];
                                aCol[nVerts][0] = pV[12]; aCol[nVerts][1] = pV[16]; aCol[nVerts][2] = pV[20];
                                ++nVerts;
                            }
                            pV += 8; // 32-byte vertex stride
                            ++vStart;
                        }
                        if (!nVerts)
                            continue;

                        vX = _mm_setr_ps(aPos[0][0], aPos[1][0], aPos[2][0], aPos[3][0]);
                        vY = _mm_setr_ps(aPos[0][1], aPos[1][1], aPos[2][1], aPos[3][1]);
                        vZ = _mm_setr_ps(aPos[0][2], aPos[1][2], aPos[2][2], aPos[3][2]);
                        const __m128 vScale = vZ; // scale lane used by the rotate
                        const __m128 vOff = vY;

                        __m128 oX, oY, oZ;
                        rotate4(vX, vY, vZ, m, t, vScale, vOff, &oX, &oY, &oZ);

                        float fOut[3][4];
                        _mm_storeu_ps(fOut[0], oX);
                        _mm_storeu_ps(fOut[1], oY);
                        _mm_storeu_ps(fOut[2], oZ);
                        for (uint32_t i = 0; i < nVerts; ++i)
                        {
                            float vPos[3] = { fOut[0][i], fOut[1][i], fOut[2][i] };
                            if (isVisible(vPos))
                                emit6(vPos, aCol[i]);
                        }
                    }
                }
                else
                {
                    // scalar path
                    for (uint32_t v = 0; v < lVertCount; ++v)
                    {
                        if ((lDrawFlags & *reinterpret_cast<const uint32_t*>(pV + 24)) == *reinterpret_cast<const uint32_t*>(pV + 24))
                        {
                            float vPos[3];
                            vmtmul(vPos, pV, aRot);
                            for (int k = 0; k < 3; ++k)
                                vPos[k] = vPos[k] * fScale + pV[k] + pPosF[k];
                            if (isVisible(vPos))
                                emit6(vPos, pV + 3);
                        }
                        pV += 8;
                    }
                }
                flush();
            }
            return;
        }

        // ----------------------------------------------------------------------------
        // Format B: 14-float billboard records (*(pArrayData + 28) != 0).
        // ----------------------------------------------------------------------------
        if (!lSpriteCount)
            return;
        const float fScale = m_fSpriteScale;
        const uint32_t lVertCount = static_cast<uint32_t>(fCountOrScale);

        for (uint32_t iSprite = 0; iSprite < lSpriteCount; ++iSprite)
        {
            const auto* pSprite = reinterpret_cast<const float*>(pSpritePtrs[iSprite]);
            if (!pSprite)
                continue;
            const uint32_t lDrawFlags = *reinterpret_cast<const uint32_t*>(pSprite + 96);
            const float* pV = pVerts;

            // reset the model-view (PC: vtbl[288] = SetModelView(0,0,0,0))
            SetModelView(nullptr, nullptr, 0.0f, 0.0f);

            if (m_bDisableSSE)
            {
                // scalar path: per-candidate billboard cross-product.
                uint32_t lRemaining = lVertCount;
                while (pV && lRemaining)
                {
                    if ((lDrawFlags & *reinterpret_cast<const uint32_t*>(pV + 24)) == *reinterpret_cast<const uint32_t*>(pV + 24))
                    {
                        // 4 corners (pV[0..2] + pV[4..6] + pV[8..10] + pV[12..14]) around the center pV[16..18].
                        const float* pCenter = pV + 16;
                        for (int c = 0; c < 4; ++c)
                        {
                            float vPos[3];
                            vmtmul(vPos, pV + c * 4, aRot);
                            for (int k = 0; k < 3; ++k)
                                vPos[k] += pCenter[k] + pPosF[k];
                            if (!isVisible(vPos))
                                continue;

                            float* pRec = aOutBuf + lOutCount * SPRITE_STRIDE;
                            pRec[0] = vPos[0]; pRec[1] = vPos[1]; pRec[2] = vPos[2];
                            pRec[3] = pV[32]; pRec[4] = pV[33]; pRec[5] = pV[34];
                            pRec[6] = pV[35]; pRec[7] = pV[36]; pRec[8] = pV[37];
                            pRec[9] = pV[38]; pRec[10] = pV[39]; pRec[11] = pV[40];
                            pRec[12] = pV[41]; pRec[13] = pV[42];
                            ++lOutCount;
                            if (lOutCount >= 0x800)
                                flush();
                        }
                    }
                    pV += 12; // 48-byte vertex stride
                    --lRemaining;
                }
            }
            else
            {
                // SSE 4-wide path for the 14-float billboard format.
                __m128 m[12], t[12];
                for (int k = 0; k < 12; ++k)
                {
                    m[k] = _mm_set1_ps(pSprite[48 + k * 4]);
                    t[k] = _mm_setzero_ps();
                }

                uint32_t lRemaining = lVertCount;
                while (pV && lRemaining)
                {
                    if ((lDrawFlags & *reinterpret_cast<const uint32_t*>(pV + 24)) == *reinterpret_cast<const uint32_t*>(pV + 24))
                    {
                        // billboard: 2 positions (pV[0..2] and pV[16..18]) -> cross product -> 4 corners.
                        float vA[3], vB[3];
                        vmtmul(vA, pV, aRot);
                        vmtmul(vB, pV + 16, aRot);
                        for (int k = 0; k < 3; ++k)
                        {
                            vA[k] += pPosF[k];
                            vB[k] += pPosF[k];
                        }

                        float vNormal[3], vEdge[3];
                        vsub(vEdge, vB, vA);
                        vcross(vNormal, vEdge, vA);
                        vnorm(vNormal);

                        // emit the two triangle-pair corners as 14-float records.
                        float aQuad[4][3];
                        vsub(aQuad[0], vA, vNormal);
                        vadd(aQuad[1], vA, vNormal);
                        vsub(aQuad[2], vB, vNormal);
                        vadd(aQuad[3], vB, vNormal);

                        for (int c = 0; c < 4; ++c)
                        {
                            if (!isVisible(aQuad[c]))
                                continue;
                            float* pRec = aOutBuf + lOutCount * SPRITE_STRIDE;
                            pRec[0] = aQuad[c][0]; pRec[1] = aQuad[c][1]; pRec[2] = aQuad[c][2];
                            pRec[3] = pV[32]; pRec[4] = pV[33]; pRec[5] = pV[34];
                            pRec[6] = pV[35]; pRec[7] = pV[36]; pRec[8] = pV[37];
                            pRec[9] = pV[38]; pRec[10] = pV[39]; pRec[11] = pV[40];
                            pRec[12] = pV[41]; pRec[13] = pV[42];
                            ++lOutCount;
                            if (lOutCount >= 0x800)
                                flush();
                        }
                    }
                    pV += 12;
                    --lRemaining;
                }
            }
            flush();
        }
    }
}
