#include <Glacier/System/CConfiguration.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Filesystem/ZSysFile.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>


namespace Glacier
{
    CConfiguration::CConfiguration()
        : m_GenericOptions(32, 1)
        , m_RemoveOptions(32, 0)
    {
        Initialize();
    }

    CConfiguration::~CConfiguration()
    {
        RefRun run;

        m_GenericOptions.RunInitNxtRef(&run);
        for (uint32_t* pRef = m_GenericOptions.RunNxtRefPtr(&run); pRef; pRef = m_GenericOptions.RunNxtRefPtr(&run))
        {
            auto* pValue = reinterpret_cast<MYSTR*>(pRef[1]);
            if (pValue)
            {
                ZUniMemory::Delete(pValue);
                pRef[1] = 0; // ???
            }
        }

    }

    bool CConfiguration::SaveKeys()
    {
        if (g_pSysInterface->m_bDisableOptions)
            return true;

        if (!g_pSysInterface->m_sConfigFile.Length())
            return false;

        char* buffer = static_cast<char*>(ZUniMemory::Allocate(0x2710));
        char* out = buffer;

        *out = '\0';

        out += std::sprintf(out, "<%s", "Settings");
        out += std::sprintf(out, ">\n");
        out += std::sprintf(out, "<%s", "Controls");
        out += std::sprintf(out, ">\n");
        out += std::sprintf(out, "</%s>\n", "Controls");
        out += std::sprintf(out, "</%s>\n", "Settings");

        void* file = g_pSysFile->Create(g_pSysInterface->m_sConfigFile);
        if (!file)
        {
            ZUniMemory::Free(buffer);
            return false;
        }

        g_pSysFile->WriteTo(file, buffer, static_cast<int>(out - buffer));
        g_pSysFile->Close(file);

        ZUniMemory::Free(buffer);
        return true;
    }

    const char* CConfiguration::ExtractKeyNameFromNode(int)
    {
        return "Unknown";
    }

    const char* CConfiguration::ExtractDeviceNameFromNode(unsigned int iDevice)
    {
        MakeDeviceIndex(iDevice);
        return "Unknown";
    }

    uint32_t CConfiguration::MakeDeviceIndex(unsigned int iIdx)
    {
        return iIdx;
    }

    int32_t CConfiguration::ExtractKeyFromName(const char*)
    {
        return -1;
    }

    void CConfiguration::FromString(const MYSTR& pString)
    {
        MYSTR local(pString);

        char* pszString = local;

        while (*pszString)
        {
            char* pszValue = nullptr;
            char* pszName = pszString;
            char* pszEnd = pszString;

            if (*pszString != ';')
            {
                do
                {
                    if (!*pszEnd)
                        break;

                    if (*pszEnd == '=')
                    {
                        *pszEnd = '\0';
                        pszValue = pszEnd + 1;
                    }

                    ++pszEnd;
                }
                while (*pszEnd != ';');
            }

            *pszEnd = '\0';
            pszString = pszEnd + 1;

            if (!std::memcmp(pszName, "strCC", 5) && pszValue)
            {
                const int value = std::atoi(pszValue);
                SetGenericOptionValue<int>(pszName + 5, value);
            }
            else if (!std::memcmp(pszName, "strCr", 5))
            {
                RemoveOption(pszName + 5);
            }
        }
    }

    void CConfiguration::ToString(MYSTR& sOutStr)
    {
        for (const char* pszName : m_RemoveOptions)
        {
            sOutStr += MYSTR("strCr");
            sOutStr += MYSTR(pszName);
            sOutStr += MYSTR(";");
        }

        for (const char* pszName : m_GenericOptions)
        {
            const char* pszValue = GetGenericOption(pszName);

            sOutStr += MYSTR("strCC");
            sOutStr += MYSTR(pszName);
            sOutStr += MYSTR("=");
            sOutStr += MYSTR(pszValue);
            sOutStr += MYSTR(";");
        }
    }

    void CConfiguration::ApplyKeys()
    {
        // Do nothing
    }

    void CConfiguration::ApplySound()
    {
        // Do nothing
    }

    void CConfiguration::RemoveAllKeyMappings()
    {
        // Do nothing
    }

    void CConfiguration::ReadIniSettings()
    {
        // Do nothing
    }

