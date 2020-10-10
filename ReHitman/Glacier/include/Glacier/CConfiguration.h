#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/MYSTR.h>

namespace Glacier
{
    class CConfiguration
    {
    public:
        virtual void SaveKeys();
        virtual void ExtractKeyNameFromNode(int);
        virtual void ExtractDeviceNameFromNode(unsigned int);
        virtual void MakeDeviceIndex(unsigned int);
        virtual void ExtractKeyFromName(char const*);
        virtual void FromString(MYSTR const&);
        virtual void ToString(MYSTR&);
        virtual void ApplyKeys();
        virtual void ApplySound();
        virtual void ApplyDisplay();
        virtual void RemoveAllKeyMappings();
        virtual void ReadIniSettings();
        virtual void WriteIniSettings();
        virtual void WriteIniSettings(char const*);
        virtual void* GetGenericOptions();
        virtual void GetGenericOption(char const*);
        virtual float GetGenericFloat(char const*, float);
        virtual int GetGenericInt(char const*, int);
        virtual bool GetGenericBool(char const*, bool);
        virtual void SetGenericOption(char const*, char const*);
        virtual void SetGenericOption(char const*, bool);
        virtual void RemoveOption(char const*);
        virtual void Load();
    };
}