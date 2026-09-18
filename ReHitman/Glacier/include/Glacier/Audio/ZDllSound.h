#pragma once

#include <Glacier/Audio/ZSoundBuffer.h>
#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/Audio/ZLocationController.h>
#include <Glacier/Audio/ZSoundObjectManager.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/ZRefAlloc.h>

namespace Glacier
{
    class CEVENTCONTROLLER;
    class LINKSORTREFTAB;
    class ZDrawDebugRender;
    class ZMemStream;

    class ZDllSound : public ZSoundDllBase
    {
    public:
        // vtbl
        ~ZDllSound() override;
        void Init() override;
        void End() override;
        void PushScene(const char* _scene) override;
        void PopScene() override;
        void AllocSequenceStart() override;
        void AllocSequenceEnd() override;
        void InsertStart() override;
        void InsertEnd() override;
        void CrashFree() override;
        void PrintStatus() override;
        void Initialize() override;
        void Init2() override;
        ZSoundObject* AllocateObject() override;
        bool AddNewPlayBuffer(ZSoundObject* _object) override;
        bool AddNewPlayBuffer(ZREF _soundRef) override;
        void RemovePlayBuffer(ZREF _soundRef) override;
        void CalcPriority(ZSoundObject* _object) override;
        ZAudioTypeBase::ZPackedBase* GetPackedObject(int _index) override;
        uint32_t GetMapping(uint32_t _first, uint32_t _second) override;
        void AdjustExit(ZROOM* _room, int _exitIndex, float _value) override;
        SExit* GetExit(uint32_t _index) override;
        void RegisterAudioEvents(SAudioEvent* _events, SAudioEventNoiseLevel* _noiseLevels) override;
        void SetMaxGraphReflections(uint8_t _count) override;
        void ActorRegister(ZGEOM* _actor) override;
        void ActorRemove(ZGEOM* _actor) override;
        void InstallSoundGraph(char* _data, int _size) override;
        void InitializeSoundGraph() override;
        void InstallWavesWait() override;
        bool InstallWaves(int _index, const char* _fileName) override;
        bool InstallSounds(char* _data, uint32_t _size) override;
        bool InstallStreamWaves(int _index, const char* _fileName) override;
        bool InstallWaveHeaders(int _index, const char* _fileName) override;
        void InitFrame() override;
        bool RenderFrame() override;
        void DispatchSoundEvents() override;
        char* TEMPCALCSOUNDGRAPH(uint32_t* _size) override;
        ZSoundGraph* GetGraph() override;
        void SetDefaultEnv(ZREF _environment) override;
        void SetTemporaryEnv(ZREF _environment) override;
        void MuteAllWaves(float _volume) override;
        bool AddEvent(ZGROUP* _group, float* _position, int _eventIndex,
            uint32_t _arg0, uint32_t _arg1, uint32_t _arg2) override;
        void SetPlayerListener(ZGEOM& _listener) override;
        ZGEOM* GetPlayerListener() override;
        void SetCameraListener(ZGEOM& _listener) override;
        void SetDefaultCameraListener() override;
        ZGEOM* GetCameraListener() override;
        EListenerMode GetListenerMode() override;
        bool IsListenerFreeCam(ZGEOM* _listener) override;
        void SetMusicController(ZMusicControllerBase* _controller) override;
        ZMusicControllerBase* GetMusicController() override;
        void InitSoundPack() override;
        void InitSoundPackPS2() override;
        void InitSoundPackXBox() override;
        void InitSoundPackGC() override;
        void InitSoundPackXenon() override;
        bool IsPlayingMono(ZREF _soundRef) override;
        void EnableMultiListeners(bool _enabled) override;
        bool ListenerEnable(int _index, ZREF _positionRef, ZREF _directionRef) override;
        bool ListenerDisable(int _index) override;
        void SetFmvVolume(int _volume) override;
        void SetMasterVolume(int _volume) override;
        float GetPitchChange(int _pitch) override;
        void DeleteSnd(ZREF _soundRef) override;
        void DeleteSoundPtr(ZSoundObject* _object) override;
        ZSoundObject* AddSound(int _soundIndex, ZGEOM* _owner,
            float* _direction, float* _position) override;
        ZREF AddSound3d(ZGEOM* _owner, int _soundIndex,
            float* _direction, float* _position) override;
        ZREF AddSound2d(int _soundIndex) override;
        ZSoundObject* SRefToPtr(ZREF _soundRef) override;
        ZREF SPtrToRef(ZSoundObject* _object) override;
        uint32_t AddToRefChain(uint32_t _chain, ZREF _soundRef) override;
        void FreeRefChain(uint32_t _chain) override;
        void NotifyStopped(ZREF _soundRef, SSndMsg* _message) override;
        void NotifyStarted(ZREF _soundRef, SSourceStarted* _message) override;
        void DeleteRefChain(uint32_t _chain) override;
        void GetRefChainObjects(uint32_t _chain, REFTAB32* _objects) override;
        void Pause(bool _pause, bool _mute) override;
        bool IsPaused() override;
        void EnableDiscWarning(bool _enabled) override;
        bool MemStreamCreate(char* _name) override;
        void MemStreamDestroy() override;
        int32_t MemStreamPushData(char* _data, int _size) override;
        int32_t MemStreamBufferSize() override;
        ZROOM* GetListenerRoom() override;
        virtual void DrawSources(ZDrawDebugRender* _renderer);
        virtual bool RenderFrame(REFTAB32& _sources);
        virtual SWave* GetWave(ZAudioTypeBase::ZPackedBase* _packed);
        virtual float GetWaveDuration(int _soundIndex);
        virtual float GetWaveDuration(SWave* _wave);
        virtual bool InstallSynthesizer() = 0;
        virtual void CalculateSoundGraph();
        virtual char* GetSoundGraph(uint32_t* _size);
        virtual char* GetRemapTable(uint32_t* _size);
        virtual void Reset();
        virtual void SetAllVolumes(float _volume);
        virtual int32_t IsDVDReady();
        virtual void SendDVDReadyRequest();
        virtual void SendDVDUnlockRequest();
        virtual bool RenderFrameMulti(REFTAB32& _sources, LINKSORTREFTAB& _priorities);
        virtual int32_t SetMasterPan(int _pan);
        virtual void UpdateBufferVolumes();
        virtual int32_t CalcVolume(float _volume);
        virtual int32_t CalcVolume(float _volume, float _distance, float _maximum);
        virtual float CalcDelayTime(float _distance);
        virtual ZGROUP* ParentRoom(ZGEOM* _geom);
        virtual ZGROUP* GetCurrentRoom(ZGEOM* _geom);
        virtual int32_t GetMemStreamPlayCursor();
        virtual void TransferMemStreamData(char* _data, int _size);
        virtual void InitAudioStream();
        virtual void StartAudioStream();
        virtual void StopAudioStream();
        virtual bool RunAudioStream();
        virtual bool SetEnvironment();
        virtual void StopBuffers();
        virtual void SendSynthCmd(int _command);
        virtual bool SendCmdFrameBuffer();
        virtual void GetDeletedSounds(REFTAB32& _deleted, REFTAB32& _retained);
        virtual void CalcPrioTab(REFTAB32& _sources, LINKSORTREFTAB& _priorities);
        virtual void GetAudiable(REFTAB32& _sources, REFTAB32& _audible,
            REFTAB32& _inaudible, REFTAB32& _deleted);
        virtual void BuildFrame(LINKSORTREFTAB& _priorities,
            REFTAB32& _started, REFTAB32& _stopped);
        virtual void AddFilterChainCommands();
        virtual void AddSourceCommands(LINKSORTREFTAB& _sources);
        virtual void AddSourceCommand(ZSoundObject* _object, SStartSoundBase* _command) = 0;
        virtual void AddListenerCommands() = 0;
        virtual void FreeSynth() = 0;
        virtual char* GetSynthCmdArray();
        virtual void ProcessSynthCmdArray();
        virtual bool ProcessSynthCmd(const char* _command);
        virtual bool SynthModuleReady() = 0;
        virtual void ResetSynthReady();
        virtual void SignalSoundEngineReady();
        virtual bool FrameReady();