    bool CConfiguration::WriteIniSettings(const char* pszFileName)
    {
        if (g_pSysInterface->m_bDisableOptions)
            return true;

        if (!g_pSysFile->Exists(pszFileName, false))
            return false;

        const int fileSize = g_pSysFile->GetSize(pszFileName, false);

        auto* fileBuffer = static_cast<char*>(ZUniMemory::Allocate(fileSize + 1));
        g_pSysFile->Load(pszFileName, fileBuffer, fileSize, 0, false);
        fileBuffer[fileSize] = '\0';

        void* outFile = g_pSysFile->Create(pszFileName);
        if (!outFile)
        {
            ZUniMemory::Free(fileBuffer);
            return false;
        }

        const int genericCount = m_GenericOptions.Count();
        const char** genericPairs = static_cast<const char**>(
            ZUniMemory::Allocate(sizeof(const char*) * (genericCount * 2 + 2))
        );

        int pairCount = 0;

        RefRun run;
        m_GenericOptions.RunInitNxtRef(&run);

        for (uint32_t* pRef = m_GenericOptions.RunNxtRefPtr(&run);
            pRef;
            pRef = m_GenericOptions.RunNxtRefPtr(&run))
        {
            auto* pValue = reinterpret_cast<MYSTR*>(pRef[1]);

            genericPairs[pairCount * 2] = reinterpret_cast<const char*>(pRef[0]);
            genericPairs[pairCount * 2 + 1] = pValue ? static_cast<const char*>(*pValue) : nullptr;
            ++pairCount;
        }

        genericPairs[pairCount * 2] = nullptr;
        genericPairs[pairCount * 2 + 1] = nullptr;

        char* lineStart = fileBuffer;
        char lineBuffer[252];

        while (lineStart && *lineStart)
        {
            char* lineEnd = std::strchr(lineStart, '\n');

            if (!lineEnd)
                lineEnd = std::strchr(lineStart, '\r');

            if (lineEnd)
                ++lineEnd;
            else
                lineEnd = lineStart + std::strlen(lineStart);

            bool skipLine = false;

            RefRun removeRun;
            m_RemoveOptions.RunInitNxtRef(&removeRun);

            for (uint32_t* pRef = m_RemoveOptions.RunNxtRefPtr(&removeRun);
                pRef;
                pRef = m_RemoveOptions.RunNxtRefPtr(&removeRun))
            {
                const char* pszRemoveName = reinterpret_cast<const char*>(pRef[0]);

                if (memicmp(lineStart, pszRemoveName, std::strlen(pszRemoveName)) == 0)
                {
                    skipLine = true;
                    break;
                }
            }

            if (!skipLine)
            {
                bool replacedLine = false;

                for (int i = 0; genericPairs[i * 2]; ++i)
                {
                    const char* pszName = genericPairs[i * 2];
                    const char* pszValue = genericPairs[i * 2 + 1];

                    if (memicmp(lineStart, pszName, std::strlen(pszName)) == 0)
                    {
                        replacedLine = true;

                        if (pszValue)
                        {
                            const int length = std::sprintf(lineBuffer, "%s %s\r\n", pszName, pszValue);
                            g_pSysFile->WriteTo(outFile, lineBuffer, length);

                            genericPairs[i * 2 + 1] = nullptr;
                        }

                        break;
                    }
                }

                if (!replacedLine)
                {
                    g_pSysFile->WriteTo(outFile, lineStart, static_cast<int>(lineEnd - lineStart));

                    if (*(lineEnd - 1) != '\n')
                    {
                        g_pSysFile->WriteTo(
                            outFile,
                            static_cast<char*>(m_sNewLine),
                            m_sNewLine.Length()
                        );
                    }
                }
            }

            lineStart = lineEnd;
        }

        for (int i = 0; genericPairs[i * 2]; ++i)
        {
            const char* pszName = genericPairs[i * 2];
            const char* pszValue = genericPairs[i * 2 + 1];

            if (pszValue)
            {
                const int length = std::sprintf(lineBuffer, "%s %s\r\n", pszName, pszValue);
                g_pSysFile->WriteTo(outFile, lineBuffer, length);

                genericPairs[i * 2 + 1] = nullptr;
            }
        }

        g_pSysFile->Close(outFile);

        ZUniMemory::Free(genericPairs);
        ZUniMemory::Free(fileBuffer);

        return true;
    }

    bool CConfiguration::WriteIniSettings()
    {
        const char* pszFileName = g_pSysInterface->m_sIniFile;
        return WriteIniSettings(pszFileName);
    }
    
    const STRREFTAB& CConfiguration::GetGenericOptions() const
    {
        return m_GenericOptions;
    }

    const char* CConfiguration::GetGenericOption(const char* psOptionName)
    {
        uint32_t* pRef = m_GenericOptions.FindStr(psOptionName);
        if (pRef)
        {
            auto* pValue = reinterpret_cast<MYSTR*>(pRef[1]);
            return pValue ? static_cast<const char*>(*pValue) : nullptr;
        }

        char* pszResult = nullptr;
        g_pSysInterface->GetOption(psOptionName, &pszResult);
        return pszResult;
    }

    float CConfiguration::GetGenericFloat(const char* psOptionName, float fDefault)
    {
        auto* psValue = GetGenericOption(psOptionName);
        if (psValue)
        {
            return atof(psValue);
        }

        return fDefault;
    }

