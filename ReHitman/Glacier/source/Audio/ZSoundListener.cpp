#include <Glacier/Audio/ZSoundListener.h>

namespace Glacier
{
    ZSoundListener::ZSoundListener()
    {
        m_rOwner = 0;
        m_rEnvironment = 0;
    }

    ZSoundListener::~ZSoundListener() = default;

    bool ZSoundListener::SetEnvSize(float)
    {
        return true;
    }

    void ZSoundListener::SetEnvironment(int)
    {
    }

    void ZSoundListener::SetEnv(ZREF _environment)
    {
        m_rEnvironment = _environment;
    }

    ZREF ZSoundListener::GetEnv() const
    {
        return m_rEnvironment;
    }

    void ZSoundListener::Update(SSynthCmdBase*)
    {
    }
}
