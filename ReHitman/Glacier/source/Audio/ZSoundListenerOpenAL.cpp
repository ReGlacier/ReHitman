#include <Glacier/Audio/ZSoundListenerOpenAL.h>

namespace Glacier
{
    ZSoundListenerOpenAL::~ZSoundListenerOpenAL() = default;

    bool ZSoundListenerOpenAL::Init()
    {
        return true;
    }

    void ZSoundListenerOpenAL::Free()
    {
    }

    void ZSoundListenerOpenAL::Update(SSynthCmdBase* _command)
    {
        const auto* vectors = reinterpret_cast<const ZVector3*>(
            reinterpret_cast<const char*>(_command) + 0x08);
        m_vFront = vectors[0];
        m_vTop = vectors[1];
        m_vEar = vectors[2];
        m_vPosition = vectors[3];

        // TODO: Finish this place after ZSynthOpenAL will be reversed
    }
}