    int CConfiguration::GetGenericInt(const char* psOptionName, int iDefault)
    {
        auto* psValue = GetGenericOption(psOptionName);
        if (psValue)
        {
            return atoi(psValue);
        }

        return iDefault;
    }

    bool CConfiguration::GetGenericBool(const char* psOptionName, bool bDefault)
    {
        auto* psValue = GetGenericOption(psOptionName);
        if (psValue)
        {
            return atoi(psValue) != 0;
        }

        return bDefault;
    }

    void CConfiguration::SetGenericOption(const char* psOptionName, const char* psValue)
    {
        if (m_RemoveOptions.Exists(psOptionName))
        {
            m_RemoveOptions.RemoveStr(psOptionName);
        }

        uint32_t* pRef = m_GenericOptions.FindStr(psOptionName);
        uint32_t* pValueSlot = nullptr;

        if (pRef)
        {
            pValueSlot = pRef + 1;
        }
        else
        {
            pValueSlot = m_GenericOptions.AddStr(psOptionName);
            *pValueSlot = reinterpret_cast<uint32_t>(ZUniMemory::New<MYSTR>());
        }

        MYSTR sValue(psValue);
        *reinterpret_cast<MYSTR*>(*pValueSlot) = sValue;
    }

    void CConfiguration::SetGenericOption(const char* psOptionName, bool bValue)
    {
        SetGenericOptionValue<int>(psOptionName, static_cast<int>(bValue));
    }

    void CConfiguration::RemoveOption(const char* psOptionName)
    {
        m_RemoveOptions.AddStr(psOptionName);

        uint32_t* pRef = m_GenericOptions.FindStr(psOptionName);
        if (pRef)
        {
            auto* pValue = reinterpret_cast<MYSTR*>(*(pRef + 1));
            if (pValue)
            {
                ZUniMemory::Delete(pValue);
            }

            m_GenericOptions.RemoveStr(psOptionName);
        }

        g_pSysInterface->RemoveOption(psOptionName);

    }
    
    void CConfiguration::Load()
    {
        char* pszValue = nullptr;

        if (Action::instance)
        {
            MYSTR sConfigFile = g_pSysInterface->ConvertFileName(g_pSysInterface->m_sConfigFile);

            if (!Action::instance->LoadBindings(sConfigFile))
            {
                g_pSysInterface->m_sConfigFile.Length();
            }
        }

        if (g_pSysInterface->GetOption("MouseSpeed", &pszValue))
        {
            m_fMouseSpeed = std::clamp(static_cast<float>(std::atof(pszValue)), 0.1f, 2.0f);
        }

        char key[32];

        std::strcpy(key, "InvertVerticalPlayer0");
        for (uint32_t i = 0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (g_pSysInterface->GetOption(key, &pszValue))
            {
                m_bInvertVertical[i] = std::clamp(std::atoi(pszValue), 0, 1) != 0;
            }

            ++key[std::strlen(key) - 1];
        }

        std::strcpy(key, "InvertHorizontalPlayer0");
        for (uint32_t i = 0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (g_pSysInterface->GetOption(key, &pszValue))
            {
                m_bInvertHorizontal[i] = std::clamp(std::atoi(pszValue), 0, 1) != 0;
            }

            ++key[std::strlen(key) - 1];
        }

        if (g_pSysInterface->GetOption("UseMouse", &pszValue))
        {
            m_bUseMouse = std::clamp(std::atoi(pszValue), 0, 1) != 0;
        }

        if (g_pSysInterface->GetOption("EnableCheats", nullptr))
        {
            m_bCheatsEnabled = false;
        }

        std::strcpy(key, "ControlModePlayer0");
        for (uint32_t i = 0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (g_pSysInterface->GetOption(key, &pszValue))
            {
                m_iControlMode[i] = std::clamp(std::atoi(pszValue), 0, 1);
            }

            ++key[std::strlen(key) - 1];
        }

        std::strcpy(key, "AutoSaveEnabledPlayer0");
        for (uint32_t i = 0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (g_pSysInterface->GetOption(key, &pszValue))
            {
                m_bAutoSaveEnabled[i] = std::clamp(std::atoi(pszValue), 0, 1) != 0;
            }

            ++key[std::strlen(key) - 1];
        }
    }

    void CConfiguration::Initialize()
    {
        if (m_bInitConfig)
        {
            m_fMouseSpeed = 1.0f;
            for (int iPlayer = 0; iPlayer < NUMBER_OF_PLAYERS; ++iPlayer)
            {
                SetInvertVertical(false, iPlayer);
                SetInvertHorizontal(false, iPlayer);
                SetControlMode(0, iPlayer);
                SetAutoSaveEnabled(false, iPlayer); // Actually, in PS2 always used player #0
            }

            m_sNewLine = "\n"; // weird
            m_bWideScreen = false;
            m_bInitConfig = false;
            m_bCheatsEnabled = false;
        }
    }

