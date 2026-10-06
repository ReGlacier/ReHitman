#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/Input/ZInterface.h>
#include <cstdint>


namespace Glacier
{
    class ZSysInput : public SysInput::ZInterface 
    {
    public:
        // vtbl
        ~ZSysInput() override;
        int Update() override;
        void Suspend(bool bSuspend) override;
        void ResetTables(bool bClearKeys) override;
        SysInput::ZDevice* GetDevice(int id) override;
        const char* GetDeviceName(int id) override;
        int GetDeviceIdByName(const char* name) override;
        int GetNthDevice(SysInput::EDeviceType deviceType, int index) override; // Return index of device by specific type with offset 'index'
        int DeviceCount(SysInput::EDeviceType deviceType) override;
        SysInput::EDeviceType DeviceType(int deviceIndex) override;
        int GetPrimaryDevice(SysInput::EDeviceType deviceType) override; // return device index
        SysInput::ZDevice* GetPrimaryDevicePtr(SysInput::EDeviceType deviceType) override;
        SysInput::ZDevice* GetNthDevicePtr(SysInput::EDeviceType deviceType, int nth) override;
        bool MapDigital(const char* devname, const char* ctrlname, int* devid, int* ctrlid) override;
        bool MapAnalog(const char* devname, const char* ctrlname, int* devid, int* ctrlid) override;
        float Analog(int devid, int ctrlid) override;
        float AnalogMotion(int devid, int ctrlid) override;
        int Digital(int devid, int ctrlid) override;
        void DeactivateDevices() override;
        void ActivateDevices() override;
        bool InstallHandler(int deviceIndex, SysInput::DeviceHandler handler, void* userData) override;
        bool UninstallHandler(int deviceIndex, SysInput::DeviceHandler handler) override;
        bool OpenControllerMissingWindow() override;
        bool CheckForKeyPress(int* device, int* controlid) override;

        // methods
        ZSysInput();
        int AddDevice(ZInputDevice*);
        int DeleteDevice(ZInputDevice*);
        int GetDeviceIdByPtr(ZInputDevice*);

        // members
        ZInputDevice* m_devices[32] { nullptr };
        int m_devicecount{ 0 };
        bool m_bSuspend{ false };
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZSysInput, 0x98);
    RE_VERIFY_OFFSET(ZSysInput, m_devicecount, 0x90);
    RE_VERIFY_OFFSET(ZSysInput, m_bSuspend, 0x94);
}
