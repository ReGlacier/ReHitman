#include <Glacier/Input/ZMouseWintel.h>
#include <Glacier/Input/ZDirectInput.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ResourceCollection.h>

#include <cstring>

#include <Windows.h>
#include <dinput.h>


namespace Glacier
{
#ifndef DIDFT_OPTIONAL
    static constexpr DWORD DIDFT_OPTIONAL = 0x80000000;
#endif

    static DIOBJECTDATAFORMAT s_MouseObjectDataFormat[] =
    {
        { &GUID_XAxis, offsetof(SMouseData, lX),            DIDFT_RELAXIS | DIDFT_ANYINSTANCE, 0 },
        { &GUID_YAxis, offsetof(SMouseData, lY),            DIDFT_RELAXIS | DIDFT_ANYINSTANCE, 0 },
        { &GUID_ZAxis, offsetof(SMouseData, lZ),            DIDFT_RELAXIS | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[0]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[1]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[2]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[3]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[4]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[5]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[6]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, 0 },
        { nullptr,     offsetof(SMouseData, rgbButtons[7]), DIDFT_PSHBUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, 0 },
    };

    static DIDATAFORMAT g_MouseDataFormat =
    {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_RELAXIS,
        sizeof(SMouseData),
        11,
        s_MouseObjectDataFormat
    };

    const char* GetMouseKeyName(ZInputDevice::CtrlInfo* pInfo, int id)
    {
        if (!pInfo || !pInfo->name)
        {
            return nullptr;
        }

        while (pInfo->nr != id)
        {
            ++pInfo;

            if (!pInfo->name)
            {
                return nullptr;
            }
        }

        return pInfo->name;
    }

    ZMouseWintel::ZMouseWintel(IDirectInput8A* pDI, GUID guidInstance, const char* instanceName, const char* productName)
        : ZMouseDevice(10, 3)
    {
        m_pDI = pDI;
        m_pDevice = nullptr;
        m_bAcquired = 0;
        m_instanceName[0] = 0;
        m_productName[0] = 0;
        strncpy(m_instanceName, instanceName, 0x104u);
        strncpy(m_productName, productName, 0x104u);
        m_instanceName[259] = 0;
        m_productName[259] = 0;
        m_guidInstance.Data1 = guidInstance.Data1;
        m_guidInstance = guidInstance;
        m_diginf = g_MouseDigitalControls;
        m_anainf = g_MouseAnalogControls;
    }

    ZMouseWintel::~ZMouseWintel() = default;

    const char* ZMouseWintel::DigitalSystemName(int id)
    {
        const auto* psKeyName = GetMouseKeyName(m_diginf, id);
        return g_pEngineData->m_pLocaleResources->GetResourceText("AllLevels/Interface/Mouse", psKeyName);
    }

    bool ZMouseWintel::Initialize()
    {
        m_pDevice = nullptr;

        return 
            SUCCEEDED(m_pDI->CreateDevice(m_guidInstance, &m_pDevice, nullptr)) &&
            SUCCEEDED(m_pDevice->SetDataFormat(&g_MouseDataFormat)) &&
            SUCCEEDED(m_pDevice->SetCooperativeLevel(static_cast<HWND>(g_pSysInterface->MainhWnd), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
    }

    bool ZMouseWintel::Terminate()
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

    void ZMouseWintel::Update()
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
            Acquire(10);
        }

        std::memset(&m_mouseData, 0, sizeof(m_mouseData));
        if (FAILED(m_pDevice->GetDeviceState(sizeof(m_mouseData), &m_mouseData)))
        {
            m_bAcquired = false;
            return;
        }

        for (int i = 0; i < 8; ++i)
        {
            buttonSample(i, (m_mouseData.rgbButtons[i] >> 7) != 0, g_pSysInterface->m_fRealTime);
        }

        AnalogMotionSample(0, static_cast<float>(m_mouseData.lX));
        AnalogMotionSample(1, static_cast<float>(m_mouseData.lY));
        AnalogMotionSample(2, static_cast<float>(m_mouseData.lZ));

        if (m_mouseData.lZ > 0)
        {
            buttonSample(8, true, g_pSysInterface->m_fRealTime);
            buttonSample(8, false, g_pSysInterface->m_fRealTime);
        }

        if (m_mouseData.lZ < 0)
        {
            buttonSample(9, true, g_pSysInterface->m_fRealTime);
            buttonSample(9, false, g_pSysInterface->m_fRealTime);
        }
    }

    int ZMouseWintel::Acquire()
    {
        if (!m_pDevice)
        {
            return 0;
        }

        int retries = 0;
        while (retries >= 0)
        {
            const HRESULT hr = m_pDevice->Acquire();
            --retries;

            if (hr == DI_OK || hr == S_FALSE)
            {
                m_bAcquired = true;
                return 1;
            }
        }

        return 0;
    }

    int ZMouseWintel::Acquire(int retries)
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

    int ZMouseWintel::Unacquire()
    {
        if (!m_pDevice) return 0;

        m_pDevice->Unacquire();
        m_bAcquired = false;
        
        return 1;
    }

    static ZInputDevice::CtrlInfo s_MouseDigitalControls[] = {
        { .name = "mb1", .nr = 0 },
        { .name = "left", .nr = 0 },
        { .name = "1", .nr = 0 },
        { .name = "mb2", .nr = 1 },
        { .name = "right", .nr = 1 },
        { .name = "2", .nr = 1 },
        { .name = "mb3", .nr = 2 },
        { .name = "middle", .nr = 2 },
        { .name = "3", .nr = 2 },
        { .name = "mb4", .nr = 3 },
        { .name = "4", .nr = 3 },
        { .name = "mb5", .nr = 4 },
        { .name = "5", .nr = 4 },
        { .name = "mb6", .nr = 5 },
        { .name = "6", .nr = 5 },
        { .name = "mb7", .nr = 6 },
        { .name = "7", .nr = 6 },
        { .name = "mb8", .nr = 7 },
        { .name = "8", .nr = 7 },
        { .name = "wheelup", .nr = 8 },
        { .name = "wheeldn", .nr = 9 },
        { .name = "wheeldown", .nr = 9 },
        {} //<<< REQUIRED
    };

    static ZInputDevice::CtrlInfo s_MouseAnalogControls[] = {
        { .name = "x", .nr = 0 },
        { .name = "y", .nr = 1 },
        { .name = "z", .nr = 2 },
        { .name = "wheel", .nr = 2 }, 
        {} //<<< REQUIRED
    };

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZInputDevice::CtrlInfo*, g_MouseDigitalControls, 0x0075EF80, &s_MouseDigitalControls[0]);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZInputDevice::CtrlInfo*, g_MouseAnalogControls, 0x0075F038, &s_MouseAnalogControls[0]);
}
