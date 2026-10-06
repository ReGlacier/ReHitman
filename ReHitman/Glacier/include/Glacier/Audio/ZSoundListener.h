#pragma once

#include <Glacier/Audio/ZSoundBase.h>

namespace Glacier
{
    class ZSynth;
    struct SSynthCmdBase;

    class ZSoundListener : public ZSoundBase
    {
    public:
        // vtbl
        ~ZSoundListener() override;
        virtual bool Init() = 0;
        virtual void Free() = 0;
        virtual bool SetEnvSize(float _size);
        virtual void SetEnvironment(int _environment);
        virtual void SetEnv(ZREF _environment);
        virtual ZREF GetEnv() const;
        virtual void Update(SSynthCmdBase* _command);

        // methods
        ZSoundListener();

        // members
        ZSynth* m_pSynth;
        RE_ADD_PADDING(4);
        ZREF m_rEnvironment;
    };

    RE_VERIFY_OFFSET(ZSoundListener, m_pSynth, 0x08);
    RE_VERIFY_OFFSET(ZSoundListener, m_rEnvironment, 0x10);
    RE_VERIFY_SIZE(ZSoundListener, 0x14);
}
