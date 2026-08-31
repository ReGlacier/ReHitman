#include <Glacier/Audio/ZDllSound.h>
#include <Glacier/Audio/ZSynth.h>

namespace Glacier
{
    ZDllSound::ZDllSound()
        : m_pEventController(nullptr)
        , m_pSoundGraph(nullptr)
        , m_pMusicController(nullptr)
        , m_bMultiListeners(false)
        , m_ListenerMatrix{}
        , m_ListenerPosition{}
        , m_ListenerVelocity{}
        , m_pPlayerListener(nullptr)
        , m_pCameraListener(nullptr)
        , m_eListenerMode(LISTENER_MODE_DEFAULT)
        , m_pMemStream(nullptr)
        , m_lSynthCmdBufferSize(0x10000)
        , m_bStartMemStream(false)
        , m_bStopMemStream(false)
        , m_lFmvVolume(100)
        , m_bSoundInitialized(false)
        , m_pSynthCmdBuffer(ZSynth::GetSharedInputFrameBuffer())
        , m_pSynthCmd(m_pSynthCmdBuffer)
        , m_pPackedData(nullptr)
        , m_lPackedDataSize(0)
        , m_lPackedLookupCapacity(1000)
        , m_bPackedLookupReady(false)
    {
    }

    void ZDllSound::AppendSourceCommand(ZSoundObject* _object, ZREF _soundRef, SWave* _wave)
    {
        if (!_object || !_wave || !m_pSynthCmd)
            return;

        auto* command = reinterpret_cast<SStartSoundBase*>(m_pSynthCmd);
        command->m_lFlags = 0;
        if (_object->m_eState == STATE_STARTPLAY)
        {
            _object->m_eState = STATE_PLAYING;
            command->m_lFlags |= 1;
        }
        if ((_object->m_lSoundFlags & 0x10000000) != 0)
            command->m_lFlags |= 2;

        if (_object->m_lBufferId < 0)
            _object->m_lBufferId = 127;
        command->m_lBufferId = _object->m_lBufferId;
        command->m_lSndRef = _soundRef;
        command->m_lPrio = static_cast<uint32_t>(_object->m_fPrio);
        command->m_lHeaderOffset = _wave->m_lHeaderOffset & ~1u;
        command->m_bLooping = (_object->m_lSoundFlags >> 3) & 1;
        command->m_lPathIdx = _object->m_lNumPathes > 0 ? _object->m_PathIndices[0] : -1;
        command->m_lPause = 0;
        command->m_lBufferType = _object->m_eSourceType;
        command->m_lStartOffset = static_cast<uint32_t>(_object->m_fStartOffset * 256.0f);
        command->m_lLowpassEnabled = (_object->m_lSoundFlags & 0x200000) != 0;

        AddSourceCommand(_object, command);
        m_pSynthCmd += command->m_lSize;
    }

    ZSoundObject* ZDllSound::AllocateObject()
    {
        return m_ObjectManager.ConvRefToPtr(m_ObjectManager.AllocateObject());
    }

    void ZDllSound::DeleteSnd(ZREF _soundRef)
    {
        RemovePlayBuffer(_soundRef);
        m_ObjectManager.FreeObject(_soundRef);
    }

    void ZDllSound::DeleteSoundPtr(ZSoundObject* _object)
    {
        if (_object)
        {
            RemovePlayBuffer(_object->m_lKey);
            m_ObjectManager.FreeObject(_object->m_lKey);
        }
    }

    ZSoundObject* ZDllSound::SRefToPtr(ZREF _soundRef)
    {
        return m_ObjectManager.ConvRefToPtr(_soundRef);
    }

    ZREF ZDllSound::SPtrToRef(ZSoundObject* _object)
    {
        return _object ? _object->m_lKey : 0;
    }

    uint32_t ZDllSound::AddToRefChain(uint32_t _chain, ZREF _soundRef)
    {
        return m_AllocRef.AddToChain(_chain, _soundRef);
    }

    void ZDllSound::FreeRefChain(uint32_t _chain)
    {
        m_AllocRef.FreeChain(_chain);
    }

    void ZDllSound::DeleteRefChain(uint32_t _chain)
    {
        m_AllocRef.FreeChain(_chain);
    }

    void ZDllSound::EnableMultiListeners(bool _enabled)
    {
        m_bMultiListeners = _enabled;
    }

    bool ZDllSound::ListenerEnable(int _index, ZREF _directionRef, ZREF _positionRef)
    {
        if (_index == 4)
            return false;
        m_Listeners[_index].m_rDirObj = _directionRef;
        m_Listeners[_index].m_rPosObj = _positionRef;
        m_Listeners[_index].m_bActive = true;
        return true;
    }

    bool ZDllSound::ListenerDisable(int _index)
    {
        if (_index >= 4)
            return false;
        m_Listeners[_index].m_bActive = false;
        return true;
    }

    void ZDllSound::EnableDiscWarning(bool _enabled)
    {
        *reinterpret_cast<bool*>(reinterpret_cast<char*>(this) + 0x5098) = _enabled;
    }

    bool ZDllSound::IsPaused()
    {
        return *reinterpret_cast<bool*>(reinterpret_cast<char*>(this) + 0x509A);
    }

    void ZDllSound::SetMusicController(ZMusicControllerBase* _controller)
    {
        m_pMusicController = _controller;
    }

    ZMusicControllerBase* ZDllSound::GetMusicController()
    {
        return m_pMusicController;
    }

    ZSoundGraph* ZDllSound::GetGraph()
    {
        return m_pSoundGraph;
    }

    ZAudioTypeBase::ZPackedBase* ZDllSound::GetPackedObject(int _index)
    {
        if (_index <= 0 || !m_pPackedData ||
            static_cast<uint32_t>(_index) + sizeof(ZAudioTypeBase::ZPackedBase) > m_lPackedDataSize)
            return nullptr;
        auto* packed = reinterpret_cast<ZAudioTypeBase::ZPackedBase*>(m_pPackedData + _index);
        return packed->m_Type <= ZAudioTypes::Layered ? packed : nullptr;
    }

    SWave* ZDllSound::GetWave(ZAudioTypeBase::ZPackedBase* _packed)
    {
        if (!_packed || _packed->m_Type != ZAudioTypes::Sound)
            return nullptr;
        return static_cast<SWave*>(GetPackedObject(_packed->GetAs<SSound>().m_lWaveOffset));
    }

    float ZDllSound::GetWaveDuration(SWave* _wave)
    {
        return _wave ? _wave->m_fDuration : 0.0f;
    }

    float ZDllSound::GetPitchChange(int _pitch)
    {
        return _pitch >= 0 ? _pitch * 0.00083333335f + 1.0f :
            1.0f / (_pitch * -0.00083333335f + 1.0f);
    }
}
