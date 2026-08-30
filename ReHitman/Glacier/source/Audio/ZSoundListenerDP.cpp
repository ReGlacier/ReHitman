#include <Glacier/Audio/ZSoundListenerDP.h>

namespace Glacier
{
    namespace
    {
        void CopyListenerVectors(ZSoundListener3D* _listener, const SSynthCmdBase* _command)
        {
            const auto* vectors = reinterpret_cast<const ZVector3*>(
                reinterpret_cast<const char*>(_command) + 0x08);
            _listener->m_vFront = vectors[0];
            _listener->m_vTop = vectors[1];
            _listener->m_vEar = vectors[2];
            _listener->m_vPosition = vectors[3];
        }
    }

    ZSoundListenerDP::~ZSoundListenerDP() = default;

    bool ZSoundListenerDP::Init()
    {
        return true;
    }

    void ZSoundListenerDP::Free()
    {
    }

    void ZSoundListenerDP::Update(SSynthCmdBase* _command)
    {
        CopyListenerVectors(this, _command);
        // TODO: Finish this place after ZSynthDP will be reversed
    }
}