        // methods
        ZDllSound();
        void AppendSourceCommand(ZSoundObject* _object, ZREF _soundRef, SWave* _wave);

        // members
        ZSoundObjectManager m_ObjectManager;
        ZLocationController m_LocationController;
        SListener m_Listeners[4];
        ZRefAlloc m_AllocRef;
        MYSTR m_sCurrentScene;
        RE_ADD_PADDING(0x04);
        bool m_bMultiListeners;
        RE_ADD_PADDING(3);
        ZMat3x3 m_ListenerMatrix;
        ZVector3 m_ListenerPosition;
        ZVector3 m_ListenerVelocity;
        ZGEOM* m_pPlayerListener;
        ZGEOM* m_pCameraListener;
        EListenerMode m_eListenerMode;
        void* m_pMemStream;
        RE_ADD_PADDING(0x08);
        int32_t m_lSynthCmdBufferSize;
        bool m_bStartMemStream;
        RE_ADD_PADDING(1);
        bool m_bStopMemStream;
        RE_ADD_PADDING(0x19);
        int32_t m_lFmvVolume;
        RE_ADD_PADDING(0x14);
        bool m_bSoundInitialized;
        RE_ADD_PADDING(0x0B);
        char* m_pSynthCmdBuffer;
        char* m_pSynthCmd;
        RE_ADD_PADDING(0x14);
        CEVENTCONTROLLER* m_pEventController;
        ZSoundGraph* m_pSoundGraph;
        ZMusicControllerBase* m_pMusicController;
        REFTAB32 m_RefTabA;
        REFTAB32 m_RefTabB;
        char* m_pPackedData;
        uint32_t m_lPackedDataSize;
        int32_t m_lPackedLookupCapacity;
        RE_ADD_PADDING(0x3E80);
        bool m_bPackedLookupReady;
        RE_ADD_PADDING(0x0F);
    };

