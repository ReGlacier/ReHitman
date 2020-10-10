#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    class ZInputDevice
    {
    public:
        virtual void Release(bool);
        virtual int Type();
        virtual int DigitalCount();
        virtual int DigitalState(int);
        virtual void* DigitalName(int);
        virtual void DigitalId(char *);
        virtual void DigitalHist(int,int);
        virtual int AnalogCount();
        virtual int AnalogState(int);
        virtual const char* AnalogName(int);
        virtual void AnalogId(char *);
        virtual void AnalogMotion(int);
        virtual void ResetTables(bool);
        virtual void ClearKey(int);
        virtual void RumbleSetPitch(int,float);
        virtual void StartHaptic(uint,float,float,bool);
        virtual void StopHaptics();
        virtual void SetRumbleBGPitch(float,float);
        virtual void Activate();
        virtual void Deactivate();
        virtual const char* GetName();
        virtual void PrintInfo();
        virtual const char* DigitalSystemName(int);
        virtual void UnknownVirtualMethod();
        virtual bool Initialize();
        virtual void Terminate();
        virtual void Update();
        virtual void Acquire(int);
        virtual void Acquire();
        virtual void Unacquire();
    };
}