    void CConfiguration::Apply()
    {
        // TODO: Implement me
    }
    
    void CConfiguration::ApplyDisplay()
    {
        // Do nothing
    }
    
    float CConfiguration::GetMouseSpeed()
    {
        return m_fMouseSpeed;
    }

    void CConfiguration::SetMouseSpeed(float fSpeed)
    {
        if (fSpeed != m_fMouseSpeed)
        {
            // In original clamp implemented via max(0.1, min(fSpeed, 1.0))
            fSpeed = std::clamp(fSpeed, 0.1f, 1.0f);

            m_fMouseSpeed = fSpeed;
            SetGenericOptionValue<float>("MouseSpeed", fSpeed);
        }
    }
    
    bool CConfiguration::GetUseMouse()
    {
        return m_bUseMouse;
    }
    
    void CConfiguration::SetUseMouse(bool bUseMouse)
    {
        m_bUseMouse = bUseMouse;
    }

    bool CConfiguration::GetInvertVertical(uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);
        return m_bInvertVertical[iPlayer];
    }
    
    void CConfiguration::SetInvertVertical(bool bInvert, uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);
        m_bInvertVertical[iPlayer] = bInvert;
    }
    
    bool CConfiguration::GetInvertHorizontal(uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);
        return m_bInvertHorizontal[iPlayer];
    }
    
    void CConfiguration::SetInvertHorizontal(bool bInvert, uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);
        m_bInvertHorizontal[iPlayer] = bInvert;
    }

    void CConfiguration::SetCheatsEnabled(bool bEnabled)
    {
        // NOTE: In PC it's covered into 'ifdef RELEASE -> always false'
        m_bCheatsEnabled = bEnabled;
    }
    
    bool CConfiguration::GetCheatsEnabled()
    {
        return m_bCheatsEnabled;
    }
    
    void CConfiguration::SetWideScreen(bool bWideScreen)
    {
        m_bWideScreen = bWideScreen;
    } 
    
    bool CConfiguration::GetWideScreen()
    {
        return m_bWideScreen;
    }

    void CConfiguration::SetControlMode(uint32_t lCtrl, uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);

        m_iControlMode[iPlayer] = lCtrl;
    }
    
    uint32_t CConfiguration::GetControlMode(uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);
        return m_iControlMode[iPlayer];
    }
    
    void CConfiguration::SetAutoSaveEnabled(bool bEnabled, uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);
        m_bAutoSaveEnabled[iPlayer] = bEnabled;
    }

    bool CConfiguration::GetAutoSaveEnabled(uint32_t iPlayer)
    {
        ZASSERT(iPlayer < NUMBER_OF_PLAYERS);
        return m_bAutoSaveEnabled[iPlayer];
    }
    
    bool CConfiguration::GetSubtitles()
    {
        return m_bSubtitles;
    }

    void CConfiguration::SetSubtitles(bool bShowSubtitles)
    {
        m_bSubtitles = bShowSubtitles;
    }

    void CConfiguration::Save()
    {
        SaveKeys();
        WriteIniSettings();
    }

    STATIC_CLASS_VAR_IMPL(CConfiguration, float, m_fMouseSpeed, 0x008ACA98, 0.f);
    STATIC_CLASS_VAR_IMPL(CConfiguration, bool, m_bUseMouse, 0x008ACA94, false);
    STATIC_CLASS_VAR_ARRAY_IMPL(CConfiguration, bool, m_bInvertVertical, NUMBER_OF_PLAYERS, 0x008ACA90);
    STATIC_CLASS_VAR_ARRAY_IMPL(CConfiguration, bool, m_bInvertHorizontal, NUMBER_OF_PLAYERS, 0x008ACA8C);
    STATIC_CLASS_VAR_IMPL(CConfiguration, bool, m_bCheatsEnabled, 0x008ACA89, false);
    STATIC_CLASS_VAR_IMPL(CConfiguration, bool, m_bWideScreen, 0x008ACA88, false);
    STATIC_CLASS_VAR_ARRAY_IMPL(CConfiguration, uint32_t, m_iControlMode, NUMBER_OF_PLAYERS, 0x008ACA78);
    STATIC_CLASS_VAR_ARRAY_IMPL(CConfiguration, bool, m_bAutoSaveEnabled, NUMBER_OF_PLAYERS, 0x008ACA74);
    STATIC_CLASS_VAR_IMPL(CConfiguration, bool, m_bInitConfig, 0x007F50A8, false);
    STATIC_CLASS_VAR_IMPL(CConfiguration, bool, m_bSubtitles, 0x008ACA9C, false);
}
