#pragma once

#include <Glacier/Audio/ZSoundListenerWintel.h>

struct IDirectSound3DListener;
struct IDirectSoundBuffer;

namespace Glacier
{
    class _ZSoundBufferDS;
    class ZEaxListener;

    class ZSoundListenerDS : public ZSoundListenerWintel
    {
    public:
        // vtbl
        ~ZSoundListenerDS() override;
        bool Init() override;
        void Free() override;
        bool SetEnvSize(float _size) override;
        void SetEnvironment(int _environment) override;
        void SetEnv(ZREF _environment) override;
        void Update(SSynthCmdBase* _command) override;
        virtual void SetPrimary(IDirectSoundBuffer* _primary);

        // methods
        ZSoundListenerDS();

        // members
        IDirectSound3DListener* m_pDSListener;
        IDirectSoundBuffer* m_pPrimary;
        _ZSoundBufferDS* m_pEaxBuffer;
        ZEaxListener* m_pEaxListener;
    };

    RE_VERIFY_OFFSET(ZSoundListenerDS, m_pDSListener, 0x44);
    RE_VERIFY_OFFSET(ZSoundListenerDS, m_pPrimary, 0x48);
    RE_VERIFY_OFFSET(ZSoundListenerDS, m_pEaxBuffer, 0x4C);
    RE_VERIFY_OFFSET(ZSoundListenerDS, m_pEaxListener, 0x50);
    RE_VERIFY_SIZE(ZSoundListenerDS, 0x54);
}
