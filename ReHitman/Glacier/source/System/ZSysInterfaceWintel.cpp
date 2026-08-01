#include <Glacier/System/ZSysInterfaceWintel.h>
#include <Glacier/System/CConfigurationWintel.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/ZDllBase.h>
#include <Glacier/ZEngineDataBase.h>

#include <Windows.h>
#include <intrin.h>
#pragma intrinsic(__rdtsc)

#include <functional>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <cmath>



namespace Glacier
{
    static char ClassName[] = "ZSystemClass000";

    // Globals
    STATIC_GLOBAL_CLASS_INSTANCE(MYSTR*, psErrorLog);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(MYSTR*, psErrorLog, 0x008B4FB8, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, g_lRunOutOfFocus, 0x008EE930, false);

    // ZSysInterfaceWintel
    ZSysInterfaceWintel::ZSysInterfaceWintel(int hInstance, bool bEditorMode)
        : ZSysInterfacePack(hInstance)
    {
        std::memset(m_RecordRestoreOldValues, 0, sizeof(m_RecordRestoreOldValues));

        // Get system path by our module name
        char* psModName = (char*)ZUniMemory::Allocate(0x104);
        GetModuleFileNameA((HMODULE)hInstance, psModName, 0x104);
        *strrchr(psModName, '\\') = '\0';
        m_sSystemPath = MYSTR(psModName);
        ZUniMemory::Free(psModName);

        if (bEditorMode)
        {
            SetRunMode(ERunModes::EDITOR);
        }

        if (MainWindowInit())
        {
            m_bUsePerformanceCounter = true;
            m_bStillFrame = false;
            m_bUseTryCatch = false;
            m_iNumExcept = false;
            TimeOffset = 0;
            m_bUnPauseAudio = true;
            m_sTextureExtension = "_pc";
            m_fFixedTimeStep = 0u;
            DoMainLoop = &ZSysInterfaceWintel::NormalMainLoop;
            GenerateLogPath(m_sErrorLog);
            psErrorLog = &m_sErrorLog;
            m_pReplayBuffer = nullptr;
            m_bIsPacking = false;
            m_bCaptureMouse = true;
        }
    }

    ZSysInterfaceWintel::~ZSysInterfaceWintel()
    {
        WriteReplayBuffer();
        Action::Free();

        if (MainhWnd)
        {
            DestroyWindow((HWND)MainhWnd);
            MainhWnd = nullptr;
        }

        UnregisterClassA("ZSystemClass000", (HINSTANCE)m_hInstance);
        g_pSysInterface = nullptr;
    }

    void ZSysInterfaceWintel::GenerateLogPath(MYSTR& pLogFilePath)
    {
        char aFileNameBuffer[0x400] { 0 };

        GetModuleFileNameA(nullptr, aFileNameBuffer, sizeof(aFileNameBuffer));

        char* lastSlash = strrchr(aFileNameBuffer, '\\');
        if (lastSlash)
        {
            lastSlash[1] = '\0';
        }

        char* end = aFileNameBuffer;
        while (*end)
        {
            ++end;
        }

        strcpy(end, "error.log");

        pLogFilePath = aFileNameBuffer;
    }

    void ZSysInterfaceWintel::Init()
    {
        LockDoMessages = false;
        SetGameName(nullptr);

        m_bRestartEngineFlag = false;
        m_sAutoDumpName = "pic0001.tga";
        m_bRestartEngineFlag = true;
    }

    void ZSysInterfaceWintel::PrintStatus()
    {
        g_pSysFile->PrintStatus();

        if (!m_bQuit)
        {
            // TODO: Finish after ZSoundDllWintel reversed! 
            // NOTE: Actually, nothing serious here
        }
    }

    void ZSysInterfaceWintel::CloseDown()
    {
        ZSysInterface::CloseDown();
    }

    bool ZSysInterfaceWintel::WindowDoMessages(void* hWnd)
    {
        if (LockDoMessages)
            return true;

        LockDoMessages = true;

        MSG Msg;
        if (!PeekMessageA(&Msg, (HWND)hWnd, 0, 0, 0))
        {
            LockDoMessages = false;
            return true;
        }

        while (GetMessageA(&Msg, (HWND)hWnd, 0, 0))
        {
            TranslateMessage(&Msg);
            DispatchMessageA(&Msg);
            if (!PeekMessageA(&Msg, (HWND)hWnd, 0, 0, 0))
            {
                LockDoMessages = false;
                return true;
            }
        }

        return false;
    }

    void ZSysInterfaceWintel::CloseDownMain()
    {
        for (auto* pCurrentRender = WindowFirst; pCurrentRender; pCurrentRender = pCurrentRender->Nxt)
        {
            pCurrentRender->ForceAllLeave();
        }

        NotifySystemClose(m_pSoundDll);
        NotifySystemClose(g_pRenderDll);

        if (m_pConfiguration)
        {
            ZUniMemory::Delete(m_pConfiguration);
            m_pConfiguration = nullptr;
        }

        m_bQuit = true;

        if (m_pEngineData)
        {
            m_pEngineData->CloseDown();
        }

        g_pSysInterface->UnlockRefs();

        if (m_pSoundDll)
        {
            // TODO: Finish me after ZSoundDllWintel reversed!
            // m_pSoundDll->{VFTBL +0x10}()
            RemoveDll(m_pSoundDll);
            m_pSoundDll = nullptr;
        }

        g_pSysInterface->LockRefs();
        CloseAllWindows();

        if (m_pMainDll)
        {
            m_pMainDll->End();
        }

        PrintStatus();
        UnloadRuntimeLoadedDLLs();
        if (g_pRenderDll)
        {
            // TODO: Finish me after ZRenderBaseDll reversed
            // g_pRenderDll->{VFTBL + 0x14}?
            RemoveDll(g_pRenderDll);
            g_pRenderDll = nullptr;
        }

        RemoveDll(m_pMainDll);
        m_pMainDll = nullptr;

        // Weird part, everything been removed at UnloadRuintimeLoadedDLLs
        while (m_rtLoadedDllFiles.Count() > 0)
        {
            auto rEntry = m_rtLoadedDllFiles.GetRefNr(0);
            RemoveDll(reinterpret_cast<ZDllBase*>(rEntry));
        }

        m_bInitialized = false;
    }

    void ZSysInterfaceWintel::CloseAllWindows()
    {
        // REALLY WEIRD CODE! Who will cleanup memory?!?!
        // TODO: Check this later!!!
        WindowFirst->~ZRender();
    }

    void ZSysInterfaceWintel::ReloadDLLs()
    {
        m_pEngineData->FreeDlcFiles();
        // ^^ That's all folks   
    }

    void ZSysInterfaceWintel::EditorMessage(int, void*, int)
    {
        // Do nothing
    }

    ZDllBase* ZSysInterfaceWintel::AddDll(const char* psDllPath)
    {
        return nullptr;
    }

    bool ZSysInterfaceWintel::RemoveDll(ZDllBase* pDllBase)
    {
        if (!pDllBase)
            return false;

        pDllBase->m_pHandle = nullptr; // What the fuck?! Who will release WinAPI resources??? IOI!!!!
        ZUniMemory::Delete(pDllBase);
        return true;
    }

