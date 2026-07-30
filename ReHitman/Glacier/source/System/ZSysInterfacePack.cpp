#include <Glacier/System/ZSysInterfacePack.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/ZSTL/CHUNK.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZDllBase.h>


namespace Glacier
{
    ZSysInterfacePack::ZSysInterfacePack(int hInstance) 
        : ZSysInterface(hInstance)
    {
        m_sRecordFile = "";
        m_sPlayFile = "";
        m_bSaveRecordRuntime = 0;
        m_pReplayBuffer = 0;
        m_pReplayPointer = 0;
        m_lReplayBufferSize = 0;
        m_pSetupWindow = 0;
        m_lResolution[0] = 640;
        m_lResolution[1] = 480;
        m_lStartUpperLeftPos[0] = -1;
        m_lStartUpperLeftPos[1] = -1;
        m_bViewsLocked = 0;
        m_bInitialized = 0;
        m_hInstance = (void *)hInstance;
        m_refZCamera = 0;
        g_pRenderDll = nullptr; // Same in ZSysInterface
        m_pSoundDll = nullptr;
        MasterHwnd = nullptr;
        MainhWnd = nullptr;
        PlotInfoEnable = 0;
        FrameTime = TIMETYPE(0);
        PreFrameTime = TIMETYPE(0);
        WindowFirst = nullptr;
        m_bFullScreen = true;
        m_pMainDll = nullptr;
        m_pEngineData = nullptr;

        // NOTE: This code skipped
        // TODO: Reverse later
        // CHUNK sChunk { 'EMPT', nullptr, -1 };
        // sChunk.GetFileLen();

        g_pSysInterface = this;
        g_pSysInterface->SetCmdLine("");
        m_bScriptDebug = 0;
        m_bScriptDebugPrint = 0;

        m_sTextureExtension = "_pc";
        g_pGlobalCOM = &g_GlobalCom;
    }

    ZSysInterfacePack::~ZSysInterfacePack()
    {
        if (m_pEngineData)
        {
            ZUniMemory::Delete(m_pEngineData);
            m_pEngineData = nullptr;
        }
    }
    
    void ZSysInterfacePack::ParseOptions()
    {
        ZSysInterface::ParseOptions();

        char* pszResult = nullptr;

        if (GetOption("AlwaysPack", &pszResult))
        {
            m_bAlwaysPack = true;
        }

        /*
        DronCode: iOS build contains refs to PackXenon, PackPS3, PackDurango, PackOrbis. I'm not sure that this required in this case

        Code looks like this:

        m_bSimpleRepack = ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackPS2", 0)
            || ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackXBOX", 0)
            || ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackPS2NTSC", 0)
            || ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackPS2PAL", 0)
            || ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackXenon", 0)
            || ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackPS3", 0)
            || ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackDurango", 0)
            || ZSysInterface::GetOption(g_pSysInterface, (__int64)"PackOrbis", 0);
        */
        m_bSimpleRepack = GetOption("SimpleRepack", nullptr) &&
            !GetOption("PackPS2", nullptr) &&
            !GetOption("PackXBOX", nullptr) &&
            !GetOption("PackPS2NTSC", nullptr) &&
            !GetOption("PackPS2PAL", nullptr) &&
            !GetOption("PackXenon", nullptr);

        if (GetOption("AlwaysPackPathFinder", &pszResult))
        {
            m_bAlwaysPackPathFinder = true;
        }

        if (GetOption("LoadFilter", &pszResult))
        {
            ZASSERT(!m_bUsingRawProjectPath);
            g_pSysFile->SetLoadFilter(pszResult);
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
            m_bSimpleRepack = false;
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
    }

    void ZSysInterfacePack::SetEngineData(ZEngineDataBase* pEngineData)
    {
        if (m_pEngineData)
        {
            ZUniMemory::Delete(m_pEngineData);
            m_pEngineData = nullptr;
        }

        m_pEngineData = pEngineData;
    }
}
