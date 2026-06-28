#pragma once

#include <Glacier/ZSTL/TIMETYPE.h>

namespace Glacier {
    class ZInputDevice;

    using DeviceHandler = int(__cdecl*)(unsigned int, int, void*);
    using TimeoutCallback = void(__cdecl*)();

    struct SysInput
    {
        enum EDeviceType : int
        {
            eMOUSE_TYPE = 0x0,
            eKEYBOARD_TYPE = 0x1,
            eGAMECONTROL_TYPE = 0x2
        };

        struct ZDevice
        {
            virtual ~ZDevice();
            virtual EDeviceType Type();
            virtual int DigitalCount();
            virtual int DigitalState();
            virtual char* DigitalName();
            virtual int DigitalId();
            virtual TIMETYPE DigitalHist();
            virtual int AnalogCount();
            virtual float AnalogState();
            virtual char* AnalogName();
            virtual int AnalogId();
            virtual float AnalogMotion();
            virtual void ResetTables();
            virtual void ClearKey();
            virtual void RumbleSetPitch();
            virtual bool StartHaptic();
            virtual void StopHaptics();
            virtual void SetRumbleBGPitch();
            virtual void Activate();
            virtual void Deactivate();
            virtual char* GetName();
            virtual void PrintInfo();
            virtual char* DigitalSystemName();
            virtual bool Connected();
        };

        struct ZInterface
        {
            // vtbl
            virtual ~ZInterface();
            virtual void Update();
            virtual void Suspend(bool value);
            virtual void ResetTables(bool value);
            virtual ZDevice* GetDevice(int deviceIdx);
            virtual const char* GetDeviceName(int deviceIdx);
            virtual int GetDeviceIdByName(const char* name);
            virtual int GetNthDevice(EDeviceType deviceType, int index); // Return index of device by specific type with offset 'index'
            virtual int DeviceCount(EDeviceType deviceType);
            virtual EDeviceType DeviceType(int deviceIndex);
            virtual int GetPrimaryDevice(EDeviceType deviceType); // return device index
            virtual ZDevice* GetPrimaryDevicePtr(EDeviceType deviceType);
            virtual ZDevice* GetNthDevicePtr(EDeviceType deviceType, int);
            virtual bool MapDigital(const char* a1, const char* a2, int* a3, int* a4);
            virtual bool MapAnalog(const char* a1, const char* a2, int* a3, int* a4);
            virtual void Analog(int, int);
            virtual void AnalogMotion(int, int);
            virtual void Digital(int, int);
            virtual void DeactivateDevices();
            virtual void ActivateDevices();
            virtual bool InstallHandler(int deviceIndex, DeviceHandler handler, void* userData);
            virtual bool UninstallHandler(int deviceIndex, DeviceHandler handler);
            virtual const char* SetActiveController(int controlledIndex);
            virtual void SetTimeoutCallback(TimeoutCallback callback, int timeout); // Not working in PC build
            virtual void UnSetTimeoutCallback(); // Not working in PC build
            virtual bool IsControllerConnected();
            virtual void OpenControllerMissingWindow();
            virtual void SetIsDialogAllowed(bool);
            virtual bool IsDialogAllowed();
            virtual bool CheckForKeyPress(int* a1, int* a2);

            // data
            int  m_iMode;                 // +0x04
            int  m_iActiveController;     // +0x08
            bool m_bAllowRumble;         // +0x0c
            bool m_Win32Mouse;           // +0x0d
            RE_ADD_PADDING(2);
        };

        static bool Initialize();
        static void Shutdown();
    };

    class ZSysInput : public SysInput::ZInterface {
    public:
        // Data
        ZInputDevice* m_devices[32];
        int m_devicecount{ 0 }; // 0x90
        bool m_bSuspend{ false }; // field94
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZSysInput, 0x98);
    RE_VERIFY_OFFSET(ZSysInput, m_bSuspend, 0x94);
}