#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/System/ZSysInterfacePack.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/MYSTR.h>

namespace Glacier
{
    class ZEngineDataBase;
    class ZRenderWintelD3D;
    class CConfiguration;

    struct SRecordRestoreOldValues
    {
        char*  m_pData;
        uint32_t m_lDataLen;
    };

    class ZSysInterfaceWintel : public ZSysInterfacePack
    {
    public:
        // types
        using MainLoopFunc_t = void(ZSysInterfaceWintel::*)(bool);

        // vtbl
        ~ZSysInterfaceWintel() override;
        void Init() override;
        void PrintStatus() override;
        void CloseDown() override;
        bool WindowDoMessages(void* hWnd) override;
        void CloseDownMain() override;
        void CloseAllWindows() override;
        void ReloadDLLs() override;
        bool RemoveDll(ZDllBase* pDllBase) override;
        void ReloadRender() override;
        void CloseForRestart() override;
        void ParseOptions() override;
        void InitConfiguration() override;
        void SetGameName(const char* psGameName) override;
        MYSTR ConvertFileName(const char* pName) override;
        bool IsPacking() const override;
        void SetIsPacking(bool bPacking) override;
        void RunMain(char* StartCmdLine) override;
        bool RunMainOnce(bool UpdateViews) override;
        void StepFrameTime() override;
        TIMETYPE StepTime() override;
        void StillFrame() override;
        void ClearTime() override;
        void ResetTime() override;
        void CalcCycSec() override;
        void Sleep(float fTime) override;
        bool DisplayAssert(const char* pMessage, const char* pFileName, int lLineNr) override;
        MYSTR GetSuggestedUserPath() const override;
        void SRand(int lSeed, const char* pSourceFile, int lLineNr) override;
        int Rand(char* pSourceFile, int lLineNr) override;
        float FRand(char* pSourceFile, int lLineNr) override;
        int64_t TimeStampCounter(const char* pSourceFile, int lLineNr) override;
        virtual void NotifySystemClose(ZDllBase*);
        virtual void SaveGraphicsOptions();
        virtual void SetFixedTimeStep(float fTimeStep);
        virtual void ReplaceDll(ZDllBase* pDll);
        virtual void SendToPartner(const char*, uint32_t);
        virtual void GetFromPartner(const char*, uint32_t);
        virtual void UPlotF(ZRender* Window, int PosX, int PosY, const char* Format, ...);
        virtual void UPlotF(int PosX, int PosY,const char* Format,...);
        virtual void UPlotFNxt(const char* Format,...);
        virtual void ProcessReplay(void *pBuffer, uint32_t lDataSize, int lChannelIdx, const char *pFuncName, int lFuncLine);

        // methods
        ZSysInterfaceWintel(int hInstance, bool bEditorMode);
        bool StartUpMain();
        bool MainWindowInit();
        void NormalMainLoop(bool UpdateViews);
        void TryCatchMainLoop(bool);
        void GenerateLogPath(MYSTR& pLogFilePath);
        void WriteReplayBuffer();
        void UnloadRuntimeLoadedDLLs();
        void SaveReplayBuffer();
        void ClosePlayFile();
        void PlaybackFrame(void *pBuffer, uint32_t lDataSize, int lChannelIdx, const char *pFuncName, int lFuncLine);
        void RecordFrame(void *pBuffer, uint32_t lDataSize, int lChannelIdx, const char *pFuncName, int lFuncLine);

        // members (starts at 0xDE0)
        bool m_bFullScreenWanted;
        bool m_bUsePerformanceCounter;
        bool m_bCaptureMouse;
        MYSTR m_sActiveDrawDll;
        MYSTR m_sTextureExtension;
        bool m_bIsPacking;
        MYSTR m_sActiveScriptDll;
        MYSTR m_sActiveDirectPlayDll;
        MYSTR m_sActiveSoundDll;
        MYSTR m_sActiveLocaleDll;
        MYSTR m_sMovieOutput;
        MYSTR m_sMovieExtension;
        int m_iFrameGrabStart;
        int m_iFrameGrabEnd;
        int m_iFrameGrabScaledW;
        int m_iFrameGrabScaledH;
        uint64_t m_fFixedTimeStep;
        REFTAB32 m_rtLoadedDllFiles;
        ZDllBase *m_pMainDll;
        volatile bool LockDoMessages;
        MYSTR m_sErrorLog;
        int m_iNumExcept;
        MYSTR m_sGameName;
        MYSTR m_sAutoDumpName;
        int64_t TimeOffset;
        long double m_fInterpolatedTimeDiff;
        MainLoopFunc_t DoMainLoop;
        bool m_bPack;
        bool m_bUnPauseAudio;
        MYSTR m_sRecordFile;
        MYSTR m_sStartRecordFile;
        MYSTR m_sStartPlayFile;
        MYSTR m_sPlayFile;
        bool m_bSaveRecordRuntime;
        char *m_pReplayBuffer;
        char *m_pReplayPointer;
        int m_lReplayBufferSize;
        bool m_bStillFrame;
        bool m_bUseTryCatch;
        bool m_bRestartEngineFlag;
        SRecordRestoreOldValues m_RecordRestoreOldValues[19];
    }; // Total size is 0x1700
    RE_VERIFY_SIZE(ZSysInterfaceWintel, 0x1700);
    RE_VERIFY_OFFSET(ZSysInterfaceWintel, m_bFullScreenWanted, 0xDE0);
}