    RE_VERIFY_OFFSET(ZDllSound, m_ObjectManager, 0x18);
    RE_VERIFY_OFFSET(ZDllSound, m_LocationController, 0x82C);
    RE_VERIFY_OFFSET(ZDllSound, m_Listeners, 0x904);
    RE_VERIFY_OFFSET(ZDllSound, m_AllocRef, 0x944);
    RE_VERIFY_OFFSET(ZDllSound, m_sCurrentScene, 0x4F8C);
    RE_VERIFY_OFFSET(ZDllSound, m_bMultiListeners, 0x5010);
    RE_VERIFY_OFFSET(ZDllSound, m_ListenerMatrix, 0x5014);
    RE_VERIFY_OFFSET(ZDllSound, m_ListenerPosition, 0x5038);
    RE_VERIFY_OFFSET(ZDllSound, m_pPlayerListener, 0x5050);
    RE_VERIFY_OFFSET(ZDllSound, m_pCameraListener, 0x5054);
    RE_VERIFY_OFFSET(ZDllSound, m_eListenerMode, 0x5058);
    RE_VERIFY_OFFSET(ZDllSound, m_pMemStream, 0x505C);
    RE_VERIFY_OFFSET(ZDllSound, m_lSynthCmdBufferSize, 0x5068);
    RE_VERIFY_OFFSET(ZDllSound, m_bStartMemStream, 0x506C);
    RE_VERIFY_OFFSET(ZDllSound, m_bStopMemStream, 0x506E);
    RE_VERIFY_OFFSET(ZDllSound, m_lFmvVolume, 0x5088);
    RE_VERIFY_OFFSET(ZDllSound, m_bSoundInitialized, 0x50A0);
    RE_VERIFY_OFFSET(ZDllSound, m_pSynthCmdBuffer, 0x50AC);
    RE_VERIFY_OFFSET(ZDllSound, m_pSynthCmd, 0x50B0);
    RE_VERIFY_OFFSET(ZDllSound, m_pEventController, 0x50C8);
    RE_VERIFY_OFFSET(ZDllSound, m_RefTabA, 0x50D4);
    RE_VERIFY_OFFSET(ZDllSound, m_pPackedData, 0x522C);
    RE_VERIFY_OFFSET(ZDllSound, m_lPackedDataSize, 0x5230);
    RE_VERIFY_OFFSET(ZDllSound, m_lPackedLookupCapacity, 0x5234);
    RE_VERIFY_OFFSET(ZDllSound, m_bPackedLookupReady, 0x90B8);
    RE_VERIFY_SIZE(ZDllSound, 0x90C8);
}
