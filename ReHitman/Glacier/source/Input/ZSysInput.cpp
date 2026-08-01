#include <Glacier/Input/ZSysInput.h>
#include <Glacier/Input/ZInputDevice.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>


namespace Glacier
{
    ZSysInput::ZSysInput() = default;

    ZSysInput::~ZSysInput()
    {
        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i])
            {
                ZUniMemory::Delete(m_devices[i]);
                m_devices[i] = nullptr;
            }
        }

        m_devicecount = 0;
    }

    int ZSysInput::Update()
    {
        if (m_bSuspend)
            return 0;
        
        for (int i = 0; i < m_devicecount; ++i)
        {
            if (!m_devices[i])
                continue;

            m_devices[i]->Update();
        }
        
        return 1;
    }

    void ZSysInput::Suspend(bool bSuspend)
    {
        m_bSuspend = bSuspend;
    }

    void ZSysInput::ResetTables(bool bClearKeys)
    {
        if (!m_devicecount) 
            return;

        for (int i = 0; i < m_devicecount; ++i)
        {
            if (!m_devices[i])
                continue;

            m_devices[i]->ResetTables(bClearKeys);
        }
    }

    SysInput::ZDevice* ZSysInput::GetDevice(int id)
    {
        if (id == -1) return nullptr;

        ZASSERT(id < m_devicecount);
        return m_devices[id];
    }

    const char* ZSysInput::GetDeviceName(int id)
    {
        auto* pDevice = GetDevice(id);
        if (!pDevice)
        {
            return "Undefined";
        }

        static char szName[32] { '\0' };
        int deviceTypeIndex = 0;

        if (id >= 0)
        {
            for (int i = 0; i <= id; ++i)
            {
                if (m_devices[i]->Type() == pDevice->Type())
                {
                    ++deviceTypeIndex;
                }
            }
        }

        switch (pDevice->Type())
        {
            case SysInput::eMOUSE_TYPE:
                if (deviceTypeIndex == 1)
                {
                    return "ms";
                }
                return pDevice->DigitalSystemName(0);

            case SysInput::eKEYBOARD_TYPE:
                if (deviceTypeIndex == 1)
                {
                    return "kb";
                }
                return pDevice->DigitalSystemName(0);

            case SysInput::eGAMECONTROL_TYPE:
                if (deviceTypeIndex == 1)
                {
                    return "gc";
                }

                std::sprintf(szName, "gc%i", deviceTypeIndex - 1);
                return szName;

            default:
                return pDevice->DigitalSystemName(0);
        }
    }
    
    int ZSysInput::GetDeviceIdByName(const char* name)
    {
        if (!strcmp(name, "mouse") || !strcmp(name, "ms"))
            return GetPrimaryDevice(SysInput::EDeviceType::eMOUSE_TYPE);

        if (!strcmp(name, "keyboard") || !strcmp(name, "kb"))
            return GetPrimaryDevice(SysInput::EDeviceType::eKEYBOARD_TYPE);

        if (strncmp(name, "gc", 2u))
        {
            if (m_devicecount <= 0)
                return -1;

            for (int i = 0; i < m_devicecount; ++i)
            {
                const char* pszDevName = m_devices[i]->GetName();
                if (pszDevName && !strcmp(name, pszDevName))
                {
                    return i;
                }
            }

            return -1;
        }

        char indexBuffer[3]{};
        int indexLength = 0;
        const char* current = name + 2;

        while (*current)
        {
            if (*current < '0' || *current > '9')
            {
                return -1;
            }

            if (indexLength >= 2)
            {
                return -1;
            }

            indexBuffer[indexLength++] = *current++;
        }

        if (!indexLength)
        {
            return GetNthDevice(SysInput::EDeviceType::eGAMECONTROL_TYPE, 0);
        }

        return GetNthDevice(SysInput::EDeviceType::eGAMECONTROL_TYPE, std::atoi(indexBuffer));
    }
    
    int ZSysInput::GetNthDevice(SysInput::EDeviceType deviceType, int index)
    {
        if (m_devicecount <= 0)
        {
            return -1;
        }

        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i] && m_devices[i]->Type() == deviceType && --index < 0)
            {
                return i;
            }
        }

        return -1;
    }

    int ZSysInput::DeviceCount(SysInput::EDeviceType deviceType)
    {
        int result = 0;

        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i] && m_devices[i]->Type() == deviceType)
            {
                ++result;
            }
        }

        return result;
    }

    SysInput::EDeviceType ZSysInput::DeviceType(int deviceIndex)
    {
        if (auto* pDevice = GetDevice(deviceIndex))
        {
            return pDevice->Type();
        }

        printf("WARNING: ZSysInput::DeviceType(%d) will return -1 (BAD VALUE) due device not found\n", deviceIndex);
        return static_cast<SysInput::EDeviceType>(-1);
    }

    int ZSysInput::GetPrimaryDevice(SysInput::EDeviceType deviceType)
    {
        return GetNthDevice(deviceType, 0);
    }

    SysInput::ZDevice* ZSysInput::GetPrimaryDevicePtr(SysInput::EDeviceType deviceType)
    {
        return GetDevice(GetNthDevice(deviceType, 0));   
    }

    SysInput::ZDevice* ZSysInput::GetNthDevicePtr(SysInput::EDeviceType deviceType, int nth)
    {
        return GetDevice(GetNthDevice(deviceType, nth));
    }

    bool ZSysInput::MapDigital(const char* devname, const char* ctrlname, int* devid, int* ctrlid)
    {
        auto id = GetDeviceIdByName(devname);
        *devid = id;

        if (id == -1)
            return false;
        
        auto digitalId = m_devices[id]->DigitalId(ctrlname);
        *ctrlid = digitalId;
        return digitalId != -1;
    }

    bool ZSysInput::MapAnalog(const char* devname, const char* ctrlname, int* devid, int* ctrlid)
    {
        auto id = GetDeviceIdByName(devname);
        *devid = id;

        if (id == -1)
            return false;

        auto analogId = m_devices[id]->AnalogId(ctrlname);
        *ctrlid = analogId;
        return analogId != -1;
    }

    float ZSysInput::Analog(int devid, int ctrlid)
    {
        if (devid < 0 || ctrlid < 0 || !m_devices[devid])
            return 0.0f;

        return m_devices[devid]->AnalogState(ctrlid);
    }

    float ZSysInput::AnalogMotion(int devid, int ctrlid)
    {
        if (devid < 0 || ctrlid < 0 || !m_devices[devid])
            return 0.0f;

        return m_devices[devid]->AnalogMotion(ctrlid);
    }

    int ZSysInput::Digital(int devid, int ctrlid)
    {
        if (devid < 0 || ctrlid < 0 || !m_devices[devid])
            return 0;

        return m_devices[devid]->DigitalState(ctrlid);
    }

    void ZSysInput::DeactivateDevices()
    {
        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i])
            {
                m_devices[i]->Deactivate();
            }
        }
    }

    void ZSysInput::ActivateDevices()
    {
        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i])
            {
                m_devices[i]->Activate();
            }
        }
    }

    bool ZSysInput::InstallHandler(int deviceIndex, SysInput::DeviceHandler handler, void* userData)
    {
        auto* pDevice = reinterpret_cast<ZInputDevice*>(GetDevice(deviceIndex));
        if (!pDevice)
            return false;

        return pDevice->InstallHandler(handler, userData);
    }

    bool ZSysInput::UninstallHandler(int deviceIndex, SysInput::DeviceHandler handler)
    {
        auto* pDevice = reinterpret_cast<ZInputDevice*>(GetDevice(deviceIndex));
        if (!pDevice)
            return false;

        return pDevice->UninstallHandler(handler);
    }

    bool ZSysInput::OpenControllerMissingWindow()
    {
        // TODO: Finish me after ZGameData, ZXMLGUISystem and other hell reversed
        return false;
    }

    bool ZSysInput::CheckForKeyPress(int* device, int* controlid)
    {
        *device = -1;
        *controlid = -1;

        if (m_devicecount <= 0)
        {
            return false;
        }

        for (int deviceIndex = 0; deviceIndex < m_devicecount; ++deviceIndex)
        {
            SysInput::ZDevice* pDevice = GetDevice(deviceIndex);
            const int digitalCount = pDevice->DigitalCount();

            for (int controlIndex = 0; controlIndex < digitalCount; ++controlIndex)
            {
                if (pDevice->DigitalState(controlIndex))
                {
                    *device = deviceIndex;
                    *controlid = controlIndex;
                    return true;
                }

                const float timeThreshold = static_cast<float>(g_pSysInterface->m_fRealTime.secs - 102) * TIMETYPE::kInvTPS;
                if (static_cast<float>(pDevice->DigitalHist(controlIndex, 0)) > timeThreshold &&
                    static_cast<float>(pDevice->DigitalHist(controlIndex, 1)) > timeThreshold)
                {
                    *device = deviceIndex;
                    *controlid = controlIndex;
                    return true;
                }
            }
        }

        return false;
    }

    int ZSysInput::AddDevice(ZInputDevice* pDevice)
    {
        const int deviceIndex = m_devicecount;
        if (deviceIndex > 31)
        {
            return 0;
        }

        m_devices[deviceIndex] = pDevice;
        pDevice->m_iDeviceID = static_cast<uint8_t>(deviceIndex);
        m_devicecount = deviceIndex + 1;
        return 1;
    }

    int ZSysInput::DeleteDevice(ZInputDevice* pDevice)
    {
        int deviceIndex = -1;
        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i] == pDevice)
            {
                deviceIndex = i;
                break;
            }
        }

        if (deviceIndex < 0)
        {
            return 0;
        }

        for (int i = deviceIndex; i < m_devicecount - 1; ++i)
        {
            m_devices[i] = m_devices[i + 1];
            if (m_devices[i])
            {
                m_devices[i]->m_iDeviceID = static_cast<uint8_t>(i);
            }
        }

        --m_devicecount;
        m_devices[m_devicecount] = nullptr;
        return 1;
    }

    int ZSysInput::GetDeviceIdByPtr(ZInputDevice* pDevice)
    {
        for (int i = 0; i < m_devicecount; ++i)
        {
            if (m_devices[i] == pDevice)
            {
                return i;
            }
        }

        return -1;
    }
}
