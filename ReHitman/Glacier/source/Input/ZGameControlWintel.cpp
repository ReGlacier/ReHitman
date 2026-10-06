#include <Glacier/Input/ZGameControlWintel.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZInterface.h>
#include <Glacier/System/ZSysInterface.h>

#include <algorithm>
#include <cmath>
#include <cstring>


#ifndef DIDFT_OPTIONAL
static constexpr DWORD DIDFT_OPTIONAL = 0x80000000;
#endif


namespace Glacier
{
    static constexpr float kGameControlAxisScale = 0.001001001f;

    static DIOBJECTDATAFORMAT s_GameControlObjectDataFormat[0xA4];

    static DIDATAFORMAT g_GameControlDataFormat =
    {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_ABSAXIS,
        sizeof(DIJOYSTATE2),
        0xA4,
        s_GameControlObjectDataFormat
    };

    static void InitGameControlDataFormat()
    {
        static bool initialized = false;
        if (initialized)
        {
            return;
        }

        constexpr DWORD optionalAxis = DIDFT_AXIS | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL;
        constexpr DWORD optionalPushButton = DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL;
        constexpr DWORD optionalPov = DIDFT_POV | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL;
        constexpr DWORD aspectPosition = 0x00000100;
        constexpr DWORD aspectVelocity = 0x00000200;
        constexpr DWORD aspectAcceleration = 0x00000300;
        constexpr DWORD aspectForce = 0x00000400;

        const GUID* axisGuids[8] =
        {
            &GUID_XAxis,
            &GUID_YAxis,
            &GUID_ZAxis,
            &GUID_RxAxis,
            &GUID_RyAxis,
            &GUID_RzAxis,
            &GUID_Slider,
            &GUID_Slider,
        };

        int entry = 0;
        auto addAxisBlock = [&](DWORD baseOffset, DWORD aspect)
        {
            for (int i = 0; i < 8; ++i)
            {
                s_GameControlObjectDataFormat[entry++] =
                {
                    axisGuids[i],
                    baseOffset + static_cast<DWORD>(i * sizeof(LONG)),
                    optionalAxis,
                    aspect
                };
            }
        };

        addAxisBlock(offsetof(DIJOYSTATE2, lX), aspectPosition);

        for (int i = 0; i < 4; ++i)
        {
            s_GameControlObjectDataFormat[entry++] =
            {
                &GUID_POV,
                offsetof(DIJOYSTATE2, rgdwPOV) + static_cast<DWORD>(i * sizeof(DWORD)),
                optionalPov,
                0
            };
        }

        for (int i = 0; i < 128; ++i)
        {
            s_GameControlObjectDataFormat[entry++] =
            {
                nullptr,
                offsetof(DIJOYSTATE2, rgbButtons) + static_cast<DWORD>(i),
                optionalPushButton,
                0
            };
        }

        addAxisBlock(offsetof(DIJOYSTATE2, lVX), aspectVelocity);
        addAxisBlock(offsetof(DIJOYSTATE2, lAX), aspectAcceleration);
        addAxisBlock(offsetof(DIJOYSTATE2, lFX), aspectForce);

        initialized = true;
    }

    static ZInputDevice::CtrlInfo s_GameControlDigitalControls[] =
    {
        { "triangle", 0 },
        { "circle", 1 },
        { "cross", 2 },
        { "square", 3 },
        { "left2", 4 },
        { "right2", 5 },
        { "left1", 6 },
        { "right1", 7 },
        { "select", 8 },
        { "leftstick", 9 },
        { "rightstick", 10 },
        { "start", 11 },
        { "up", 12 },
        { "right", 13 },
        { "down", 14 },
        { "left", 15 },
        {}
    };

    static ZInputDevice::CtrlInfo s_GameControlAnalogControls[] =
    {
        { "leftx", 0 },
        { "lefty", 1 },
        { "rightx", 2 },
        { "righty", 3 },
        {}
    };

    static BOOL CALLBACK EnumGameControlObjectsCallback(const DIDEVICEOBJECTINSTANCEA* objectInstance, void* ref)
    {
        auto* device = static_cast<IDirectInputDevice8A*>(ref);

        DIPROPRANGE range{};
        range.diph.dwSize = sizeof(DIPROPRANGE);
        range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        range.diph.dwHow = DIPH_BYID;
        range.diph.dwObj = objectInstance->dwType;
        range.lMin = -999;
        range.lMax = 999;
        device->SetProperty(DIPROP_RANGE, &range.diph);

        return DIENUM_CONTINUE;
    }

    ZGameControlWintel::ZGameControlWintel(IDirectInput8A* pDI, GUID guidInstance, const char* instanceName, const char* productName)
        : ZGameControlDevice(16, 4)
    {
        m_pDI = nullptr;
        m_pDevice = nullptr;
        m_bAcquired = false;
        m_instanceName[0] = '\0';
        m_productName[0] = '\0';
        std::strncpy(m_instanceName, instanceName, sizeof(m_instanceName));
        std::strncpy(m_productName, productName, sizeof(m_productName));
        m_instanceName[sizeof(m_instanceName) - 1] = '\0';
        m_productName[sizeof(m_productName) - 1] = '\0';
        m_guidInstance = guidInstance;
        m_pDI = pDI;
        m_buttonCount = 0;
        m_axisCount = 0;
        std::memset(&m_wintelCaps, 0, sizeof(m_wintelCaps));
        m_diginf = s_GameControlDigitalControls;
        m_anainf = s_GameControlAnalogControls;
        m_fDeadZone = 0.3f;
    }

