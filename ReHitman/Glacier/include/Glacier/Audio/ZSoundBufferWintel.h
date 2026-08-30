#pragma once

#include <Glacier/Audio/ZSoundBuffer.h>
#include <Glacier/Audio/ZVorbisStream.h>

namespace Glacier
{
    class ZSoundBufferWintel : public _ZSoundBuffer
    {
    public:
        explicit ZSoundBufferWintel(ZSynth* _synth);
        ~ZSoundBufferWintel() override = default;

        bool Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags) override;
        void Update(SStartSoundBase* _command) override;
        void Stop() override;
        int CopyWaveData() override;

        virtual int GetNumStreamSamples();
        virtual void Update(SStartSoundBFormat* _command);
        virtual void Update(SStartSound3D* _command) = 0;
        virtual void Update(SStartSound2D* _command) = 0;
        virtual void Update(SStartSound* _command) = 0;
        virtual void UpdateFilters();
        virtual void UnlockBuffer(void* _buffer1, int _size1, void* _buffer2, int _size2) = 0;
        virtual bool LockBuffer(int _offset, int _size, void** _buffer1, int* _size1,
            void** _buffer2, int* _size2) = 0;
        virtual int Calc3DPan(SStartSound3D* _command);
        virtual int CalcBFormatPan(SStartSoundBFormat* _command);

        // methods
        void ApplyLowpass(int16_t* _data, int _size);

        // members
        int32_t m_lLowpassHistoryLeft;
        int32_t m_lLowpassHistoryRight;
        RE_ADD_PADDING(4);
        ZVorbisStream m_VorbisStream;
        int32_t m_lStreamRequestSize;
        int32_t m_field63E4;
        float m_fVolumePercent;
        int32_t m_field63EC;
        int32_t m_lCurrentStreamSegment;
    };

    RE_VERIFY_OFFSET(ZSoundBufferWintel, m_lLowpassHistoryLeft, 0x40C4);
    RE_VERIFY_OFFSET(ZSoundBufferWintel, m_VorbisStream, 0x40D0);
    RE_VERIFY_OFFSET(ZSoundBufferWintel, m_lStreamRequestSize, 0x63E0);
    RE_VERIFY_OFFSET(ZSoundBufferWintel, m_fVolumePercent, 0x63E8);
    RE_VERIFY_OFFSET(ZSoundBufferWintel, m_lCurrentStreamSegment, 0x63F0);
    RE_VERIFY_SIZE(ZSoundBufferWintel, 0x63F4);
}
