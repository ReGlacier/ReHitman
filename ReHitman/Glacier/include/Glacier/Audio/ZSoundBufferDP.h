#pragma once

#include <Glacier/Audio/ZSoundBufferWintel.h>

namespace Glacier
{
    struct IDieselPowerSoundSource;

    class ZSoundBufferDP : public ZSoundBufferWintel
    {
    public:
        explicit ZSoundBufferDP(ZSynth* _synth);
        ~ZSoundBufferDP() override;

        bool NeedData(int* _needed) override;
        bool Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags) override;
        void Start() override;
        void Stop() override;
        _ZSoundBuffer* Duplicate() override;
        int GetPlayCursor() override;
        void Free() override;
        void Resume() override;
        void Pause() override;
        void Update(SStartSound2D* _command) override;
        void Update(SStartSound3D* _command) override;
        void Update(SStartSoundBFormat* _command) override;
        void Update(SStartSound* _command) override;
        void UpdateFilters() override;
        void UnlockBuffer(void* _buffer1, int _size1, void* _buffer2, int _size2) override;
        bool LockBuffer(int _offset, int _size, void** _buffer1, int* _size1,
            void** _buffer2, int* _size2) override;

        IDieselPowerSoundSource* m_pDPVoice;
    };

    RE_VERIFY_OFFSET(ZSoundBufferDP, m_pDPVoice, 0x63F4);
    RE_VERIFY_SIZE(ZSoundBufferDP, 0x63F8);
}
