#pragma once

#include <Glacier/Audio/ZAudioType.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/System/ZDllBase.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    class LINKSORTREFTAB;
    class REFTAB32;
    class ZGEOM;
    class ZGROUP;
    class ZMusicControllerBase;
    class ZROOM;
    class ZSoundGraph;
    class ZSoundObject;
    struct SAudioEvent;
    struct SAudioEventNoiseLevel;
    struct SExit;
    struct SSndMsg;
    struct SSourceStarted;

    enum EListenerMode : int32_t
    {
        LISTENER_MODE_DEFAULT = 0
    };

    class ZSoundDllBase : public ZDllBase, public ZSerializable
    {
    public:
        // vtbl
        ~ZSoundDllBase() override;
        virtual void Initialize() = 0;
        virtual void Init2();
        virtual ZSoundObject* AllocateObject() = 0;
        virtual bool AddNewPlayBuffer(ZSoundObject* _object) = 0;
        virtual bool AddNewPlayBuffer(ZREF _soundRef) = 0;
        virtual void RemovePlayBuffer(ZREF _soundRef) = 0;
        virtual void CalcPriority(ZSoundObject* _object) = 0;
        virtual ZAudioTypeBase::ZPackedBase* GetPackedObject(int _index) = 0;
        virtual uint32_t GetMapping(uint32_t _first, uint32_t _second) = 0;
        virtual void AdjustExit(ZROOM* _room, int _exitIndex, float _value) = 0;
        virtual SExit* GetExit(uint32_t _index) = 0;
        virtual void RegisterAudioEvents(SAudioEvent* _events, SAudioEventNoiseLevel* _noiseLevels) = 0;
        virtual void SetMaxGraphReflections(uint8_t _count) = 0;
        virtual void ActorRegister(ZGEOM* _actor) = 0;
        virtual void ActorRemove(ZGEOM* _actor) = 0;
        virtual void InstallSoundGraph(char* _data, int _size) = 0;
        virtual void InitializeSoundGraph() = 0;
        virtual void InstallWavesWait() = 0;
        virtual bool InstallWaves(int _index, const char* _fileName) = 0;
        virtual bool InstallSounds(char* _data, uint32_t _size) = 0;
        virtual bool InstallStreamWaves(int _index, const char* _fileName) = 0;
        virtual bool InstallWaveHeaders(int _index, const char* _fileName) = 0;
        virtual void InitFrame() = 0;
        virtual bool RenderFrame() = 0;
        virtual void DispatchSoundEvents() = 0;
        virtual char* TEMPCALCSOUNDGRAPH(uint32_t* _size) = 0;
        virtual ZSoundGraph* GetGraph() = 0;
        virtual void SetDefaultEnv(ZREF _environment);
        virtual void SetTemporaryEnv(ZREF _environment);
        virtual void MuteAllWaves(float _volume) = 0;
        virtual bool AddEvent(ZGROUP* _group, float* _position, int _eventIndex,
            uint32_t _arg0, uint32_t _arg1, uint32_t _arg2) = 0;
        virtual void SetPlayerListener(ZGEOM& _listener);
        virtual ZGEOM* GetPlayerListener() = 0;
        virtual void SetCameraListener(ZGEOM& _listener);
        virtual void SetDefaultCameraListener();
        virtual ZGEOM* GetCameraListener() = 0;
        virtual EListenerMode GetListenerMode() = 0;
        virtual bool IsListenerFreeCam(ZGEOM* _listener) = 0;
        virtual void SetMusicController(ZMusicControllerBase* _controller);
        virtual ZMusicControllerBase* GetMusicController() = 0;
        virtual void InitSoundPack() = 0;
        virtual void InitSoundPackPS2() = 0;
        virtual void InitSoundPackXBox() = 0;
        virtual void InitSoundPackGC() = 0;
        virtual void InitSoundPackXenon() = 0;
        virtual bool IsPlayingMono(ZREF _soundRef) = 0;
        virtual void EnableMultiListeners(bool _enabled) = 0;
        virtual bool ListenerEnable(int _index, ZREF _positionRef, ZREF _directionRef) = 0;
        virtual bool ListenerDisable(int _index) = 0;
        virtual void SetFmvVolume(int _volume) = 0;
        virtual void SetMasterVolume(int _volume) = 0;
        virtual float GetPitchChange(int _pitch) = 0;
        virtual void DeleteSnd(ZREF _soundRef) = 0;
        virtual void DeleteSoundPtr(ZSoundObject* _object) = 0;
        virtual ZSoundObject* AddSound(int _soundIndex, ZGEOM* _owner,
            float* _direction, float* _position) = 0;
        virtual ZREF AddSound3d(ZGEOM* _owner, int _soundIndex,
            float* _direction, float* _position) = 0;
        virtual ZREF AddSound2d(int _soundIndex) = 0;
        virtual ZSoundObject* SRefToPtr(ZREF _soundRef) = 0;
        virtual ZREF SPtrToRef(ZSoundObject* _object) = 0;
        virtual uint32_t AddToRefChain(uint32_t _chain, ZREF _soundRef) = 0;
        virtual void FreeRefChain(uint32_t _chain) = 0;
        virtual void NotifyStopped(ZREF _soundRef, SSndMsg* _message) = 0;
        virtual void NotifyStarted(ZREF _soundRef, SSourceStarted* _message) = 0;
        virtual void DeleteRefChain(uint32_t _chain) = 0;
        virtual void GetRefChainObjects(uint32_t _chain, REFTAB32* _objects) = 0;
        virtual void Pause(bool _pause, bool _mute) = 0;
        virtual bool IsPaused() = 0;
        virtual void EnableDiscWarning(bool _enabled) = 0;
        virtual bool MemStreamCreate(char* _name) = 0;
        virtual void MemStreamDestroy() = 0;
        virtual int32_t MemStreamPushData(char* _data, int _size) = 0;
        virtual int32_t MemStreamBufferSize() = 0;
        virtual ZROOM* GetListenerRoom() = 0;

        // methods
        ZSoundDllBase();
        bool Closing() const;
        ZGEOM* GetPlayer() const;

    protected:
        void SetPlayer(ZGEOM* _player);

    public:
        // members
        bool m_bClosing;
        RE_ADD_PADDING(3);
        ZGEOM* m_pPlayer;
    };

    RE_VERIFY_OFFSET(ZSoundDllBase, m_bClosing, 0x10);
    RE_VERIFY_OFFSET(ZSoundDllBase, m_pPlayer, 0x14);
    RE_VERIFY_SIZE(ZSoundDllBase, 0x18);
}
