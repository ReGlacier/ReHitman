#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/EDeviceType.h>
#include <Glacier/ZSTL/TIMETYPE.h>


namespace Glacier::SysInput
{
    struct ZDevice
    {
        // vtbl
        virtual ~ZDevice();
        virtual EDeviceType Type() = 0;
        virtual int DigitalCount() = 0;
        virtual int DigitalState(int nr) = 0;
        virtual const char* DigitalName(int nr) = 0;
        virtual int DigitalId(const char* name) = 0;
        virtual TIMETYPE DigitalHist(int nr, int nth) = 0;
        virtual int AnalogCount() = 0;
        virtual float AnalogState(int nr) = 0;
        virtual const char* AnalogName(int nr) = 0;
        virtual int AnalogId(const char* name) = 0;
        virtual float AnalogMotion(int nr) = 0;
        virtual void ResetTables(bool bClearKeys) = 0;
        virtual void ClearKey(int id) = 0;
        virtual void TrapKey(int id, bool bTrap) = 0;
        virtual void RumbleSetPitch(int axis, float p) = 0;
        virtual bool StartHaptic(unsigned int id, float a, float b, bool bLoop) = 0;
        virtual void StopHaptics() = 0;
        virtual void Activate() = 0;
        virtual void Deactivate() = 0;
        virtual const char* GetName() = 0;
        virtual void PrintInfo() = 0;
        virtual const char* DigitalSystemName(int id);
        virtual bool Connected() = 0;

        // methods
        ZDevice();
    };
}
