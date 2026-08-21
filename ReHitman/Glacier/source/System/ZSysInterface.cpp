#include <Glacier/System/ZIniFileParserSimple.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/Com/CCOMType.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZEngineDataBase.h>

#include <utility>
#include <cstdarg>
#include <map>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t ZGT_ZGEOM = 0;

        struct SClassInfoHolder
        {
            ZGEOMCLASSINFO* m_pClassInfo { nullptr };
            SClassInfoHolder* m_pFirstChild { nullptr };
            SClassInfoHolder* m_pNext { nullptr };

            uint32_t GetMaskWidth(uint32_t value) const
            {
                uint32_t width = 0;

                for (uint32_t i = 16; i; i >>= 1)
                {
                    if (value >> i)
                    {
                        width += i;
                        value >>= i;
                    }
                }

                if (value)
                {
                    ++width;
                }

                return width;
            }

            void Validate(uint32_t typeId, uint32_t maskId, uint32_t shift)
            {
                ZASSERT(m_pClassInfo != nullptr);

                m_pClassInfo->SetTypeIDAndMask(typeId, maskId);

                uint32_t childCount = 0;
                for (SClassInfoHolder* child = m_pFirstChild; child; child = child->m_pNext)
                {
                    ++childCount;
                }

                const uint32_t maskWidth = GetMaskWidth(childCount);
                const uint32_t step = 1u << shift;
                const uint32_t nextShift = shift + maskWidth;

                ZASSERT(nextShift < sizeof(uint32_t) * 8);

                const uint32_t childMask = nextShift ? (0xFFFFFFFFu >> (32 - nextShift)) : 0u;

                for (SClassInfoHolder* child = m_pFirstChild; child; child = child->m_pNext)
                {
                    typeId += step;
                    child->Validate(typeId, childMask, nextShift);
                }
            }

        };


        int StrCompareNoCase(const char* lhs, const char* rhs)
        {
            ZASSERT(lhs != nullptr);
            ZASSERT(rhs != nullptr);

            while (*lhs && *rhs)
            {
                char l = *lhs++;
                char r = *rhs++;

                if (l >= 'A' && l <= 'Z')
                {
                    l = static_cast<char>(l + ('a' - 'A'));
                }

                if (r >= 'A' && r <= 'Z')
                {
                    r = static_cast<char>(r + ('a' - 'A'));
                }

                if (l != r)
                {
                    return static_cast<unsigned char>(l) - static_cast<unsigned char>(r);
                }
            }

            return static_cast<unsigned char>(*lhs) - static_cast<unsigned char>(*rhs);
        }
    }

    int LoadIniFile(const char* pszIniFile, char** ppContents)
    {
        ZASSERT(pszIniFile != nullptr);
        ZASSERT(ppContents != nullptr);
        ZASSERT(g_pSysFile != nullptr);

        static char g_IniFile[0x1000]{};
        static int g_lIniFileSize = 0;

        *ppContents = g_IniFile;

        if (g_lIniFileSize)
        {
            return g_lIniFileSize;
        }

        g_lIniFileSize = g_pSysFile->GetSize(pszIniFile, 0);

        ZASSERT(g_lIniFileSize > 0);
        ZASSERT(g_lIniFileSize < sizeof(g_IniFile));

        const int bytesRead = g_pSysFile->Load(pszIniFile, g_IniFile, g_lIniFileSize, 0, 0);

        ZASSERT(bytesRead == g_lIniFileSize);

        g_IniFile[g_lIniFileSize] = '\0';

        return g_lIniFileSize;
    }

    ZSysInterface::ZSysInterface(int hInstance)
    {
        m_fRealTime = TIMETYPE(0);
        m_fRealPreTime = TIMETYPE(0);
        m_fActualTime = TIMETYPE(0);
        m_fMainCurTime = TIMETYPE(0);
        FrameTime = TIMETYPE(0);
        PreFrameTime = TIMETYPE(0);
        FrameTimeOffset = TIMETYPE(0);
        m_fFrameTimeLastKey = TIMETYPE(0);
        m_hInstance = (void*)hInstance;
        m_pSoundDll = nullptr;
        g_pRenderDll = nullptr;
        m_lResolutionWanted[0] = 0xFFFFFFFF;
        m_lResolutionWanted[1] = 0xFFFFFFFF;
        Initialize(hInstance);
    }

    ZSysInterface::~ZSysInterface() = default;

    void ZSysInterface::SaveGameProgress(ZSysInterface::ESaveGameProgress)
    {
        // Do nothing
    }

    void ZSysInterface::SetProgressBarRect(float x, float y, float w, float h)
    {
        m_v4ProgressBarRect = { x, y, w, h };
    }

    void ZSysInterface::LockRefs()
    {
        m_bRefsLocked = true;

    }

    void ZSysInterface::UnlockRefs()
    {
        m_bRefsLocked = false;
    }

    void ZSysInterface::CloseDown()
    {
        m_bQuit = true;
    }

    void ZSysInterface::ParseOptions()
    {
        if (static_cast<bool>(g_GlobalCom["DebugAnimPack"]))
        {
            m_bDebugAnimPack = true;
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("AutoExit")))
        {
            m_fAutoExitTime = static_cast<float>(std::atof(value));
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("ConfigFile")))
        {
            m_sConfigFile = MYSTR(value);

            if (static_cast<const char*>(m_sConfigFile)[0] == '%')
            {
                int endEnv = 1;

                while (static_cast<const char*>(m_sConfigFile)[endEnv] != '%')
                {
                    ++endEnv;
                }

                int suffixStart = endEnv + 1;

                while (static_cast<const char*>(m_sConfigFile)[suffixStart] == '\\' ||
                    static_cast<const char*>(m_sConfigFile)[suffixStart] == '/')
                {
                    ++suffixStart;
                }

                const char* configFile = static_cast<const char*>(m_sConfigFile);

                if (std::strncmp(configFile + 1, "USERPROFILE", 11) == 0)
                {
                    MYSTR suggestedUserPath = GetSuggestedUserPath();

                    suggestedUserPath += m_sGameTitle;
                    suggestedUserPath += MYSTR("\\");
                    suggestedUserPath += MYSTR(configFile + suffixStart);

                    m_sConfigFile = suggestedUserPath;
                }
            }
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("DefaultScene")))
        {
            const size_t length = std::strlen(value);

            if (length >= 2 && value[0] == '"' && value[length - 1] == '"')
            {
                m_sDefaultScene = MYSTR(value + 1);
                static_cast<char*>(m_sDefaultScene)[length - 2] = '\0';
            }
            else
            {
                m_sDefaultScene = MYSTR(value);
            }
        }

        if (g_GlobalCom.Exists("DisableLight", 0))
        {
            m_bDisableLight = true;
        }

        if (g_GlobalCom.Exists("DisableConfig", 0))
        {
            m_bDisableConfig = true;
        }

        if (g_GlobalCom.Exists("DisplayMemory", 0))
        {
            m_bDisplayMemory = true;
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("HeroControlMode")))
        {
            m_iHeroControlMode = static_cast<uint32_t>(std::atol(value));
        }

        if (g_GlobalCom.GetVal("GamepadMode"))
        {
            // TODO: Finish this place after SysInput reversed
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("LocaleFile")))
        {
            m_sLocaleFile = MYSTR(value);
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("MasterProject")))
        {
            m_sDefaultMasterBank = MYSTR(value);
        }

        if (g_GlobalCom.Exists("NoBlood", 0))
        {
            m_bWantBlood = false;
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("ProjectPath")))
        {
            m_sProjectPath = MYSTR(value);
            NormalizePath(&m_sProjectPath);
            m_sProjectPath += MYSTR("Final\\");
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("ProjectFile")))
        {
            m_RunMode = VIEWER;
            m_sDefaultScene = MYSTR("");
            m_sProjectFile = MYSTR(value);

            char* projectFile = static_cast<char*>(m_sProjectFile);

            if (projectFile[0] == '"')
            {
                char* closingQuote = std::strchr(projectFile + 1, '"');
                if (closingQuote)
                {
                    *closingQuote = '\0';
                    m_sProjectFile = MYSTR(projectFile + 1);
                }
            }
        }

        if (g_GlobalCom.GetVal("AnimCollection") || g_GlobalCom.GetVal("AnimFile"))
        {
            SetRunMode(ANIMATION_VIEWER);
        }

        if (g_GlobalCom.GetVal("EnableAudioInViewer"))
        {
            m_bUseAudioInViewer = true;
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("RawProjectPath")))
        {
            m_sProjectPath = MYSTR(value);
            NormalizePath(&m_sProjectPath);
            m_bUsingRawProjectPath = true;
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("ScenesPath")))
        {
            m_sScenesPath = MYSTR(value);
        }

        // This code is just in PC, I guess in PS2 it's assigned to smth
        // g_GlobalCom.GetVal("ShowGeomMem");

        if (g_GlobalCom.GetVal("ScriptDebugPrint"))
        {
            m_bScriptDebugPrint = true;
        }

        if (g_GlobalCom.GetVal("ScriptEnableSanityCheck"))
        {
            m_bScriptEnableSanityCheck = true;
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("TexturePath")))
        {
            m_sTexturePath = MYSTR(value);
            NormalizePath(&m_sTexturePath);
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("TextureResolution")))
        {
            const int resolution = std::atoi(value);

            m_lTextureResolution[0] = resolution;
            m_lTextureResolution[1] = resolution;
        }

        if (g_GlobalCom.GetVal("AnimationViewer") || g_GlobalCom.GetVal("CharacterViewer"))
        {
            SetRunMode(ANIMATION_VIEWER);
        }

        if (g_GlobalCom.GetVal("Viewer"))
        {
            m_RunMode = VIEWER;
        }

        if (const char* value = static_cast<const char*>(g_GlobalCom.GetVal("LevelListFile")))
        {
            m_sLevelListFile = MYSTR(value);
        }
    }

    void ZSysInterface::SetOption(const char* psName, const char* psValue)
    {
        g_GlobalCom.SetVal(psName, psValue, CCOM_TYPE_STRING);
    }

    void ZSysInterface::RemoveOption(const char* psName)
    {
        g_GlobalCom.RemoveVal(psName, 0);
    }

    bool ZSysInterface::ReadCmdLine(const char* psCmdLine)
    {
        ZIniFileParserSimple parser;

        MYSTR sProcessedCmdLine = parser.ProcessCmdLine(psCmdLine);
        const char* pszIniFile = static_cast<const char*>(sProcessedCmdLine);

        if (!pszIniFile || !*pszIniFile)
        {
            return true;
        }

        char* pszContents = nullptr;
        LoadIniFile(pszIniFile, &pszContents);

        return parser.ParseIniFile(pszContents);
    }

    bool ZSysInterface::FindDefaultIniFile()
    {
        // Lol, but same on PC
        m_sIniFile = "main.ini";
        return true;
    }

    void ZSysInterface::LoadConfiguration()
    {
        // Do nothing
    }

    CConfiguration* ZSysInterface::GetConfiguration() const
    {
        return m_pConfiguration;
    }

    bool ZSysInterface::IsControllerConnected(uint32_t lControlledId)
    {
        return false;
    }

    MYSTR ZSysInterface::ProjectPath() const
    {
        return m_sProjectPath;
    }

    MYSTR ZSysInterface::DefaultScene() const
    {
        return m_sDefaultScene;
    }

    MYSTR ZSysInterface::DatabasePath() const
    {
        MYSTR sResult = ProjectName();
        if (!sResult.Length())
        {
            return sResult;
        }

        MYSTR sP0 = "P:\\";
        sP0 += sResult;

        return sP0;
    }

    MYSTR ZSysInterface::ProjectName() const
    {
        const char* pszProjectPath = static_cast<const char*>(m_sProjectPath);
        const char* driveSeparator = std::strrchr(pszProjectPath, ':');

        if (driveSeparator)
        {
            const char* projectStart = std::strchr(driveSeparator, '\\');
            if (projectStart)
            {
                MYSTR result(projectStart + 1);

                char* projectEnd = std::strchr(static_cast<char*>(result), '\\');
                if (projectEnd)
                {
                    *projectEnd = '\0';
                    return result;
                }
            }
        }

        return MYSTR("");
    }

    MYSTR ZSysInterface::WorldPath() const
    {
        MYSTR suffix("Geometry");
        return m_sProjectPath + suffix;
    }

    MYSTR ZSysInterface::DataPath() const
    {
        MYSTR suffix("Data");
        return m_sProjectPath + suffix;
    }

    MYSTR ZSysInterface::ScenesPath() const
    {
        return m_sProjectPath + m_sScenesPath;
    }

    MYSTR ZSysInterface::MastersPath() const
    {
        return WorldPath() + MYSTR("/Masters");
    }

    MYSTR ZSysInterface::MaterialsPath() const
    {
        return m_sProjectPath + MYSTR("Materials");
    }

    MYSTR ZSysInterface::SoundsPath() const
    {
        return m_sProjectPath + MYSTR("Audio");
    }

    MYSTR ZSysInterface::TexturePath() const
    {
        MYSTR sBasePath {};

        if (m_sTexturePath.Length() > 0)
        {
            sBasePath = m_sTexturePath;
        }
        else
        {
            sBasePath = "P:\\";
            sBasePath += ProjectName();
        }

        sBasePath += "\\TextureMaps";
        return sBasePath;
    }

    MYSTR ZSysInterface::LocalePath() const
    {
        const char* pszLocale = static_cast<const char*>(g_GlobalCom.GetVal("Locale"));

        if (pszLocale)
        {
            return m_sProjectPath + MYSTR("Locale\\") + MYSTR(pszLocale);
        }

        return m_sProjectPath + MYSTR("Locale");

    }

    MYSTR ZSysInterface::SaveGamePath() const
    {
        return m_sProjectPath + MYSTR("Save");
    }

    MYSTR ZSysInterface::CorrectSceneFileName(const char* psSceneFileName)
    {
        MYSTR result;

        if (!psSceneFileName || !*psSceneFileName)
        {
            result = MYSTR("");
            return result;
        }

        result = MYSTR(psSceneFileName);

        if (!std::strchr(static_cast<const char*>(result), ':'))
        {
            result = ScenesPath() + MYSTR("\\") + MYSTR(psSceneFileName);
        }

        return result;
    }

    void ZSysInterface::SetCmdLine(const char* psCmdLine)
    {
        m_sCmdLine = MYSTR(psCmdLine);
    }

    bool ZSysInterface::IsPacking() const
    {
        return false;
    }

    void ZSysInterface::SetIsPacking(bool bPacking)
    {
        // Do nothing
    }

    void ZSysInterface::StepFrameTime()
    {
        TIMETYPE stepTime = StepTime();

        PreFrameTime = FrameTime;

        if (m_pEngineData->IsPaused())
        {
            FrameTimeOffset.secs = PreFrameTime.secs - stepTime.secs;
        }
        else
        {
            FrameTime.secs = stepTime.secs + FrameTimeOffset.secs;
        }

        DeltaFrameTime = static_cast<float>(FrameTime.secs - PreFrameTime.secs) / static_cast<float>(TIMETYPE::kTicksPerSecond);
    }

    bool ZSysInterface::DisplayAssert(const char* pMessage, const char* pFileName, int lLineNr)
    {
        // Do nothing
        return false;
    }

    ZSysInterface* ZSysInterface::BeforeFormat()
    {
        // What?
        return this;
    }

    int ZSysInterface::Vsprintf(char* buffer, const char* format, void* parg)
    {
        return vsprintf(buffer, format, (va_list)parg);
    }

    int ZSysInterface::SPrintF(char* buf, const char* format, ...)
    {
        va_list args;
        va_start(args, format);

        return vsprintf(buf, format, args);
    }

    MYSTR ZSysInterface::GetSuggestedUserPath() const
    {
        // Lol wat?
        return "\\";
    }


    void ZSysInterface::SRand(int lSeed, const char* pSourceFile, int lLineNr)
    {
        m_bSRandDone = true;
        m_lRandSeed = lSeed;
    }

    int ZSysInterface::Rand(char* pSourceFile, int lLineNr)
    {
        int32_t lNextSeed = 0x10DCD * m_lRandSeed + 1;
        m_lRandSeed = lNextSeed;
        return (lNextSeed >> 8) & 0x7FFF;
    }

    float ZSysInterface::FRand(char* pSourceFile, int lLineNr)
    {
        return static_cast<float>(Rand(pSourceFile, lLineNr)) / 32768.0f;
    }

    float ZSysInterface::FRand1(char* pSourceFile, int lLineNr)
    {
        return (static_cast<float>(Rand(pSourceFile, lLineNr)) - 16384.0f) / 16384.0f;
    }

    void ZSysInterface::FollowRealtime(bool bFollowRealTime)
    {
        m_bFollowRealTime = bFollowRealTime;
    }

    void ZSysInterface::Initialize(int hInstance)
    {
        CreateGeomClassInfoData();
        CalcClassInfoNrs();
        CalcDeriveIds();
        m_fTimeMultiplier_override = -1.0;
        m_bEthernetCableMessageVisible = 0;
        m_bMoviePlaying = 0;
        m_iTimeMultiplier_Locked = 0;
        m_RunMode = RUNTIME;
        m_bUseAudioInViewer = 0;
        m_bFollowRealTime = 0;
        m_bTextures = 1;
        m_bSRandDone = 0;
        g_pSysInterface = this;
        m_fDirection = -1.0;
        m_bRefsLocked = 0;
        m_bStitchStrips = 1;
        m_bQuit = 0;
        m_bInitStatus = 0;
        m_fTimeMultiplier = 1.0;
        m_lIsActive = 0;
        m_bDoDeactivate = 1;
        m_bAlwaysPack = 0;
        m_bAlwaysPackPathFinder = 0;
        m_bDisableConfig = 0;
        m_bReloadRender = 0;
        m_bFullScreen = 0;
        m_lBitsPerPixel = 0;
        m_lTextureResolution[0] = 0;
        m_lTextureResolution[1] = 0;
        m_lShadowDetail = 2;
        m_bDisableOptions = 0;
        m_bDisplayMemory = 0;
        m_bDisplayTimeBar = 0;
        m_bDisplayStatCounter = 0;
        m_b3DStereo = 0;
        m_bPostFiltersAvailable = 1;
        m_bWantBlood = 1;
        m_bPathfinderInfo = 0;
        m_bRunTimeCompileEnabled = 0;
        m_bUseGameController = 0;
        m_bUseDirectInputMouse = 0;
        m_bUseDirectInputKeyboard = 0;
        m_iHeroControlMode = -1;
        m_bDisableLight = 0;
        CycSec = 0.0;
        m_ulSystemMemory = 0;
        m_ulVideoMemory = 0;
        WindowFirst = 0;
        m_v4ProgressBarRect = { 0.125f, 0.48f, 0.75f, 0.04f };
        m_pConfiguration = nullptr;
        m_fFramesPerSecond = 60.0;
        FrameTimeOffset = TIMETYPE(0);
        DeltaFrameTime = 0.0;
        m_fActualTimeDelta = 0.0;
        m_fRealTimeDelta = 0.0;
        m_fRealTime = TIMETYPE(0);
        m_fRealPreTime = TIMETYPE(0);
        m_lFrameCount = 1;
        m_fAutoExitTime = 0.0;
        m_bDebugAnimPack = 0;
        m_bUsingRawProjectPath = 0;
        m_sScenesPath = "SCENES";
        m_eGameMode = SINGLE_PLAYER;
        m_bScriptEnableSanityCheck = false;
        m_bScriptDebug = true;
        m_bScriptDebugPrint = true;
    }

    void ZSysInterface::CreateGeomClassInfoData()
    {
        auto& factory = ZGEOM::GetFactory();

        for (auto it = factory.Begin(); it != factory.End(); ++it)
        {
            ZGEOMCLASSINFO* pGeomClassInfo1 = *it;

            pGeomClassInfo1->Parent = nullptr;

            ZASSERT(pGeomClassInfo1->ClassInfoType() == 0);

            const char* psParentClass = pGeomClassInfo1->ParentClass();
            if (!psParentClass)
            {
                continue;
            }

            for (auto parentIt = factory.Begin(); parentIt != factory.End(); ++parentIt)
            {
                ZGEOMCLASSINFO* pGeomClassInfo2 = *parentIt;

                ZASSERT(pGeomClassInfo2->ClassInfoType() == 0);

                if (StrCompareNoCase(psParentClass, pGeomClassInfo2->ClassInfoName()) == 0)
                {
                    pGeomClassInfo1->Parent = pGeomClassInfo2;
                    break;
                }
            }
        }

    }

    void ZSysInterface::CalcClassInfoNrs()
    {
        uint16_t lTypeIndex = 0u;

        for (auto it = ZGEOM::GetFactory().Begin(); it != ZGEOM::GetFactory().End(); ++it)
        {
            it->m_iClassInfoNr = lTypeIndex++;
        }

        for (auto it = ZBaseConRout::GetFactory().Begin(); it != ZBaseConRout::GetFactory().End(); ++it)
        {
            it->m_iClassInfoNr = lTypeIndex++;
        }
    }

    void ZSysInterface::CalcDeriveIds()
    {
        std::map<uint32_t, SClassInfoHolder> classInfoMap;

        auto& factory = ZGEOM::GetFactory();

        for (auto it = factory.Begin(); it != factory.End(); ++it)
        {
            ZGEOMCLASSINFO* pGeomClassInfo = *it;

            SClassInfoHolder holder;
            holder.m_pClassInfo = pGeomClassInfo;
            holder.m_pFirstChild = nullptr;
            holder.m_pNext = nullptr;

            classInfoMap.insert(std::make_pair(pGeomClassInfo->Type(), holder));
        }

        for (auto mapIt = classInfoMap.begin(); mapIt != classInfoMap.end(); ++mapIt)
        {
            SClassInfoHolder& holder = mapIt->second;
            ZGEOMCLASSINFO* pGeomClassInfo = holder.m_pClassInfo;

            ZASSERT(pGeomClassInfo != nullptr);

            auto* pParentClassInfo = static_cast<ZGEOMCLASSINFO*>(pGeomClassInfo->Parent);
            if (pParentClassInfo)
            {
                auto parentIt = classInfoMap.find(pParentClassInfo->Type());
                ZASSERT(parentIt != classInfoMap.end());

                holder.m_pNext = parentIt->second.m_pFirstChild;
                parentIt->second.m_pFirstChild = &holder;
            }
        }

        auto rootIt = classInfoMap.find(ZGT_ZGEOM);
        if (rootIt != classInfoMap.end())
        {
            rootIt->second.Validate(0, 0, 0);
        }

        for (auto it = factory.Begin(); it != factory.End(); ++it)
        {
            ZGEOMCLASSINFO* pGeomClassInfo = *it;

            ZASSERT(pGeomClassInfo != nullptr);

            while (pGeomClassInfo->Parent)
            {
                pGeomClassInfo = static_cast<ZGEOMCLASSINFO*>(pGeomClassInfo->Parent);
            }

            ZASSERT(pGeomClassInfo->Type() == ZGT_ZGEOM);
        }
    }

    void ZSysInterface::LoadSave(ISerializerStream& stream)
    {
        stream.Exchange("m_fRealTimeOffset", m_fRealTimeOffset_);
        stream.Exchange("m_fRealTime_", m_fRealTime_);
        stream.Exchange("m_fActualTime_", m_fActualTime_);

        m_fRealTime.LoadSave(stream);

        stream.Exchange("m_fRealTimeDelta", m_fRealTimeDelta);

        m_fRealPreTime.LoadSave(stream);
        m_fActualTime.LoadSave(stream);

        stream.Exchange("m_fActualTimeDelta", m_fActualTimeDelta);

        m_fMainCurTime.LoadSave(stream);

        stream.Exchange("m_lFrameCount", m_lFrameCount);

        FrameTime.LoadSave(stream);
        PreFrameTime.LoadSave(stream);

        stream.Exchange("DeltaFrameTime", DeltaFrameTime);

        FrameTimeOffset.LoadSave(stream);

        stream.Exchange("CycSec", CycSec);
        stream.Exchange("m_lRandSeed", m_lRandSeed);
        stream.Exchange("m_bSRandDone", m_bSRandDone);
    }

    bool ZSysInterface::IsDebugActive() const
    {
        return m_bDebugActive;
    }

    TIMETYPE ZSysInterface::ActualFrameTime() const
    {
        return m_fActualTime;
    }

    float ZSysInterface::ActualFrameTimeDelta() const
    {
        return m_fActualTimeDelta;
    }

    TIMETYPE ZSysInterface::GetRealTime() const
    {
        return m_fRealTime;
    }

    float ZSysInterface::GetRealTimeDelta() const
    {
        return m_fRealTimeDelta;
    }

    bool ZSysInterface::GetOption(const char* pszName, char** pResult)
    {
        return g_GlobalCom.GetVal(pszName, pResult) != 0;
    }

    void ZSysInterface::InitActionMap()
    {
        Action::Initialize();
    }

    void ZSysInterface::FreeActionMap()
    {
        Action::Free();
    }

    float ZSysInterface::Set_TimeMultiplier(float fNewTimeMul)
    {
        const float fPrevTimeMul = m_fTimeMultiplier;
        if (!m_iTimeMultiplier_Locked)
        {
            m_fTimeMultiplier = fNewTimeMul;
        }

        return fPrevTimeMul;
    }

    float ZSysInterface::Get_TimeMultiplier() const
    {
        // TODO: Need to check this code twice due we have ability to override actual time multiplier!
        return m_fTimeMultiplier;
    }

    void ZSysInterface::Lock_TimeMultiplier()
    {
        ++m_iTimeMultiplier_Locked;
    }

    void ZSysInterface::Unlock_TimeMultiplier()
    {
        ZASSERT(m_iTimeMultiplier_Locked);

        if (m_iTimeMultiplier_Locked)
        {
            --m_iTimeMultiplier_Locked;
        }
    }

    void ZSysInterface::Reset_TimeMultiplier_Lock()
    {
        m_iTimeMultiplier_Locked = 0;
    }

    void ZSysInterface::SetRunMode(ERunModes eRunMode)
    {
        m_RunMode = eRunMode;
    }

    void ZSysInterface::NormalizePath(MYSTR* pStr) const
    {
        char* pszStr = *pStr;
        const auto lLen = strlen(pszStr);

        for (int i = 0; i < lLen; ++i)
        {
            if (pszStr[i] == '/')
            {
                pszStr[i] = '\\';
            }
        }

        if (pszStr[lLen - 1] != '\\')
        {
            (*pStr) += MYSTR("\\");
        }
    }

    float ZSysInterface::GetTimeMultiplier() const
    {
        return m_fTimeMultiplier;
    }

    // Statics
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZSysInterface*, g_pSysInterface, 0x00820820, nullptr);
}