    void ZSysInterfaceWintel::ReloadRender()
    {
        CloseAllWindows();

        const MYSTR sRenderDllName = g_pRenderDll ? g_pRenderDll->m_pName : "";

        if (g_pRenderDll)
        {
            // TODO: Finish me
            RemoveDll(g_pRenderDll);
            g_pRenderDll = nullptr;
        }

        // Backup old values
        const uint32_t lResolution[2] = { m_lResolution[0], m_lResolution[1] };
        const bool bFullScreen = m_bFullScreen;
        const auto lBpp = m_lBitsPerPixel;

        // Assign wanter
        m_lResolution[0] = m_lMainRenderResolution[0];
        m_lResolution[1] = m_lMainRenderResolution[1];
        m_lBitsPerPixel = m_lBitsPerPixelWanted;
        m_bFullScreen = m_bFullScreenWanted;


        DestroyWindow((HWND)MainhWnd);

        // Pump messages
        MSG Msg;
        while (PeekMessageA(&Msg, nullptr, 0, 0, 0))
        {
            GetMessageA(&Msg, nullptr, 0, 0);
        }

        // Lol
        Sleep(500u);

        // FUCK
        while (PeekMessageA(&Msg, nullptr, 0, 0, 0))
        {
            GetMessageA(&Msg, nullptr, 0, 0);
        }

        MainWindowInit();

        // MISSING CALL: sub_463710((int)&g_pRenderDll, (int)v10); -> Unrolled to __debugbreak

        bool bUseOldRender = false;
        if (g_pRenderDll)
        {
            // TODO: Uncomment after ZRenderBaseDll will be reversed
            // g_pRenderDll->CreateD3D();
        }
        else
        {
            MessageBoxA(nullptr, "This render is not supported. Using old render.", "Fatal error", 0x40010u);
            bUseOldRender = true;
        }

        if (bUseOldRender)
        {
            // Restore old values
            m_lResolution[0] = lResolution[0];
            m_lResolution[1] = lResolution[1];
            m_lBitsPerPixel = lBpp;
            m_bFullScreen = bFullScreen;

            if (g_pRenderDll)
            {
                // TODO: Finish me
                RemoveDll(g_pRenderDll);
                g_pRenderDll = nullptr;
            }

            m_sActiveDrawDll = sRenderDllName;
            // MISSING CALL: sub_463710((int)&g_pRenderDll, (int)v10); -> Unrolled to __debugbreak

            if (g_pRenderDll)
            {
                // TODO: Uncomment after ZRenderBaseDll will be reversed
                // g_pRenderDll->CreateD3D();
            }
            else
            {
                printf("Fatal error: failed to initialize g_pRenderDll!\n");
            }
        }

        m_bReloadRender = false;
    }

    void ZSysInterfaceWintel::CloseForRestart()
    {
        m_bQuit = true;
        CloseAllWindows();
        if (m_pEngineData)
        {
            m_pEngineData->CloseDown();
        }

        m_pMainDll->End();
        PrintStatus();

        RemoveDll(m_pMainDll);

        m_pMainDll = nullptr;
        m_bInitialized = false;
        m_bRestartEngineFlag = true;
    }

    void ZSysInterfaceWintel::ParseOptions()
    {
        ZSysInterfacePack::ParseOptions();

        char* pszResult = nullptr;

        if (GetOption("AlwaysPack", &pszResult))
        {
            m_bAlwaysPack = true;
        }

        if (GetOption("AutoDumpName", &pszResult))
        {
            m_sAutoDumpName = MYSTR(pszResult);
        }

        if (GetOption("ColorDepth", &pszResult))
        {
            m_lBitsPerPixel = std::atol(pszResult);
        }

        if (GetOption("DirectPlayDll", &pszResult))
        {
            m_sActiveDirectPlayDll = MYSTR(pszResult);
        }

        if (GetOption("DisableOptions", &pszResult))
        {
            m_bDisableOptions = true;
        }

        if (GetOption("DisableStitch", &pszResult))
        {
            const bool bDisableStitch = *pszResult ? std::atol(pszResult) != 0 : true;
            m_bStitchStrips = !bDisableStitch;
        }

        if (GetOption("DisableTextures", &pszResult))
        {
            m_bTextures = false;
        }

        if (GetOption("DrawDll", &pszResult))
        {
            m_sActiveDrawDll = MYSTR(pszResult);
        }

        if (GetOption("EnableScriptRTC", &pszResult))
        {
            m_bRunTimeCompileEnabled = true;
        }

        if (GetOption("ErrorLog", &pszResult))
        {
            m_sErrorLog = MYSTR(pszResult);
        }

        if (GetOption("CaptureMouse", &pszResult))
        {
            m_bCaptureMouse = std::atoi(pszResult) != 0;
        }

        if (GetOption("LoadFilter", &pszResult))
        {
            ZASSERT(!m_bUsingRawProjectPath);
            g_pSysFile->SetLoadFilter(pszResult);
        }

        if (GetOption("LocaleDll", &pszResult))
        {
            m_sActiveLocaleDll = MYSTR(pszResult);
        }

        if (GetOption("PackFile", &pszResult))
        {
            m_sPackFile = MYSTR(pszResult);
        }

        if (GetOption("PackPC", &pszResult))
        {
            m_sPackDestinationPath = MYSTR(pszResult);
            NormalizePath(&m_sPackDestinationPath);
            m_bAlwaysPack = true;
        }

        if (GetOption("PackPS2", &pszResult) ||
            GetOption("PackPS2PAL", &pszResult) ||
            GetOption("PackPS2NTSC", &pszResult))
        {
            m_sPackDestinationPath = MYSTR(pszResult);
            NormalizePath(&m_sPackDestinationPath);
            m_bAlwaysPack = true;
        }

        if (GetOption("PackXBox", &pszResult))
        {
            m_sPackDestinationPath = MYSTR(pszResult);
            NormalizePath(&m_sPackDestinationPath);
            m_bAlwaysPack = true;
        }

        if (GetOption("PackXenon", &pszResult))
        {
            m_sPackDestinationPath = MYSTR(pszResult);
            NormalizePath(&m_sPackDestinationPath);
            m_bAlwaysPack = true;
        }

        if (GetOption("PackGameCube", &pszResult))
        {
            m_sPackDestinationPath = MYSTR(pszResult);
            NormalizePath(&m_sPackDestinationPath);
            m_bAlwaysPack = true;
        }

        if (GetOption("PathfinderInfo", &pszResult))
        {
            m_bPathfinderInfo = true;
        }

        if (GetOption("Resolution", &pszResult))
        {
            const int width = std::atol(pszResult);
            m_lResolution[0] = width < 0 ? -width : width;

            int i = 0;
            while (pszResult[i] == '-' || (pszResult[i] >= '0' && pszResult[i] <= '9'))
            {
                ++i;
            }

            m_lResolution[1] = std::atol(&pszResult[i + 1]);
        }

        if (GetOption("ScriptDebug", &pszResult))
        {
            m_bScriptDebug = true;
        }

        if (GetOption("ScriptDll", &pszResult))
        {
            m_sActiveScriptDll = MYSTR(pszResult);
        }

        if (GetOption("ScriptIfDll", &pszResult))
        {
            m_sActiveScriptIfDll = MYSTR(pszResult);
        }

        if (GetOption("ShadowDetail", &pszResult))
        {
            m_lShadowDetail = std::atol(pszResult);

            if (m_lShadowDetail < 0)
            {
                m_lShadowDetail = 0;
            }

            if (m_lShadowDetail > 2)
            {
                m_lShadowDetail = 2;
            }
        }

        if (GetOption("StartUpperPos", &pszResult))
        {
            m_lStartUpperLeftPos[0] = std::atol(pszResult);

            int i = 0;
            while (pszResult[i] == '-' || (pszResult[i] >= '0' && pszResult[i] <= '9'))
            {
                ++i;
            }

            while (pszResult[i] == ',' || pszResult[i] == ' ')
            {
                ++i;
            }

            m_lStartUpperLeftPos[1] = std::atol(&pszResult[i]);
        }

        if (GetOption("SoundDll", &pszResult))
        {
            m_sActiveSoundDll = MYSTR(pszResult);
        }

        if (GetOption("TextureExtension", &pszResult))
        {
            m_sTextureExtension = MYSTR(pszResult);
        }

        if (GetOption("TextureResolution", &pszResult))
        {
            int textureResolution = std::atol(pszResult);

            if (textureResolution < 0)
            {
                textureResolution = 0;
            }

            if (textureResolution > 3)
            {
                textureResolution = 3;
            }

            m_lTextureResolution[0] = textureResolution;
            m_lTextureResolution[1] = textureResolution;
        }

        if (GetOption("UseGameController", &pszResult))
        {
            m_bUseGameController = true;
        }

        if (GetOption("UseDirectInputMouse", &pszResult))
        {
            m_bUseDirectInputMouse = false;
        }

        if (GetOption("UseDirectInputKeyboard", &pszResult))
        {
            m_bUseDirectInputKeyboard = true;
        }

        if (GetOption("UseTryCatchMainLoop", &pszResult))
        {
            m_bUseTryCatch = true;
        }

        if (GetOption("Window", &pszResult))
        {
            const bool bWindowed = *pszResult ? std::atol(pszResult) != 0 : true;
            m_bFullScreen = !bWindowed;
        }

        if (GetOption("PlayFile", &pszResult))
        {
            m_sStartPlayFile = MYSTR(pszResult);
        }

        if (GetOption("RecordFile", &pszResult))
        {
            m_sStartRecordFile = MYSTR(pszResult);

            if (m_sStartRecordFile.Length())
            {
                g_pSysFile->Delete(m_sStartRecordFile);
            }
        }

        if (GetOption("SaveRecordRuntime", &pszResult))
        {
            m_bSaveRecordRuntime = true;
        }
    }

