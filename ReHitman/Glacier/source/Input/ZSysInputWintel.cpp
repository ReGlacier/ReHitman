#include <Glacier/Input/ZSysInputWintel.h>
#include <Glacier/Input/ZGameControlWintel.h>
#include <Glacier/Input/ZInputDevice.h>
#include <Glacier/Input/ZKeyboardWintel.h>
#include <Glacier/Input/ZMouseWintel.h>
#include <Glacier/Input/ZDirectInput.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    static bool InitDirectInput()
    {
        if (g_pDI)
        {
            return true;
        }

        if (DirectInput8Create(
            static_cast<HINSTANCE>(g_pSysInterface->m_hInstance),
            DIRECTINPUT_VERSION,
            IID_IDirectInput8A,
            reinterpret_cast<void**>(&g_pDI),
            nullptr) >= 0)
        {
            return true;
        }

        g_pDI = nullptr;
        return false;

    }

    ZSysInputWintel::ZSysInputWintel()
        : ZSysInput()
    {
        if (g_pDI || InitDirectInput())
        {
            g_pDI->EnumDevices(
                DI8DEVCLASS_ALL,
                ZSysInputWintel::EnumDevicesCallback,
                this,
                0
            );
        }

        for (int i = 0; i < m_devicecount; ++i)
        {
            m_devices[i]->Update();
        }

        ActivateDevices();
    }

    ZSysInputWintel::~ZSysInputWintel()
    {
        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i])
            {
                m_devices[i]->Acquire();
            }
        }
    }

    int ZSysInputWintel::Update()
    {
        ZSysInput::Update();
        return 1;
    }

    BOOL CALLBACK ZSysInputWintel::EnumDevicesCallback(const DIDEVICEINSTANCEA* pDIDeviceInstance, void* pG1Device)
    {
        auto* pSysInput = static_cast<ZSysInputWintel*>(pG1Device);
        ZInputDevice* pDevice = nullptr;

        const DWORD deviceType = pDIDeviceInstance->dwDevType;
        if (deviceType & 0x10000)
        {
            const BYTE hidSubtype = static_cast<BYTE>((deviceType >> 8) & 0xFF);
            if (hidSubtype == 1 || hidSubtype == 2)
            {
                pDevice = ZUniMemory::New<ZGameControlWintel>(
                    g_pDI,
                    pDIDeviceInstance->guidInstance,
                    pDIDeviceInstance->tszInstanceName,
                    pDIDeviceInstance->tszProductName);
            }
            else
            {
                return DIENUM_CONTINUE;
            }
        }
        else
        {
            switch (static_cast<BYTE>(deviceType))
            {
            case 18:
                pDevice = ZUniMemory::New<ZMouseWintel>(
                    g_pDI,
                    pDIDeviceInstance->guidInstance,
                    pDIDeviceInstance->tszInstanceName,
                    pDIDeviceInstance->tszProductName);
                break;

            case 19:
                pDevice = ZUniMemory::New<ZKeyboardWintel>(
                    g_pDI,
                    pDIDeviceInstance->guidInstance,
                    pDIDeviceInstance->tszInstanceName,
                    pDIDeviceInstance->tszProductName);
                break;

            case 21:
                pDevice = ZUniMemory::New<ZGameControlWintel>(
                    g_pDI,
                    pDIDeviceInstance->guidInstance,
                    pDIDeviceInstance->tszInstanceName,
                    pDIDeviceInstance->tszProductName);
                break;

            default:
                return DIENUM_CONTINUE;
            }
        }

        if (pDevice)
        {
            pSysInput->AddDevice(pDevice);
        }

        return DIENUM_CONTINUE;
    }

    void ZSysInputWintel::AcquireDevices()
    {
        for (int i = 0; i < m_devicecount; ++i)
        {
            m_devices[i]->Acquire();
        }
    }
    
    void ZSysInputWintel::UnacquireDevices()
    {
        for (int i = 0; i < m_devicecount; ++i)
        {
            m_devices[i]->Unacquire();
        }
    }

    void ZSysInputWintel::SetActiveController(int controlledIndex)
    {
        // Do nothing
    }

    uint8_t ZSysInputWintel::SetTimeoutCallback(SysInput::TimeoutCallback callback, int timeout)
    {
        // Do nothing
        return 0;
    }

    void ZSysInputWintel::UnSetTimeoutCallback()
    {
        // Do nothing
    }

    bool ZSysInputWintel::IsControllerConnected()
    {
        // Do nothing
        return false;
    }

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IDirectInput8A*, g_pDI, 0x008B4BA0, nullptr);
}
