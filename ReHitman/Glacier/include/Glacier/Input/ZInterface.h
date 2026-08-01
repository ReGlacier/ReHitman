#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/Fwds.h>
#include <Glacier/Input/EDeviceType.h>
#include <Glacier/Input/Input.h>
#include <cstdint>


namespace Glacier::SysInput
{
    struct ZInterface
    {
        // vtbl
        virtual ~ZInterface();
        virtual int Update() = 0;
        virtual void Suspend(bool bSuspend) = 0;
        virtual void ResetTables(bool bClearKeys) = 0;
        virtual ZDevice* GetDevice(int id) = 0;
        virtual const char* GetDeviceName(int id) = 0;
        virtual int GetDeviceIdByName(const char* name) = 0;
        virtual int GetNthDevice(EDeviceType deviceType, int index) = 0; // Return index of device by specific type with offset 'index'
        virtual int DeviceCount(EDeviceType deviceType) = 0;
        virtual EDeviceType DeviceType(int deviceIndex) = 0;
        virtual int GetPrimaryDevice(EDeviceType deviceType) = 0; // return device index
        virtual ZDevice* GetPrimaryDevicePtr(EDeviceType deviceType) = 0;
        virtual ZDevice* GetNthDevicePtr(EDeviceType deviceType, int nth) = 0;
        virtual bool MapDigital(const char* devname, const char* ctrlname, int* devid, int* ctrlid) = 0;
        virtual bool MapAnalog(const char* devname, const char* ctrlname, int* devid, int* ctrlid) = 0;
        virtual float Analog(int devid, int ctrlid) = 0;
        virtual float AnalogMotion(int devid, int ctrlid) = 0;
        virtual int Digital(int devid, int ctrlid) = 0;
        virtual void DeactivateDevices() = 0;
        virtual void ActivateDevices() = 0;
        virtual bool InstallHandler(int deviceIndex, DeviceHandler handler, void* userData) = 0;
        virtual bool UninstallHandler(int deviceIndex, DeviceHandler handler) = 0;
        virtual void SetActiveController(int controlledIndex) = 0;
        virtual uint8_t SetTimeoutCallback(TimeoutCallback callback, int timeout) = 0; // Not working in PC build
        virtual void UnSetTimeoutCallback() = 0; // Not working in PC build
        virtual bool IsControllerConnected()  = 0;
        virtual bool OpenControllerMissingWindow();
        virtual bool IsDialogAllowed();
        virtual void SetIsDialogAllowed(bool);
        virtual bool CheckForKeyPress(int* device, int* controlid);

        // methods
        ZInterface();

        // members
        int32_t m_iMode;                 // +0x04
        int32_t m_iActiveController;     // +0x08
        bool m_bAllowRumble;         // +0x0c
        bool m_Win32Mouse;           // +0x0d
        RE_ADD_PADDING(2);
    };
}