    void ZSysInterfaceWintel::InitConfiguration()
    {
        m_pConfiguration = ZUniMemory::New<CConfigurationWintel>();
    }

    void ZSysInterfaceWintel::SetGameName(const char* psGameName)
    {
        m_sGameName = psGameName;
        SetWindowTextA((HWND)MainhWnd, m_sGameName);
    }

    MYSTR ZSysInterfaceWintel::ConvertFileName(const char* pName)
    {
        MYSTR fileName;

        if (!pName)
        {
            return fileName;
        }

        if (std::strlen(pName) > 1 && pName[1] != ':' && std::strncmp(pName, "\\\\", 2) != 0)
        {
            if (m_sProjectPath.Length())
            {
                fileName += m_sProjectPath;
            }
        }

        fileName += MYSTR(pName);

        return fileName;
    }
    
    bool ZSysInterfaceWintel::IsPacking() const
    {
        return m_bIsPacking || m_bAlwaysPack;
    }

    void ZSysInterfaceWintel::SetIsPacking(bool bPacking)
    {
        m_bIsPacking = bPacking;
    }

    void ZSysInterfaceWintel::RunMain(char* StartCmdLine)
    {
        SetCmdLine(StartCmdLine);

        if (WindowDoMessages(nullptr))
        {
            do
            {
                RunMainOnce(true);
            }
            while (m_bRestartEngineFlag);
        }

        SaveReplayBuffer();
        ClosePlayFile();
    }
    
    bool ZSysInterfaceWintel::RunMainOnce(bool UpdateViews)
    {
        if (!m_bInitialized)
        {
            if (!m_bRestartEngineFlag)
            {
                return false;
            }

            SetGameName(nullptr);
            m_bRestartEngineFlag = false;
            g_pSysFile->Restart();

            if (!StartUpMain())
            {
                CloseDownMain();
                return false;
            }

            m_bInitialized = true;
        }

        if (!WindowDoMessages(nullptr) || m_bRestartEngineFlag || m_bQuit)
        {
            CloseDown();
            m_bInitialized = false;
            return false;
        }

        // TODO: Finish next line after g_pNetwork reversed (or remove it)
        // g_pNetwork->{VFTABLE +0x20}

        std::invoke(DoMainLoop, this, UpdateViews);
        DumpAutoShots();
        return true;
    }
    
    void ZSysInterfaceWintel::DumpAutoShots()
    {
        // Do nothing
    }

    void ZSysInterfaceWintel::StepFrameTime()
    {
        ZSysInterface::StepFrameTime();
    }

    TIMETYPE ZSysInterfaceWintel::StepTime()
    {
        if (m_fTimeMultiplier_override != -1.0f)
        {
            Set_TimeMultiplier(m_fTimeMultiplier_override);
            m_fTimeMultiplier_override = -1.0f;
        }

        const double prevRealClock = m_fRealTimeOffset_;
        m_fRealTimeOffset_ = static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC);

        m_fRealPreTime = m_fRealTime;
        m_fRealTimeDelta = Get_TimeMultiplier() * static_cast<float>(m_fRealTimeOffset_ - prevRealClock);
        m_fRealTime_ += m_fRealTimeDelta;
        m_fRealTime.secs = static_cast<int>(m_fRealTime_ * TIMETYPE::kTicksPerSecond);

        const int64_t prevCounter = TimeOffset;

        if (m_bUsePerformanceCounter)
        {
            LARGE_INTEGER performanceCount;
            m_bUsePerformanceCounter = QueryPerformanceCounter(&performanceCount) != FALSE;
            TimeOffset = performanceCount.QuadPart;

            if (!m_bUsePerformanceCounter)
            {
                ZASSERT(false);
            }

            if (CPUCycSec <= 0.0f)
            {
                LARGE_INTEGER frequency;
                if (QueryPerformanceFrequency(&frequency))
                {
                    CPUCycSec = static_cast<float>(frequency.QuadPart);
                }
            }
        }
        else
        {
            TimeOffset = TimeStampCounter(__FILE__, __LINE__);
        }

        if (CPUCycSec <= 0.0f)
        {
            CPUCycSec = static_cast<float>(CLOCKS_PER_SEC);
        }

        if (TimeOffset < prevCounter)
        {
            ClearTime();
            return StepTime();
        }

        if (!m_bStillFrame)
        {
            double actualDelta = 0.0;

            if (m_bFollowRealTime)
            {
                actualDelta = Get_TimeMultiplier() * m_fRealTimeDelta;
            }
            else
            {
                int64_t currentCounter = TimeOffset;
                if (m_fFixedTimeStep)
                {
                    currentCounter = prevCounter + static_cast<int64_t>(m_fFixedTimeStep);
                    TimeOffset = currentCounter;
                }

                actualDelta = static_cast<double>(currentCounter - prevCounter) / CPUCycSec;
                if (actualDelta > 1.0)
                {
                    actualDelta = 1.0;
                }

                if (m_fInterpolatedTimeDiff == 0.0)
                {
                    m_fInterpolatedTimeDiff = actualDelta;
                }

                m_fInterpolatedTimeDiff = actualDelta * 0.300000011920929 +
                    static_cast<double>(m_fInterpolatedTimeDiff) * 0.699999988079071;

                actualDelta = static_cast<double>(m_fInterpolatedTimeDiff);
                if (actualDelta > 0.1)
                {
                    actualDelta = 0.1;
                }

                actualDelta *= Get_TimeMultiplier();
            }

            m_fActualTimeDelta = static_cast<float>(actualDelta);
            m_fActualTime_ += actualDelta;
            m_fActualTime.secs = static_cast<int>(m_fActualTime_ * TIMETYPE::kTicksPerSecond);
        }

        m_bStillFrame = false;

        ProcessReplay(&m_fRealTimeOffset_, sizeof(m_fRealTimeOffset_), 6, __FILE__, __LINE__);
        ProcessReplay(&m_fRealPreTime, sizeof(m_fRealPreTime), 7, __FILE__, __LINE__);
        ProcessReplay(&m_fRealTimeDelta, sizeof(m_fRealTimeDelta), 8, __FILE__, __LINE__);
        ProcessReplay(&m_fRealTime, sizeof(m_fRealTime), 9, __FILE__, __LINE__);
        ProcessReplay(&m_bUsePerformanceCounter, sizeof(m_bUsePerformanceCounter), 10, __FILE__, __LINE__);
        ProcessReplay(&TimeOffset, sizeof(TimeOffset), 11, __FILE__, __LINE__);
        ProcessReplay(&m_fInterpolatedTimeDiff, sizeof(m_fInterpolatedTimeDiff), 12, __FILE__, __LINE__);
        ProcessReplay(&m_fActualTimeDelta, sizeof(m_fActualTimeDelta), 13, __FILE__, __LINE__);
        ProcessReplay(&m_fActualTime, sizeof(m_fActualTime), 14, __FILE__, __LINE__);

