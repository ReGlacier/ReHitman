#pragma once

namespace Glacier {
    class ZInputDevice;

    enum class EDeviceType {
    	MOUSE = 0,
    	KEYBOARD = 1,
    	GAME_CONTROLLER = 2  ///?
    };

    using DeviceHandler = int(__cdecl*)(unsigned int, int, void*);
    using TimeoutCallback = void(__cdecl*)();

    class ZSysInput {
    public:
    	virtual void Release(bool);
    	virtual void Update();
    	virtual void Suspend(bool value);
    	virtual void ResetTables(bool value);
    	virtual ZInputDevice* GetDevice(int deviceIdx);
    	virtual const char* GetDeviceName(int deviceIdx);
    	virtual int GetDeviceIdByName(const char* name);
    	virtual int GetNthDevice(EDeviceType deviceType, int index); // Return index of device by specific type with offset 'index'
    	virtual int DeviceCount(EDeviceType deviceType);
    	virtual EDeviceType DeviceType(int deviceIndex);
    	virtual int GetPrimaryDevice(EDeviceType deviceType); // return device index
    	virtual ZInputDevice* GetPrimaryDevicePtr(EDeviceType deviceType);
    	virtual ZInputDevice* GetNthDevicePtr(EDeviceType deviceType, int);
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
    };
}