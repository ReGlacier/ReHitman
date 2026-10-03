#pragma once

#include <Glacier/Audio/Eax3.h>
#include <Glacier/Audio/ZEaxBase.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    class ZEaxListener : public ZEaxBase
    {
    public:
        // vtbl
        ~ZEaxListener() override;
        void Update() override;
        void GetDefaultValues() override;
        virtual bool Init(IDirectSound* _directSound, IDirectSoundBuffer* _buffer, bool _setDefaults);
        virtual bool SetRoom(int32_t _room);
        virtual bool SetEnvSize(float _size);
        virtual void SetEnv(ZREF _environment);
        virtual int32_t SetEnvValue(int32_t _current, int32_t _target, int32_t _step);
        virtual float SetEnvValue(float _current, float _target, float _step);

        // methods
        ZEaxListener();

        // members
        IDirectSoundBuffer* m_pSoundBuffer;
        ZREF m_rEnvironment;
        EaxListenerProperties m_Properties;
        EaxListenerProperties m_ChangeRates;
    };

    RE_VERIFY_OFFSET(ZEaxListener, m_pSoundBuffer, 0x08);
    RE_VERIFY_OFFSET(ZEaxListener, m_rEnvironment, 0x0C);
    RE_VERIFY_OFFSET(ZEaxListener, m_Properties, 0x10);
    RE_VERIFY_OFFSET(ZEaxListener, m_ChangeRates, 0x80);
    RE_VERIFY_SIZE(ZEaxListener, 0xF0);
}
