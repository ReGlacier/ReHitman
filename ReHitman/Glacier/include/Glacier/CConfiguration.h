#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/STRREFTAB.h>

namespace Glacier
{
    class CConfiguration
    {
    public:
        // data
        int m_configPath;
        int field_8;
        int field_c;
        int field_10;
        int field_14;
        int field_18;
        int field_1c;
        int field_20;
        int field_24;
        int field_28;
        int field_2c;
        int field_30;
        int field_34;
        int field_38;
        int field_3c;
        int field_40;
        int field_44;
        int field_48;
        int field_4c;
        int field_50;
        int field_54;
        int field_58;
        int field_5c;
        int field_60;
        int field_64;
        int field_68;
        int field_6c;
        int field_70;
        int field_74;
        int field_78;
        int field_7c;
        int field_80;
        STRREFTAB m_strRefTab_84;
        STRREFTAB m_strRefTab_A4;
        int field_c4;
        int field_c8;

        // vftable
        virtual void Release(bool);
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

        //public API
        static bool CanShowSubtitles();
        static void SetCanShowSubtitles(bool value);
    };
}