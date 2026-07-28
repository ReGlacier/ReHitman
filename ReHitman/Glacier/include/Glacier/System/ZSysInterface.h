#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZRender;
    class ZDllBase;
    class CConfiguration;
    class ZEngineDataBase;
    struct ISerializerStream;

    using SETUPWINDOW = ZRender*(*)(void);

    class ZSysInterface
    {
    public:
        // types
        ///! Idk what is that
        enum ESaveGameProgress : int32_t
        {
            SAVE_GAME_1_1 = 0x1,
            SAVE_GAME_1_2 = 0x2,
            SAVE_GAME_1_3 = 0x3,
            GEOM_BUFFER_1 = 0x4,
            GEOM_BUFFER_2 = 0x5,
            GEOM_BUFFER_3 = 0x6,
            GEOM_BUFFER_4 = 0x7,
            SAVE_GAME_2_1 = 0x8,
            SAVE_GAME_2_2 = 0x9,
            SAVE_GAME_2_3 = 0xA,
            SAVE_GAME_3_1 = 0xB,
            SAVE_GAME_3_2 = 0xC,
            SAVE_GAME_3_3 = 0xD,
        };

        enum GeneralGameMode_e : int32_t
        { 
            SINGLE_PLAYER = 0, ///< Single Player mode

            // In MiniNinjas added more possible values
        };

        enum ERunModes : int32_t
        {
            RUNTIME = 0, ///< Running game instance
            EDITOR = 1, ///< Running level editor instance
            VIEWER = 2, ///< Content viewer
            ANIMATION_VIEWER = 3, ///< Animation viewer
        };

        // vtbl (based on PC)
        virtual void SaveGameProgress(ZSysInterface::ESaveGameProgress);
        virtual void SetProgressBarRect(float x, float y, float w, float h);
        virtual void LockRefs();
        virtual void UnlockRefs();
        virtual ~ZSysInterface();
        virtual void Init() = 0;
        virtual void PrintStatus() = 0;
        virtual void CloseDown();
        virtual bool WindowDoMessages(void* hWnd) = 0;
        virtual void CloseDownMain() = 0;
        virtual void CloseAllWindows() = 0;
        virtual void ReloadDLLs() = 0;
        virtual void EditorMessage(int, void*, int) = 0;
        virtual ZDllBase* AddDll(const char* psDllPath) = 0;
        virtual bool RemoveDll(ZDllBase* pDllBase) = 0;
        virtual void ReloadRender() = 0;
        virtual void CloseForRestart() = 0;
        virtual void ParseOptions();
        virtual void SetOption(const char* psName, const char* psValue);
        virtual void RemoveOption(const char* psName);
        virtual bool ReadCmdLine(const char* psCmdLine);
        virtual bool FindDefaultIniFile();
        virtual void LoadConfiguration();
        virtual CConfiguration* GetConfiguration() const;
        virtual void InitConfiguration() = 0;
        virtual bool IsControllerConnected(uint32_t lControlledId);
        virtual void SetGameName(const char* psGameName) = 0;
        virtual void SetEngineData(ZEngineDataBase* pEngineData) = 0;
        virtual MYSTR ProjectPath() const;
        virtual MYSTR DefaultScene() const;
        virtual MYSTR DatabasePath() const;
        virtual MYSTR ProjectName() const;
        virtual MYSTR WorldPath() const;
        virtual MYSTR DataPath() const;
        virtual MYSTR ScenesPath() const;
        virtual MYSTR MastersPath() const;
        virtual MYSTR MaterialsPath() const;
        virtual MYSTR SoundsPath() const;
        virtual MYSTR TexturePath() const;
        virtual MYSTR LocalePath() const;
        // NOTE: In MiniNInjas and later here we have an additional method
        // virtual ELocaleLanguages Locale() const; << this method returns current locale to load specific .LOC file
        virtual MYSTR SaveGamePath() const;
        virtual MYSTR ConvertFileName(const char* pName) = 0;
        virtual MYSTR CorrectSceneFileName(const char* psSceneFileName);
        virtual void SetCmdLine(const char* psCmdLine);
        virtual bool IsPacking() const;
        virtual void SetIsPacking(bool bPacking);
        virtual void RunMain(char* StartCmdLine) = 0;
        virtual bool RunMainOnce(bool UpdateViews) = 0;
        virtual void DumpAutoShots() = 0;
        virtual void StepFrameTime();
        virtual TIMETYPE StepTime() = 0;
        virtual void StillFrame() = 0;
        virtual void ClearTime() = 0;
        virtual void ResetTime() = 0;
        virtual void CalcCycSec() = 0;
        virtual void Sleep(float fTime) = 0;
        virtual bool DisplayAssert(const char* pMessage, const char* pFileName, int lLineNr);
        virtual ZSysInterface* BeforeFormat();
        virtual int Vsprintf(char* buffer, const char* format, void* parg);
        virtual int SPrintF(char* buf, const char* format, ...);
        virtual MYSTR GetSuggestedUserPath() const;
        virtual void SRand(int lSeed, const char* pSourceFile, int lLineNr);
        virtual int Rand(char* pSourceFile, int lLineNr);
        virtual float FRand(char* pSourceFile, int lLineNr);
        virtual float FRand1(char* pSourceFile, int lLineNr);
        virtual int64_t TimeStampCounter(const char* pSourceFile, int lLineNr) = 0;
        virtual void FollowRealtime(bool bFollowRealTime);
        // NOTE: In MiniNinjas and later here we have an additional method
        // virtual void SetLocale() with ability to change current game locale

        // methods
        ZSysInterface(int hInstance);
        void Initialize(int hInstance);
        void CreateGeomClassInfoData();
        void CalcClassInfoNrs();
        void CalcDeriveIds();
        void LoadSave(ISerializerStream& stream);
        bool IsDebugActive() const;
        TIMETYPE ActualFrameTime() const;
        float ActualFrameTimeDelta() const;
        TIMETYPE GetRealTime() const;
        float GetRealTimeDelta() const;
        bool GetOption(const char*, char**);
        void InitActionMap();
        void FreeActionMap();
        float Set_TimeMultiplier(float fNewTimeMul);
        float Get_TimeMultiplier() const;
        void Lock_TimeMultiplier();
        void Unlock_TimeMultiplier();
        void Reset_TimeMultiplier_Lock();
        void SetRunMode(ERunModes eRunMode);
        void NormalizePath(MYSTR* pStr) const;

        // members
        alignas(8) bool m_bDebugActive; // It's really weird, but it is
        double m_fRealTime_; // 0x10
        double m_fRealTimeOffset_; // 0x18
        double m_fActualTime_; // 0x20
        TIMETYPE m_fRealTime; // 0x28
        float m_fRealTimeDelta; //0x2C
        TIMETYPE m_fRealPreTime; // 0x30
        float dummyu65; // 0x34
        TIMETYPE m_fActualTime; // 0x38
        float m_fActualTimeDelta; // 0x3C
        TIMETYPE m_fMainCurTime; // 0x40
        uint32_t m_lFrameCount; // 0x44
        TIMETYPE FrameTime; // 0x48
        TIMETYPE PreFrameTime; // 0x4C
        float DeltaFrameTime; // 0x50
        TIMETYPE FrameTimeOffset; // 0x54
        TIMETYPE m_fFrameTimeLastKey; // 0x58
        float m_fFramesPerSecond; // 0x5C
        float CycSec; // 0x60
        float CPUCycSec; // 0x64
        float m_fAutoExitTime; // 0x68
        bool m_bDebugAnimPack; // 0x6C
        bool m_bQuit;
        bool m_bInitStatus;
        bool m_bAlwaysPack;
        bool m_bSimpleRepack;
        bool m_bAlwaysPackPathFinder;
        bool m_bFullScreen;
        bool m_bDisplayMemory;
        bool m_bDisplayTimeBar;
        bool m_bDisplayStatCounter;
        bool m_bStitchStrips;
        bool m_bWantBlood;
        bool m_bPathfinderInfo;
        bool m_bRunTimeCompileEnabled;
        bool m_bDisableOptions;
        bool m_bUseGameController;
        bool m_bUseDirectInputMouse;
        bool m_bUseDirectInputKeyboard;
        bool m_b3DStereo; // HBM Specific
        bool m_bPostFiltersAvailable;
        float m_fDirection; // 0x80
        int m_lResolution[2]; // 0x84
        int m_lResolutionWanted[2]; // 0x008C
        int m_lMainRenderResolution[2]; // 0x0094 same to m_lResolution
        int m_lBitsPerPixel; // 0x9C
        int m_lBitsPerPixelWanted; // 0xA0
        int m_lStartUpperLeftPos[2]; // 0xA4
        int m_lShadowDetail; // 0xAC
        int m_lTextureResolution[2]; // +0xB0 | Approved by ZSysInterface::ParseOptions
        ZEngineDataBase *m_pEngineData; //0x00B8 [VALID]
        MYSTR m_sSystemPath; //0x00BC
        MYSTR m_sProjectPath; //0x013C
        MYSTR m_sScenesPath; //0x01BC
        MYSTR m_sTexturePath; //0x023C
        MYSTR m_sDefaultScene; //0x02BC
        MYSTR m_sProjectFile; //0x033C
        MYSTR m_sActiveScriptDll; //0x03BC
        MYSTR m_sActiveScriptIfDll; //0x043C
        MYSTR m_sConfigFile; //0x04BC
        MYSTR m_sRecordFile; //0x053C
        MYSTR m_sPlayFile; //0x05BC
        MYSTR m_sStartRecordFile; //0x063C
        MYSTR m_sStartPlayFile; //0x06BC
        MYSTR m_sLocaleFile; //0x073C
        MYSTR m_sStreamFile; //0x07BC
        MYSTR m_sGameTitle; //0x083C
        MYSTR m_sLevelListFile; //0x08BC
        uint32_t m_iHeroControlMode; //0x093C
        ZVector4 m_v4ProgressBarRect; //0x0940
        bool m_bEthernetCableConnected; // 0x950
        bool m_bEthernetCableMessageVisible; // 0x951
        bool m_bMoviePlaying; // 0x952
        bool m_bRefsLocked; //0x0953
        bool m_bDisableLight; // 0x0954
        RE_ADD_PADDING(3);
        uint32_t m_ulSystemMemory; //0x958
        uint32_t m_ulVideoMemory; //0x95C
        int32_t PlotInfoEnable; //0x0960
        void *m_hInstance; // 0x0964
        void *m_hDllInstance; //0x0968
        void* MasterHwnd; //0x0968
        void* MainhWnd; //0x096C
        SETUPWINDOW m_pSetupWindow; //0x0970
        ZREF m_refZCamera; //0x0974
        ZRender* WindowFirst; //0x097C
        int32_t Alignlort; //0x0980
        MYSTR m_sDefaultMasterBank; //0x0984
        MYSTR m_sCmdLine; //0x0A04
        MYSTR m_sIniFile; //0x0A84
        bool m_bReloadRender; //0x0B04
        bool m_bTextures; //0x0B05
        bool m_bViewsLocked; //0x0B06
        bool m_bInitialized; //0x0B07
        bool m_bDisableConfig; //0x0B08
        RE_ADD_PADDING(3); //0x0B09
        ZDllBase *m_pSoundDll; //0x0B0C
        void* pUnused[2]; // 0x0B10 - it's ok
        bool m_bScriptEnableSanityCheck; //0x0B18 - need check
        bool m_bScriptDebug; //0x0B19 - need check
        bool m_bScriptDebugPrint; //0x0B1A - need check
        bool m_bRestoreOver; //0x0B1B - need check
        int32_t m_lIsActive; //0x0B1C
        bool m_bDoDeactivate; // 0x0B20
        float m_fTimeMultiplier_override; //0x0B24
        float m_fTimeMultiplier; //0x0B28
        uint8_t m_iTimeMultiplier_Locked; //0x0B2C
        RE_ADD_PADDING(3); // 0x0B2D
        int32_t m_lRandSeed; //0x0B30
        CConfiguration *m_pConfiguration; //0x0B34
        bool m_bSRandDone; //0x0B38
        bool m_bSaveRecordRuntime; //0x0B39
        bool m_bUsingRawProjectPath; //0x0B3A
        bool m_bFollowRealTime; //0x0B3B
        GeneralGameMode_e m_eGameMode; //0x0B3C
        ERunModes m_RunMode; //0x0B40
        bool m_bUseAudioInViewer; //0x0B44
        RE_ADD_PADDING(3); //0x0B45
    };
    // total size 0x0B48, but ZSysInterface never allocated directly, so we don't know actual size
    // As example, PS2 build says that ZSysInteface is 0xB9C
    // ZSysInterfaceWintel about 0x1700 bytes, but PC build have different inheritance layout:
    // PS2: ZSysInterfacePS2 -> ZSysInterface
    // PC: ZSysInterfaceWintel -> ZSysInterfacePack -> ZSysInterface

    RE_VERIFY_SIZE(ZSysInterface, 0xB48); // I'm not sure about that
    RE_VERIFY_OFFSET(ZSysInterface, m_fRealTime_, 0x10); // Verified by mem layout & XBox MiniNinjas
    RE_VERIFY_OFFSET(ZSysInterface, m_fRealTimeOffset_, 0x18); // Verified by mem layout & XBox MiniNinjas
    RE_VERIFY_OFFSET(ZSysInterface, m_fActualTime_, 0x20); // Verified by mem layout & XBox MiniNinjas
    RE_VERIFY_OFFSET(ZSysInterface, m_fRealTime, 0x28); // Verified by mem layout & XBox MiniNinjas
    RE_VERIFY_OFFSET(ZSysInterface, m_pEngineData, 0xB8); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sSystemPath, 0x00BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sProjectPath, 0x013C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sScenesPath, 0x01BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sTexturePath, 0x023C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sDefaultScene, 0x02BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sProjectFile, 0x033C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sActiveScriptDll, 0x03BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sActiveScriptIfDll, 0x043C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sConfigFile, 0x04BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sRecordFile, 0x053C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sPlayFile, 0x05BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sStartRecordFile, 0x063C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sStartPlayFile, 0x06BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sLocaleFile, 0x073C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sStreamFile, 0x07BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sGameTitle, 0x083C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_sLevelListFile, 0x08BC); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_iHeroControlMode, 0x093C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_v4ProgressBarRect, 0x0940); // Approved by in-mem analysis + verified by ZSysInterface::SetProgressBarRect
    RE_VERIFY_OFFSET(ZSysInterface, m_ulSystemMemory, 0x958);
    RE_VERIFY_OFFSET(ZSysInterface, m_ulVideoMemory, 0x95C);
    RE_VERIFY_OFFSET(ZSysInterface, PlotInfoEnable, 0x0960);
    RE_VERIFY_OFFSET(ZSysInterface, m_hInstance,  0x0964);
    RE_VERIFY_OFFSET(ZSysInterface, m_hDllInstance, 0x0968);
    RE_VERIFY_OFFSET(ZSysInterface, MasterHwnd, 0x096C);
    RE_VERIFY_OFFSET(ZSysInterface, MainhWnd, 0x0970);
    RE_VERIFY_OFFSET(ZSysInterface, m_pSetupWindow, 0x0974);
    RE_VERIFY_OFFSET(ZSysInterface, m_refZCamera, 0x0978);
    RE_VERIFY_OFFSET(ZSysInterface, m_pSoundDll, 0x0B0C); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_iTimeMultiplier_Locked, 0x0B2C); // Approved by ZSysInterface::Reset_TimeMultiplier_Lock
    RE_VERIFY_OFFSET(ZSysInterface, m_pConfiguration, 0x0B34); // Approved by in-mem analysis
    RE_VERIFY_OFFSET(ZSysInterface, m_bFollowRealTime, 0x0B3B); // Approved by ZSysInterface::FollowRealtime

    // Globals
    STATIC_GLOBAL_CLASS_INSTANCE(ZSysInterface*, g_pSysInterface);
}