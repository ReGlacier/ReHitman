#include <Glacier/Audio/ZSoundDllBase.h>

namespace Glacier
{
    ZSoundDllBase::ZSoundDllBase()
        : m_bClosing(false)
        , m_pPlayer(nullptr)
    {
    }

    ZSoundDllBase::~ZSoundDllBase() = default;

    void ZSoundDllBase::Init2()
    {
    }

    void ZSoundDllBase::SetDefaultEnv(ZREF)
    {
    }

    void ZSoundDllBase::SetTemporaryEnv(ZREF)
    {
    }

    void ZSoundDllBase::SetPlayerListener(ZGEOM& _listener)
    {
        SetPlayer(&_listener);
    }

    void ZSoundDllBase::SetCameraListener(ZGEOM&)
    {
    }

    void ZSoundDllBase::SetDefaultCameraListener()
    {
    }

    void ZSoundDllBase::SetMusicController(ZMusicControllerBase*)
    {
    }

    bool ZSoundDllBase::Closing() const
    {
        return m_bClosing;
    }

    ZGEOM* ZSoundDllBase::GetPlayer() const
    {
        return m_pPlayer;
    }

    void ZSoundDllBase::SetPlayer(ZGEOM* _player)
    {
        m_pPlayer = _player;
    }
}
