#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/STRREFTAB.h>
#include <cstdint>

namespace Glacier
{
    class CConfiguration
    {
    public:
        // constants
        static constexpr int NUMBER_OF_PLAYERS = 4;

        // static
        STATIC_CLASS_VAR(CConfiguration, float, m_fMouseSpeed);
        STATIC_CLASS_VAR(CConfiguration, bool, m_bUseMouse);
        STATIC_CLASS_VAR_ARRAY(CConfiguration, bool, m_bInvertVertical, NUMBER_OF_PLAYERS);
        STATIC_CLASS_VAR_ARRAY(CConfiguration, bool, m_bInvertHorizontal, NUMBER_OF_PLAYERS);
        STATIC_CLASS_VAR(CConfiguration, bool, m_bCheatsEnabled);
        STATIC_CLASS_VAR(CConfiguration, bool, m_bWideScreen);
        STATIC_CLASS_VAR_ARRAY(CConfiguration, uint32_t, m_iControlMode, NUMBER_OF_PLAYERS);
        STATIC_CLASS_VAR_ARRAY(CConfiguration, bool, m_bAutoSaveEnabled, NUMBER_OF_PLAYERS);
        STATIC_CLASS_VAR(CConfiguration, bool, m_bInitConfig);
        STATIC_CLASS_VAR(CConfiguration, bool, m_bSubtitles);

        // vtbl | TODO: Check vtbl
        virtual bool SaveKeys();
        virtual const char* ExtractKeyNameFromNode(int);
        virtual const char* ExtractDeviceNameFromNode(unsigned int);
        virtual uint32_t MakeDeviceIndex(unsigned int);
        virtual int32_t ExtractKeyFromName(const char*);
        virtual void FromString(const MYSTR&);
        virtual void ToString(MYSTR& sOutStr);
        virtual void ApplyKeys();
        virtual void ApplySound();
        virtual void ApplyDisplay();
        virtual void RemoveAllKeyMappings();
        virtual void ReadIniSettings();
        virtual bool WriteIniSettings(const char* pszFilename);
        virtual bool WriteIniSettings();
        virtual const STRREFTAB& GetGenericOptions() const;
        virtual const char* GetGenericOption(const char* psOptionName);
        virtual float GetGenericFloat(const char* psOptionName, float fDefault);
        virtual int GetGenericInt(const char* psOptionName, int iDefault);
        virtual bool GetGenericBool(const char* psOptionName, bool bDefault);
        virtual void SetGenericOption(const char* psOptionName, const char* psValue);
        virtual void SetGenericOption(const char* psOptionName, bool bValue);
        virtual void RemoveOption(const char* psOptionName);
        virtual void Load();

        // methods
        CConfiguration();
        ~CConfiguration();
        void Initialize();
        void Apply();
        float GetMouseSpeed();
        void SetMouseSpeed(float fSpeed);
        bool GetUseMouse();
        void SetUseMouse(bool);
        bool GetInvertVertical(uint32_t iPlayer);
        void SetInvertVertical(bool bInvert, uint32_t iPlayer);
        bool GetInvertHorizontal(uint32_t iPlayer);
        void SetInvertHorizontal(bool bInvert, uint32_t iPlayer);
        float GetTurnSpeedH();
        float GetTurnSpeedV();
        void SetTurnSpeedH(float fSpeed);
        void SetTurnSpeedV(float fSpeed);
        void SetCheatsEnabled(bool bEnabled);
        bool GetCheatsEnabled();
        void SetWideScreen(bool bWideScreen);
        bool GetWideScreen();
        void SetControlMode(uint32_t lCtrl, uint32_t iPlayer);
        uint32_t GetControlMode(uint32_t iPlayer);
        void SetAutoSaveEnabled(bool bEnabled, uint32_t iPlayer);
        bool GetAutoSaveEnabled(uint32_t iPlayer);
        bool GetSubtitles();
        void SetSubtitles(bool bShowSubtitles);
        void Save();
        static bool IsMouseGrabbed();

        template <typename T>
        void SetGenericOptionValue(const char* pszName, const T& value)
        {
            MYSTR sValue;

            sValue = value;
            SetGenericOption(pszName, static_cast<const char*>(sValue));
            g_pSysInterface->SetOption(pszName, static_cast<const char*>(sValue));
        }

        // members
        MYSTR m_sNewLine;
        STRREFTAB m_GenericOptions;
        STRREFTAB m_RemoveOptions;
    };
    RE_VERIFY_SIZE(CConfiguration, 0xC4);
}