    ZGameControlWintel::~ZGameControlWintel()
    {
        Terminate();
    }

    const char* ZGameControlWintel::DigitalSystemName(int id)
    {
        return DigitalName(id);
    }

    bool ZGameControlWintel::Connected()
    {
        return m_pDevice != nullptr;
    }

    bool ZGameControlWintel::Initialize()
    {
        InitGameControlDataFormat();

        m_pDevice = nullptr;
        if (FAILED(m_pDI->CreateDevice(m_guidInstance, &m_pDevice, nullptr)) || !m_pDevice)
        {
            return false;
        }

        if (FAILED(m_pDevice->SetDataFormat(&g_GameControlDataFormat)))
        {
            return false;
        }

        if (FAILED(m_pDevice->SetCooperativeLevel(static_cast<HWND>(g_pSysInterface->MainhWnd), DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
        {
            return false;
        }

        m_wintelCaps.dwSize = 44;
        m_pDevice->GetCapabilities(reinterpret_cast<LPDIDEVCAPS>(&m_wintelCaps));

        m_axisCount = static_cast<int>(m_wintelCaps.dwAxes >> 1);
        if (m_axisCount > 2)
        {
            m_axisCount = 2;
        }

        if (FAILED(m_pDevice->EnumObjects(EnumGameControlObjectsCallback, m_pDevice, DIDFT_AXIS)))
        {
            return false;
        }

        m_buttonCount = static_cast<int>(m_wintelCaps.dwButtons);
        if (m_buttonCount > 16)
        {
            m_buttonCount = 16;
        }

        return true;
    }

    bool ZGameControlWintel::Terminate()
    {
        if (!m_pDevice)
        {
            return false;
        }

        m_pDevice->Unacquire();
        m_pDevice->Release();
        m_pDevice = nullptr;
        m_bAcquired = false;
        return true;
    }

    void ZGameControlWintel::Update()
    {
        if (m_updatetime != g_pSysInterface->m_fRealTime)
        {
            m_updatetime = g_pSysInterface->m_fRealTime;
        }

        if (!m_pDevice || !m_active)
        {
            return;
        }

        if (!m_bAcquired)
        {
            Acquire();
        }

        m_pDevice->Poll();

        DIJOYSTATE2 state{};
        if (FAILED(m_pDevice->GetDeviceState(sizeof(state), &state)))
        {
            m_bAcquired = false;
            return;
        }

        float leftX = static_cast<float>(state.lX) * kGameControlAxisScale;
        float leftY = static_cast<float>(state.lY) * kGameControlAxisScale;
        float rightX = static_cast<float>(state.lZ) * kGameControlAxisScale;
        float rightY = static_cast<float>(state.lRz) * kGameControlAxisScale;

        NormalizeStick(&leftX, &leftY);
        NormalizeStick(&rightX, &rightY);

        if (SysInput::instance && (SysInput::instance->m_iMode & 1) != 0)
        {
            std::swap(rightX, rightY);
            const BYTE button9 = state.rgbButtons[9];
            state.rgbButtons[9] = state.rgbButtons[10];
            state.rgbButtons[10] = state.rgbButtons[11];
            state.rgbButtons[11] = button9;
        }

        for (int i = 0; i < m_buttonCount; ++i)
        {
            buttonSample(i, state.rgbButtons[i] != 0, g_pSysInterface->m_fRealTime);
        }

        AnalogSample(0, leftX);
        AnalogSample(1, leftY);
        AnalogSample(2, rightX);
        AnalogSample(3, rightY);
    }

    int ZGameControlWintel::Acquire(int retries)
    {
        if (!m_pDevice)
        {
            return 0;
        }

        for (int i = retries; i >= 0; --i)
        {
            const HRESULT hr = m_pDevice->Acquire();
            if (hr == DI_OK || hr == S_FALSE)
            {
                m_bAcquired = true;
                return 1;
            }
        }

        return 0;
    }

    int ZGameControlWintel::Acquire()
    {
        return Acquire(0);
    }

    int ZGameControlWintel::Unacquire()
    {
        if (!m_pDevice)
        {
            return 0;
        }

        m_pDevice->Unacquire();
        m_bAcquired = false;
        return 1;
    }

    void ZGameControlWintel::NormalizeStick(float* x, float* y)
    {
        const float magnitude = std::sqrt((*x * *x) + (*y * *y));
        if (magnitude < m_fDeadZone)
        {
            *x = 0.0f;
            *y = 0.0f;
            return;
        }

        float scale = 1.0f;
        if (magnitude < 1.0f)
        {
            scale = (magnitude - m_fDeadZone) / ((1.0f - m_fDeadZone) * magnitude);
        }
        else
        {
            scale = 1.0f / magnitude;
        }

        *x *= scale;
        *y *= scale;
    }
}
