#include <Glacier/Audio/ZDllSound.h>

namespace Glacier
{
    ZDllSound::ZDllSound()
        : m_pEventController(nullptr)
        , m_pSoundGraph(nullptr)
        , m_pMusicController(nullptr)
        , m_lPackedLookupCapacity(1000)
        , m_bPackedLookupReady(false)
    {
    }

    ZSoundObject* ZDllSound::AllocateObject()
    {
        return m_ObjectManager.ConvRefToPtr(m_ObjectManager.AllocateObject());
    }

    void ZDllSound::DeleteSnd(ZREF _soundRef)
    {
        m_ObjectManager.FreeObject(_soundRef);
    }

    void ZDllSound::DeleteSoundPtr(ZSoundObject* _object)
    {
        if (_object)
            m_ObjectManager.FreeObject(_object->m_lKey);
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
        *reinterpret_cast<bool*>(reinterpret_cast<char*>(this) + 0x5010) = _enabled;
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

    ZAudioTypeBase::ZPackedBase* ZDllSound::GetPackedObject(int)
    {
        // TODO: Finish this place after Audio::ZPackedData will be reversed
        return nullptr;
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