        TIMETYPE result;
        result.secs = m_fActualTime.secs;
        return result;
    }

    void ZSysInterfaceWintel::StillFrame()
    {
        m_bStillFrame = true;
    }

    void ZSysInterfaceWintel::ClearTime()
    {
        m_fRealTimeOffset_ = 0.0;
        m_fRealTime_ = 0.0;
        m_fRealTimeDelta = 0.0f;

        m_fRealPreTime = 0;
        PreFrameTime = 0;
        m_fActualTime = 0;
        m_fActualTime_ = 0.0;
        m_fRealTime = 0;
        FrameTime = 0;
        FrameTimeOffset = 0;
        m_fInterpolatedTimeDiff = 0.0f;
        ResetTime();

        if (Action::instance)
        {
            Action::instance->DisableInputTimer(0.0f);
        }
    }

    void ZSysInterfaceWintel::ResetTime()
    {
        if (m_bUsePerformanceCounter)
        {
            LARGE_INTEGER performanceCount;
            QueryPerformanceCounter(&performanceCount);
            TimeOffset = performanceCount.QuadPart;
        }
        else
        {
            TimeOffset = TimeStampCounter(__FILE__, __LINE__);
        }

        m_fRealTimeOffset_ = static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC);

        ProcessReplay(&m_fRealTimeOffset_, sizeof(m_fRealTimeOffset_), 6, __FILE__, __LINE__);
        ProcessReplay(&TimeOffset, sizeof(TimeOffset), 11, __FILE__, __LINE__);
    }

    void ZSysInterfaceWintel::CalcCycSec()
    {
        if (CycSec == 0.0f)
        {
            LARGE_INTEGER performanceCount;
            LARGE_INTEGER frequency;

            const bool hasPerformanceCounter = QueryPerformanceCounter(&performanceCount) != FALSE;
            const bool hasPerformanceFrequency = QueryPerformanceFrequency(&frequency) != FALSE;

            m_bUsePerformanceCounter = hasPerformanceCounter && hasPerformanceFrequency;
            if (m_bUsePerformanceCounter)
            {
                CycSec = static_cast<float>(frequency.QuadPart);
            }
            else
            {
                const int64_t start = TimeStampCounter(__FILE__, __LINE__);
                Sleep(100u);
                CPUCycSec = static_cast<float>(static_cast<double>(TimeStampCounter(__FILE__, __LINE__) - start) * 10.0);
            }

            const int64_t cpuStart = TimeStampCounter(__FILE__, 1906);
            Sleep(100u);
            CPUCycSec = static_cast<float>(static_cast<double>(TimeStampCounter(__FILE__, __LINE__) - cpuStart) * 10.0);

            ProcessReplay(&m_bUsePerformanceCounter, sizeof(m_bUsePerformanceCounter), 10, __FILE__, __LINE__);
            ProcessReplay(&CycSec, sizeof(CycSec), 15, __FILE__, __LINE__);
        }
    }
    
    void ZSysInterfaceWintel::Sleep(float fTime)
    {
        Sleep(fTime);
    }

    bool ZSysInterfaceWintel::DisplayAssert(const char* pMessage, const char* pFileName, int lLineNr)
    {
        // NOTE: This is not original blood money code! Just for debug, will remove later
        char aMessageBuffer[1024] {};
        
        std::snprintf(
            aMessageBuffer, 
            sizeof(aMessageBuffer), 
            "Assertion Failed!\n\nMessage: %s\nFile: %s\nLine: %d", 
            pMessage  ? pMessage  : "N/A", 
            pFileName ? pFileName : "N/A", 
            lLineNr
        );

        MessageBoxA(
            nullptr, 
            aMessageBuffer, 
            "ReHitman | Assertion failed", 
            MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST
        );

        ZASSERT(false);
        return true;
    }
    
    MYSTR ZSysInterfaceWintel::GetSuggestedUserPath() const
    {
        char rawPath[MAX_PATH] = { 0 };
        char expandedPath[MAX_PATH] = { 0 };
        DWORD cbData = sizeof(rawPath);
        DWORD type = REG_SZ;
        HKEY hKey = nullptr;

        bool bSuccess = false;

        if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",  0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueExA(hKey, "Personal", nullptr, &type, (BYTE*)rawPath, &cbData) == ERROR_SUCCESS)
            {
                ExpandEnvironmentStringsA(rawPath, expandedPath, sizeof(expandedPath));
                strcat_s(expandedPath, "\\");
                bSuccess = true;
            }
            RegCloseKey(hKey);
        }

        if (bSuccess)
        {
            return MYSTR(expandedPath);
        }

        return MYSTR("\\");
    }

    void ZSysInterfaceWintel::SRand(int lSeed, const char* pSourceFile, int lLineNr)
    {
        ProcessReplay(&lSeed, sizeof(4), 17, pSourceFile, lLineNr);
        m_lRandSeed = lSeed;
    }
    
    int ZSysInterfaceWintel::Rand(char* pSourceFile, int lLineNr)
    {
        // 0x130 + 0x0075FDAC
        auto lValue = ZSysInterface::Rand(pSourceFile, lLineNr);
        const auto lStoredValue = lValue;

        ProcessReplay(&lValue, sizeof(lValue), 18, pSourceFile, lLineNr);
        ZASSERT(lStoredValue == lValue);

        return lValue;
    }
    
    float ZSysInterfaceWintel::FRand(char* pSourceFile, int lLineNr)
    {
        // DronCode: I'm not sure about this, IDA disasm says that constant is different, but a little bit
        return ZSysInterface::FRand(pSourceFile, lLineNr);
    }

    int64_t ZSysInterfaceWintel::TimeStampCounter(const char* pSourceFile, int lLineNr)
    {
        int64_t lTicks = __rdtsc();
        ProcessReplay(&lTicks, sizeof(lTicks), 16, pSourceFile, lLineNr);
        return lTicks;
    }

    void ZSysInterfaceWintel::NotifySystemClose(ZDllBase* pDll)
    {
        if (!pDll) return;

        using NotifyDestroy_t = int(__cdecl*)();

        if (auto* pNotifyFunc = (NotifyDestroy_t)pDll->GetAddress("NotifyDestroy"))
        {
            pNotifyFunc();
        }
    }
    
    void ZSysInterfaceWintel::SaveGraphicsOptions()
    {
        if (m_bDisableOptions)
        {
            return;
        }

        const char* cmdLine = m_sCmdLine;
        if (!cmdLine || *cmdLine != '@')
        {
            return;
        }

        const char* iniFileName = cmdLine + 1;
        if (!g_pSysFile->Exists(iniFileName, false))
        {
            return;
        }

        const int fileSize = g_pSysFile->GetSize(iniFileName, false);
        char* fileBuffer = static_cast<char*>(ZUniMemory::Allocate(fileSize + 1));
        g_pSysFile->Load(iniFileName, fileBuffer, fileSize, 0, false);
        fileBuffer[fileSize] = '\0';

        void* outFile = g_pSysFile->Create(iniFileName);
        if (!outFile)
        {
            ZUniMemory::Free(fileBuffer);
            return;
        }

        char resolution[16];
        char colorDepth[12];
        char shadowDetail[12];
        char textureResolution[12];

        std::sprintf(resolution, "%dx%d", m_lResolution[0], m_lResolution[1]);
        std::sprintf(colorDepth, "%d", m_lBitsPerPixel);
        std::sprintf(shadowDetail, "%d", m_lShadowDetail);
        std::sprintf(textureResolution, "%d", m_lTextureResolution[0]);

        struct GraphicsOption
        {
            const char* name;
            const char* value;
            bool written;
        };

        GraphicsOption options[] =
        {
            { "Window", m_bFullScreen ? nullptr : "1", false },
            { "Resolution", resolution, false },
            { "DrawDll", g_pRenderDll ? g_pRenderDll->m_pName : nullptr, false },
            { "ColorDepth", colorDepth, false },
            { "ShadowDetail", shadowDetail, false },
            { "TextureResolution", textureResolution, false },
            { nullptr, nullptr, false }
        };

        char* lineStart = fileBuffer;
        char lineBuffer[252];

        while (lineStart && *lineStart)
        {
            char* lineEnd = std::strchr(lineStart, '\n');

            if (!lineEnd)
            {
                lineEnd = std::strchr(lineStart, '\r');
            }

            if (lineEnd)
            {
                ++lineEnd;
            }
            else
            {
                lineEnd = lineStart + std::strlen(lineStart);
            }

            bool matched = false;

            for (int i = 0; options[i].name; ++i)
            {
                const char* name = options[i].name;
                if (memicmp(lineStart, name, std::strlen(name)) == 0)
                {
                    matched = true;

                    if (!options[i].written)
                    {
                        options[i].written = true;

                        const int length = options[i].value ?
                            std::sprintf(lineBuffer, "%s %s\r\n", name, options[i].value) :
                            std::sprintf(lineBuffer, "%s\r\n", name);

                        g_pSysFile->WriteTo(outFile, lineBuffer, length);
                    }

                    break;
                }
            }

            if (!matched)
            {
                g_pSysFile->WriteTo(outFile, lineStart, static_cast<int>(lineEnd - lineStart));
            }

            lineStart = lineEnd;
        }

        for (int i = 0; options[i].name; ++i)
        {
            if (!options[i].written)
            {
                options[i].written = true;

                const int length = options[i].value ?
                    std::sprintf(lineBuffer, "%s %s\r\n", options[i].name, options[i].value) :
                    std::sprintf(lineBuffer, "%s\r\n", options[i].name);

                g_pSysFile->WriteTo(outFile, lineBuffer, length);
            }
        }

        g_pSysFile->Close(outFile);
        ZUniMemory::Free(fileBuffer);
    }

    void ZSysInterfaceWintel::SetFixedTimeStep(float fTimeStep)
    {
        CalcCycSec();
        m_fFixedTimeStep = fTimeStep * CycSec;
    }

    void ZSysInterfaceWintel::ReplaceDll(ZDllBase* pDll)
    {
        m_rtLoadedDllFiles.Add(reinterpret_cast<uint32_t>(pDll));
    }

    void ZSysInterfaceWintel::SendToPartner(const char*, uint32_t)
    {
        // Do nothing
    }

    void ZSysInterfaceWintel::GetFromPartner(const char*, uint32_t)
    {
        // Do nothing
    }
    
    void ZSysInterfaceWintel::UPlotF(ZRender* Window, int PosX, int PosY, const char* Format, ...)
    {
        char aBuffer[0x400] { 0 };

        va_list args;
        va_start(args, Format);
        vsprintf(aBuffer, Format, args);

        Window->UPlotF(PosX, PosY, aBuffer);
    }

    void ZSysInterfaceWintel::UPlotF(int PosX, int PosY, const char* Format,...)
    {
        char aBuffer[0x400] { 0 };

        va_list args;
        va_start(args, Format);
        vsprintf(aBuffer, Format, args); // In original code, this method inside loop

        for (auto* pCurrentRender = WindowFirst; pCurrentRender; pCurrentRender = pCurrentRender->Nxt)
        {
            pCurrentRender->UPlotF(PosX, PosY, aBuffer);
        }
    }

    void ZSysInterfaceWintel::UPlotFNxt(const char* Format, ...)
    {
        char aBuffer[0x400] { 0 };

        va_list args;
        va_start(args, Format);
        vsprintf(aBuffer, Format, args); // In original code, this method inside loop

        for (auto* pCurrentRender = WindowFirst; pCurrentRender; pCurrentRender = pCurrentRender->Nxt)
        {
            pCurrentRender->UPlotFNxt(aBuffer);
        }
    }

    void ZSysInterfaceWintel::ProcessReplay(void *pBuffer, uint32_t lDataSize, int lChannelIdx, const char *pFuncName, int lFuncLine)
    {
        if (m_sPlayFile.Length())
        {
            PlaybackFrame(pBuffer, lDataSize, lChannelIdx, pFuncName, lFuncLine);
        }
        if (m_sRecordFile.Length())
        {
            RecordFrame(pBuffer, lDataSize, lChannelIdx, pFuncName, lFuncLine);
        }
    }

    bool ZSysInterfaceWintel::StartUpMain()
    {
        if (!ReadCmdLine(m_sCmdLine))
        {
            return false;
        }

        ParseOptions();

        if (m_bUseTryCatch)
        {
            DoMainLoop = &ZSysInterfaceWintel::TryCatchMainLoop;

            const char* errorLogFileName = m_sErrorLog;
            g_pSysFile->StartTemp(const_cast<char*>(errorLogFileName));

            time_t currentTime;
            std::time(&currentTime);

            char logLine[508];
            std::sprintf(logLine, "Starting error log on: %s", std::ctime(&currentTime));
            g_pSysFile->Append(errorLogFileName, logLine, static_cast<int>(std::strlen(logLine)));
        }

        m_sDefaultScene = CorrectSceneFileName(m_sDefaultScene);

        ResetTime();
        CalcCycSec();
        InitConfiguration();

        // TODO: Finish this place after ZDllMain will be reversed
        // m_pMainDll = ZDllMain::ZDllMain();
        // if (m_pMainDll)
        // {
        //     m_pMainDll->Init();
        // }
        m_pMainDll = nullptr;

        if (!g_pRenderDll)
        {
            // TODO: Finish this place after ZRenderBaseDll and CreateD3DDll will be reversed
            // g_pRenderDll = CreateD3DDll();
            // g_pRenderDll->CreateMaterialBuffer();
        }

        m_pSoundDll = nullptr;
        if (!GetOption("DisableAudio", nullptr))
        {
            // TODO: Finish this place after ZDllSoundWintel will be reversed
            // m_pSoundDll = ZDllSoundWintel::BuildInstance();
        }

        if (m_pSoundDll)
        {
            m_pSoundDll->Init();
        }

        m_pEngineData->CreateObjectFactories();

        // TODO: Finish this place after SysInput will be reversed
        // if (SysInput::instance)
        // {
        //     SysInput::instance->Init();
        // }

        m_pEngineData->StartUp();
        ResetTime();

        if (WindowFirst)
        {
            // TODO: Finish this place after ZRender will be fully reversed
            // WindowFirst->InitRender(); // vftable + 0x214
        }

        SetupThreadAffinity();
        return true;
    }

    bool ZSysInterfaceWintel::SetupThreadAffinity()
    {
        if (GetOption("DisableThreadAffinity", nullptr))
        {
            return true;
        }

        ULONG_PTR processAffinityMask = 0;
        ULONG_PTR systemAffinityMask = 0;
        if (!GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemAffinityMask))
        {
            return false;
        }

        if (!processAffinityMask)
        {
            ZASSERT(false);
        }

        if (!systemAffinityMask)
        {
            ZASSERT(false);
        }

        if (processAffinityMask == 1)
        {
            return (systemAffinityMask & 0xFF) != 0;
        }

        int processorCount = 0;
        for (ULONG_PTR mask = 1; mask; mask <<= 1)
        {
            if (processAffinityMask & mask)
            {
                ++processorCount;
            }
        }

        if (processorCount <= 1)
        {
            return false;
        }

        const int seed = static_cast<int>(TimeStampCounter(__FILE__, __LINE__));
        SRand(seed, __FILE__, __LINE__);

        const int selectedProcessor = Rand(const_cast<char*>(__FILE__), __LINE__) % processorCount;
        int processorIndex = 0;
        DWORD_PTR threadAffinityMask = 0;

        for (ULONG_PTR mask = 1; mask; mask <<= 1)
        {
            if (!(processAffinityMask & mask))
            {
                continue;
            }

            if (processorIndex == selectedProcessor)
            {
                threadAffinityMask = mask;
                break;
            }

            ++processorIndex;
        }

        if (!threadAffinityMask)
        {
            ZASSERT(false);
        }

        if ((threadAffinityMask & processAffinityMask) == 0)
        {
            ZASSERT(false);
        }

        return SetThreadAffinityMask(GetCurrentThread(), threadAffinityMask) != 0;
    }

    bool ZSysInterfaceWintel::MainWindowInit()
    {
        WNDCLASSEXA wcx;
        char errorMessage[512]{};

        int classIndex = 0;
        for (; classIndex != 1000; ++classIndex)
        {
            std::sprintf(&ClassName[12], "%03d", classIndex);

            wcx.cbSize = sizeof(wcx);
            if (!GetClassInfoExA(static_cast<HINSTANCE>(m_hInstance), ClassName, &wcx))
            {
                break;
            }
        }

        const int screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);
        const int x = screenWidth / 2 - 320;
        const int y = screenHeight / 2 - 240;
        const int right = screenWidth / 2 + 320;
        const int bottom = screenHeight / 2 + 240;

        std::memset(&wcx, 0, sizeof(wcx));
        wcx.cbSize = sizeof(wcx);
        wcx.style = 0xAB;
        wcx.lpfnWndProc = ZSysInterfaceWintel::MainWindowProc;
        wcx.cbClsExtra = 0;
        wcx.cbWndExtra = 4;
        wcx.hInstance = static_cast<HINSTANCE>(m_hInstance);
        wcx.hIcon = LoadIconA(static_cast<HINSTANCE>(m_hInstance), MAKEINTRESOURCEA(0x65));
        wcx.hCursor = LoadCursorA(nullptr, IDC_ARROW);
        wcx.hbrBackground = static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));
        wcx.lpszMenuName = nullptr;
        wcx.lpszClassName = ClassName;
        wcx.hIconSm = LoadIconA(static_cast<HINSTANCE>(m_hInstance), MAKEINTRESOURCEA(0x65));

        if (!RegisterClassExA(&wcx))
        {
            const DWORD lastError = GetLastError();
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, lastError, 0, errorMessage, sizeof(errorMessage), nullptr);
            ZASSERT(false);
        }

        MainhWnd = CreateWindowExA(
            0,
            ClassName,
            "Glacier",
            WS_OVERLAPPEDWINDOW,
            x,
            y,
            right - x,
            bottom - y,
            nullptr,
            nullptr,
            static_cast<HINSTANCE>(m_hInstance),
            nullptr);

        if (!MainhWnd)
        {
            const DWORD lastError = GetLastError();
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, lastError, 0, errorMessage, sizeof(errorMessage), nullptr);
            ZASSERT(false);
        }

        SetWindowLongA(static_cast<HWND>(MainhWnd), 0, reinterpret_cast<LONG>(this));
        return true;
    }

    LRESULT CALLBACK ZSysInterfaceWintel::MainWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        if (Msg > WM_SETFOCUS)
        {
            if (Msg == WM_KILLFOCUS)
            {
                // TODO: Finish this place after ZDllSoundWintel will be reversed
                // if (g_pSysInterface->m_pSoundDll && !g_pSysInterface->m_pSoundDll->IsMuted())
                // {
                //     g_pSysInterface->m_bUnPauseAudio = true;
                //     g_pSysInterface->m_pSoundDll->Pause(1, 1);
                // }

                // TODO: Finish this place after fullscreen deactivate lock flag will be identified
                if (!g_lRunOutOfFocus)
                {
                    g_pSysInterface->m_lIsActive = 0;

                    // TODO: Finish this place after SysInput will be reversed
                    // if (SysInput::instance)
                    // {
                    //     SysInput::OnFocusLost();
                    // }
                }
            }
            else if (Msg == WM_SYSCOMMAND)
            {
                if (wParam == 0x1000)
                {
                    MessageBoxA(nullptr, "Censured by tech", "", 0);
                }
                else if (wParam == SC_CLOSE)
                {
                    return 0;
                }
            }
        }
        else
        {
            switch (Msg)
            {
            case WM_SETFOCUS:
                // TODO: Finish this place after ZDllSoundWintel will be reversed
                // if (g_pSysInterface->m_pSoundDll && g_pSysInterface->m_bUnPauseAudio)
                // {
                //     g_pSysInterface->m_bUnPauseAudio = false;
                //     g_pSysInterface->m_pSoundDll->Pause(0, 1);
                // }

                g_pSysInterface->ResetTime();
                g_pSysInterface->m_lIsActive = 1;

                // TODO: Finish this place after SysInput will be reversed
                // if (SysInput::instance)
                // {
                //     SysInput::OnFocusGained();
                // }
                break;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_ACTIVATE:
                if (LOWORD(wParam))
                {
                    g_pSysInterface->ResetTime();
                    g_pSysInterface->m_lIsActive = 1;

                    if (g_pSysInterface->m_bFullScreen)
                    {
                        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        ShowWindow(hWnd, SW_RESTORE);
                    }
                }
                else
                {
                    // TODO: Finish this place after fullscreen deactivate lock flag will be identified
                    if (!g_lRunOutOfFocus)
                    {
                        g_pSysInterface->m_lIsActive = 0;

                        if (g_pSysInterface->m_bFullScreen)
                        {
                            SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        }

                        // TODO: Finish this place after ZDllSoundWintel will be reversed
                        // if (g_pSysInterface->m_pSoundDll)
                        // {
                        //     g_pSysInterface->m_pSoundDll->SetActive(false);
                        // }
                    }
                }
                break;
            }
        }

        if (g_pSysInterface->WindowFirst)
        {
            // TODO: Finish this place after ZRender window message handler will be reversed
            // return g_pSysInterface->WindowFirst->HandleWindowMessage(Msg, wParam, lParam); // vftable + 0x1D8
        }

        return DefWindowProcA(hWnd, Msg, wParam, lParam);
    }

    void ZSysInterfaceWintel::NormalMainLoop(bool UpdateViews)
    {
        if (m_lIsActive)
        {
            TimeStampCounter(__FILE__, __LINE__);
            StepFrameTime();

            if (std::isnan(g_pSysInterface->DeltaFrameTime))
            {
                ZASSERT(false);
            }

            if (g_pSysInterface->DeltaFrameTime < 0.0f)
            {
                ZASSERT(false);
            }

            if (FrameTime.secs >= 0)
            {
                // TODO: Finish this place after SysInput will be reversed
                // if (SysInput::instance)
                // {
                //     SysInput::instance->Update();
                // }

                // TODO: Finish this place after ZSysInterface::CheckDebugKeys will be reversed
                // CheckDebugKeys();

                if (!g_pSysInterface->m_bUseDirectInputMouse && g_pSysInterface->WindowFirst)
                {
                    POINT point;
                    GetCursorPos(&point);

                    // TODO: Finish this place after ZRender Win32 window fields and mouse input will be reversed
                    // ScreenToClient(g_pSysInterface->WindowFirst->m_hWnd, &point);
                    // g_pSysInterface->WindowFirst->SetMousePosition(0, point.x, point.y); // vftable + 0x208
                }

                m_pEngineData->MainLoop(UpdateViews);
            }
            else
            {
                m_pEngineData->LoadScene(m_pEngineData->GetSceneName());
                m_pEngineData->ControlSceneChange();
            }

            if (m_sMovieOutput.Length())
            {
                static int lFrameNr = 0;

                char frameFileName[512];
                ZSysInterface* formatter = BeforeFormat();
                formatter->SPrintF(frameFileName, "%s/%04d.bmp", static_cast<const char*>(m_sMovieOutput), lFrameNr);
                ++lFrameNr;

                if (WindowFirst)
                {
                    WindowFirst->Dump(frameFileName);
                }

                const size_t frameFileNameLength = std::strlen(frameFileName);
                frameFileName[frameFileNameLength] = '\n';
                frameFileName[frameFileNameLength + 1] = '\0';

                MYSTR frameGrabListName = m_sMovieOutput + MYSTR("/FrameGrab.lst");
                g_pSysFile->Append(frameGrabListName, frameFileName, static_cast<int>(std::strlen(frameFileName)));
            }
        }
        else
        {
            MSG msg;
            if (!PeekMessageA(&msg, nullptr, 0, 0, 0))
            {
                WaitMessage();
            }
        }
    }

    void ZSysInterfaceWintel::TryCatchMainLoop(bool UpdateViews)
    {
        try
        {
            NormalMainLoop(UpdateViews);
        }
        catch (...)
        {
            exit(1);
        }
    }

    void ZSysInterfaceWintel::UnloadRuntimeLoadedDLLs()
    {
        while (true)
        {
            RefRun run;
            m_rtLoadedDllFiles.RunInitNxtRef(&run);

            ZDllBase* pDllToRemove = nullptr;
            for (uint32_t* pRef = m_rtLoadedDllFiles.RunNxtRefPtr(&run);
                pRef;
                pRef = m_rtLoadedDllFiles.RunNxtRefPtr(&run))
            {
                auto* pDll = reinterpret_cast<ZDllBase*>(*pRef);
                if (pDll && pDll->CleanupBeforeCloseDown())
                {
                    pDllToRemove = pDll;
                    m_rtLoadedDllFiles.RunDelRef(&run);
                    break;
                }
            }

            if (!pDllToRemove)
            {
                break;
            }

            RemoveDll(pDllToRemove);
        }
    }

    void ZSysInterfaceWintel::SaveReplayBuffer()
    {
        WriteReplayBuffer();
        m_sRecordFile = "";
    }

    void ZSysInterfaceWintel::WriteReplayBuffer()
    {
        if (m_sRecordFile.Length())
        {
            const int lBufferSize = m_pReplayPointer - m_pReplayBuffer;
            g_pSysFile->Save(m_sRecordFile, m_pReplayBuffer, lBufferSize, 0);
        }

        if (m_pReplayBuffer)
        {
            ZUniMemory::Free(m_pReplayBuffer);
            m_pReplayBuffer = nullptr;
        }

        m_pReplayBuffer = nullptr;
        m_pReplayPointer = 0;
        m_lReplayBufferSize = 0;
    }

    void ZSysInterfaceWintel::ClosePlayFile()
    {
        m_sPlayFile = "";
        if (m_pReplayBuffer)
        {
            ZUniMemory::Free(m_pReplayBuffer);
            m_pReplayBuffer = nullptr;
        }
    }

    /**
     * @brief Finds the next literal byte range used by the replay RLE compressor.
     *
     * @details
     * The Wintel replay system records and plays back selected pieces of runtime state
     * that pass through @ref ZSysInterfaceWintel::ProcessReplay. Typical callers are
     * timing, random number generation, and other small deterministic synchronization
     * points. The goal is not to record every frame as a video stream; instead, the
     * system records enough binary state to reproduce the same execution path later.
     *
     * Each replay entry is associated with a small integer channel id. A channel is a
     * stable stream of one kind of data, for example real time, frame time, random seed,
     * or another engine synchronization value. The original engine reserves 19 channels,
     * and @ref m_RecordRestoreOldValues stores the previous decoded frame for each one.
     * That per-channel previous frame is the key to the compact replay format.
     *
     * Before compression, @ref ZSysInterfaceWintel::RecordFrame builds a raw frame with
     * the following layout:
     *
     * @code
     * int32_t  channelIdx;
     * uint32_t dataSize;
     * char     fileTail[16];
     * int32_t  lineNr;
     * uint8_t  payload[dataSize];
     * @endcode
     *
     * @c fileTail stores only the last up-to-15 characters of the source filename that
     * called @ref ProcessReplay. This is how playback can cheaply detect that the replay
     * stream is still aligned with the same code path without storing long absolute paths.
     * The source line is recorded as well, matching the original format, although playback
     * primarily validates the channel, data size, and filename tail.
     *
     * The raw frame is not written directly. Instead, recording computes a byte-wise delta
     * against the previous raw frame for the same channel:
     *
     * @code
     * delta[i] = currentRawFrame[i] - previousRawFrame[i];
     * @endcode
     *
     * The previous raw frame is then replaced with the current raw frame. This makes replay
     * data very small for values that change slowly or remain identical across frames: most
     * bytes in the delta become zero, and zero runs compress extremely well.
     *
     * The stored stream layout is therefore:
     *
     * @code
     * uint32_t compressedSize;
     * uint8_t  compressedDelta[compressedSize];
     * @endcode
     *
     * @ref ZSysInterfaceWintel::PlaybackFrame performs the inverse operation. It reads the
     * compressed size, decompresses the delta, adds that delta back into the previous raw
     * frame for the requested channel, and then validates that the reconstructed raw frame
     * describes the same channel, payload size, and filename tail as the current call site.
     * If validation succeeds, the payload bytes are copied into the caller-provided buffer.
     * If validation fails or the replay stream ends, @ref m_bRestoreOver is set and playback
     * stops driving state from the replay file.
     *
     * The compression used here is a small signed-run RLE format:
     *
     * @li A positive 16-bit run length means: repeat the following byte that many times.
     *     The encoded block is @c int16_t(length), then @c uint8_t(value).
     * @li A negative 16-bit run length means: copy the following @c -length literal bytes.
     *     The encoded block is @c int16_t(-length), then that many raw bytes.
     * @li Repeated-byte runs are only emitted when the run is longer than 3 bytes. Shorter
     *     sequences are cheaper as literals because a repeated-byte block costs 3 bytes.
     * @li Runs are capped to @c 0x7FFF because the length is stored in a signed 16-bit value.
     *
     * This helper returns the length of the next literal range. A literal range continues
     * until the compressor sees a repeated-byte run that is worth encoding separately, or
     * until the input ends. In other words, this function identifies the chunk that should
     * be emitted as a negative-length literal block before the next positive repeated-byte
     * block can be written.
     *
     * The implementation intentionally mirrors the original binary format rather than using
     * a general-purpose compression library. Replay files depend on this exact byte layout.
     */
    static uint32_t RLE_FindLiteralRunLength(const uint8_t* pInBufferStart, const uint8_t* pInBufferEnd)
    {
        const uint8_t* current = pInBufferStart;

        while (current < pInBufferEnd)
        {
            const uint8_t* runEnd = current + 1;
            while (runEnd < pInBufferEnd && runEnd < current + 0x7FFF && *runEnd == *current)
            {
                ++runEnd;
            }

            if (runEnd - current > 3)
            {
                break;
            }

            ++current;
        }

        return static_cast<uint32_t>(current - pInBufferStart);
    }

    static int CompressRLE(char* pOutBuffer, uint32_t lOutBufferSize, const uint8_t* pInBufferStart, const uint8_t* pInBufferEnd)
    {
        const uint8_t* input = pInBufferStart;
        char* output = pOutBuffer;
        char* outputEnd = pOutBuffer + lOutBufferSize;

        while (input < pInBufferEnd)
        {
            const uint8_t* runEnd = input + 1;
            while (runEnd < pInBufferEnd && runEnd < input + 0x7FFF && *runEnd == *input)
            {
                ++runEnd;
            }

            const uint32_t runLength = static_cast<uint32_t>(runEnd - input);
            if (runLength > 3)
            {
                if (output + 3 > outputEnd)
                {
                    return -1;
                }

                *reinterpret_cast<int16_t*>(output) = static_cast<int16_t>(runLength);
                output[2] = static_cast<char>(*input);
                output += 3;
                input += runLength;
            }

            const uint32_t literalLength = RLE_FindLiteralRunLength(input, pInBufferEnd);
            if (literalLength)
            {
                if (output + literalLength + 2 > outputEnd)
                {
                    return -1;
                }

                *reinterpret_cast<int16_t*>(output) = -static_cast<int16_t>(literalLength);
                output += 2;
                std::memcpy(output, input, literalLength);
                output += literalLength;
                input += literalLength;
            }
        }

        return static_cast<int>(output - pOutBuffer);
    }

    static int DecompressRLE(char* pOutBufferStart, char* pOutBufferEnd, const char* pInBufferStart, const char* pInBufferEnd)
    {
        char* output = pOutBufferStart;
        const char* input = pInBufferStart;

        while (output < pOutBufferEnd && input < pInBufferEnd)
        {
            const int16_t runLength = *reinterpret_cast<const int16_t*>(input);
            input += 2;

            if (runLength <= 0)
            {
                const int literalLength = -runLength;
                if (output + literalLength > pOutBufferEnd || input + literalLength > pInBufferEnd)
                {
                    return -1;
                }

                std::memcpy(output, input, literalLength);
                output += literalLength;
                input += literalLength;
            }
            else
            {
                if (output + runLength > pOutBufferEnd || input >= pInBufferEnd)
                {
                    return -1;
                }

                std::memset(output, *input, runLength);
                output += runLength;
                ++input;
            }
        }

        return (output == pOutBufferEnd && input == pInBufferEnd) ? static_cast<int>(output - pOutBufferStart) : -1;
    }

    void ZSysInterfaceWintel::PlaybackFrame(void *pBuffer, uint32_t lDataSize, int lChannelIdx, const char *pFuncName, int lFuncLine)
    {
        if (m_bRestoreOver || !m_sPlayFile.Length())
        {
            return;
        }

        if (lChannelIdx < 0 || lChannelIdx >= 19)
        {
            m_bRestoreOver = true;
            return;
        }

        if (!m_pReplayBuffer)
        {
            m_lReplayBufferSize = g_pSysFile->GetSize(m_sPlayFile, false);
            if (m_lReplayBufferSize <= 0)
            {
                m_bRestoreOver = true;
                return;
            }

            m_pReplayBuffer = static_cast<char*>(ZUniMemory::Allocate(m_lReplayBufferSize));
            m_pReplayPointer = m_pReplayBuffer;
            g_pSysFile->Load(m_sPlayFile, m_pReplayBuffer, m_lReplayBufferSize, 0, false);
        }

        if (m_pReplayPointer - m_pReplayBuffer >= m_lReplayBufferSize)
        {
            m_bRestoreOver = true;
            return;
        }

        constexpr uint32_t frameHeaderSize = 28;
        SRecordRestoreOldValues& oldValues = m_RecordRestoreOldValues[lChannelIdx];
        const uint32_t frameSize = lDataSize + frameHeaderSize;

        if (!oldValues.m_pData)
        {
            oldValues.m_pData = static_cast<char*>(ZUniMemory::Allocate(frameSize));
            std::memset(oldValues.m_pData, 0, frameSize);
            oldValues.m_lDataLen = frameSize;
        }

        if (!oldValues.m_pData || oldValues.m_lDataLen != frameSize)
        {
            m_bRestoreOver = true;
            return;
        }

        if (m_pReplayPointer + 4 > m_pReplayBuffer + m_lReplayBufferSize)
        {
            m_bRestoreOver = true;
            return;
        }

        const uint32_t compressedSize = *reinterpret_cast<uint32_t*>(m_pReplayPointer);
        m_pReplayPointer += 4;

        if (m_pReplayPointer + compressedSize > m_pReplayBuffer + m_lReplayBufferSize)
        {
            m_bRestoreOver = true;
            return;
        }

        char* deltaFrame = static_cast<char*>(ZUniMemory::Allocate(frameSize));
        const int decompressedSize = DecompressRLE(deltaFrame, deltaFrame + frameSize, m_pReplayPointer, m_pReplayPointer + compressedSize);
        if (decompressedSize != static_cast<int>(frameSize))
        {
            ZASSERT(false);
            ZUniMemory::Free(deltaFrame);
            m_bRestoreOver = true;
            return;
        }

        for (uint32_t i = 0; i < frameSize; ++i)
        {
            oldValues.m_pData[i] += deltaFrame[i];
        }

        ZUniMemory::Free(deltaFrame);
        m_pReplayPointer += compressedSize;

        char fileName[16]{};
        const size_t fileNameLength = std::strlen(pFuncName);
        const size_t tailLength = fileNameLength > 15 ? 15 : fileNameLength;
        std::memcpy(fileName, pFuncName + fileNameLength - tailLength, tailLength + 1);

        const int recordedChannel = *reinterpret_cast<int*>(oldValues.m_pData);
        const uint32_t recordedDataSize = *reinterpret_cast<uint32_t*>(oldValues.m_pData + 4);
        const char* recordedFileName = oldValues.m_pData + 8;

        if (recordedChannel == lChannelIdx && recordedDataSize == lDataSize && stricmp(fileName, recordedFileName) == 0)
        {
            if (!lDataSize)
            {
                ZASSERT(false);
                return;
            }

            std::memcpy(pBuffer, oldValues.m_pData + frameHeaderSize, lDataSize);
        }
        else
        {
            m_bRestoreOver = true;
        }

    }

    void ZSysInterfaceWintel::RecordFrame(void *pBuffer, uint32_t lDataSize, int lChannelIdx, const char *pFuncName, int lFuncLine)
    {
        if (!m_sRecordFile.Length())
        {
            return;
        }

        if (lChannelIdx < 0 || lChannelIdx >= 19)
        {
            ZASSERT(false);
            return;
        }

        if (!m_pReplayBuffer)
        {
            m_lReplayBufferSize = 500000000;
            m_pReplayBuffer = static_cast<char*>(ZUniMemory::Allocate(m_lReplayBufferSize));
            m_pReplayPointer = m_pReplayBuffer;
        }

        constexpr uint32_t frameHeaderSize = 28;
        constexpr int replayReserveSize = 1008;
        const uint32_t frameSize = lDataSize + frameHeaderSize;
        const int usedSize = static_cast<int>(m_pReplayPointer - m_pReplayBuffer);

        if (usedSize + static_cast<int>(lDataSize) + replayReserveSize >= m_lReplayBufferSize)
        {
            m_lReplayBufferSize = usedSize;
            return;
        }

        char* frameStart = m_pReplayPointer;

        *reinterpret_cast<int*>(m_pReplayPointer) = lChannelIdx;
        m_pReplayPointer += 4;

        *reinterpret_cast<uint32_t*>(m_pReplayPointer) = lDataSize;
        m_pReplayPointer += 4;

        char fileName[16]{};
        const size_t fileNameLength = std::strlen(pFuncName);
        const size_t tailLength = fileNameLength > 15 ? 15 : fileNameLength;
        std::memcpy(fileName, pFuncName + fileNameLength - tailLength, tailLength + 1);
        std::memcpy(m_pReplayPointer, fileName, sizeof(fileName));
        m_pReplayPointer += sizeof(fileName);

        *reinterpret_cast<int*>(m_pReplayPointer) = lFuncLine;
        m_pReplayPointer += 4;

        std::memcpy(m_pReplayPointer, pBuffer, lDataSize);
        m_pReplayPointer += lDataSize;

        SRecordRestoreOldValues& oldValues = m_RecordRestoreOldValues[lChannelIdx];
        if (!oldValues.m_pData)
        {
            oldValues.m_pData = static_cast<char*>(ZUniMemory::Allocate(frameSize));
            std::memset(oldValues.m_pData, 0, frameSize);
            oldValues.m_lDataLen = frameSize;
        }

        if (oldValues.m_pData)
        {
            if (oldValues.m_lDataLen != frameSize)
            {
                ZASSERT(false);
                return;
            }

            for (uint32_t i = 0; i < frameSize; ++i)
            {
                oldValues.m_pData[i] = frameStart[i] - oldValues.m_pData[i];
            }

            const int compressedBufferSize = static_cast<int>(frameSize * 4 + 100);
            char* compressedFrame = static_cast<char*>(ZUniMemory::Allocate(compressedBufferSize));
            const int compressedSize = CompressRLE(
                compressedFrame,
                compressedBufferSize,
                reinterpret_cast<uint8_t*>(oldValues.m_pData),
                reinterpret_cast<uint8_t*>(oldValues.m_pData + frameSize));

            if (compressedSize < 0)
            {
                ZASSERT(false);
                ZUniMemory::Free(compressedFrame);
                return;
            }

            std::memcpy(oldValues.m_pData, frameStart, frameSize);

            *reinterpret_cast<uint32_t*>(frameStart) = static_cast<uint32_t>(compressedSize);
            std::memcpy(frameStart + 4, compressedFrame, compressedSize);
            m_pReplayPointer = frameStart + 4 + compressedSize;
            ZUniMemory::Free(compressedFrame);

            if (m_bSaveRecordRuntime)
            {
                g_pSysFile->Append(m_sRecordFile, frameStart, static_cast<int>(m_pReplayPointer - frameStart));
            }
        }

    }
}
