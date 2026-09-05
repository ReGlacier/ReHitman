#pragma once

#include <Glacier/Audio/ZSoundBufferWintel.h>

struct IDirectSoundBuffer;
struct IDirectSound3DBuffer;

namespace Glacier
{
    class ZEaxSource;

    class _ZSoundBufferDS : public ZSoundBufferWintel
    {
    public:
        explicit _ZSoundBufferDS(ZSynth* _synth);
        ~_ZSoundBufferDS() override;

        bool NeedData(int* _needed) override;
        bool Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags) override;
        void Start() override;
        void Start(bool _loop) override;
        void Stop() override;
        _ZSoundBuffer* Duplicate() override;
        int GetPlayCursor() override;
        void Free() override;
        void Resume() override;
        void Pause() override;
        void ResetVolume() override;
        void Update(SStartSound2D* _command) override;
        void Update(SStartSound3D* _command) override;
        void Update(SStartSoundBFormat* _command) override;
        void Update(SStartSound* _command) override;
        void UpdateFilters() override;
        void UnlockBuffer(void* _buffer1, int _size1, void* _buffer2, int _size2) override;
        bool LockBuffer(int _offset, int _size, void** _buffer1, int* _size1,
            void** _buffer2, int* _size2) override;

        RE_ADD_PADDING(8);
        IDirectSoundBuffer* m_pDSBuffer;
        IDirectSound3DBuffer* m_pDS3DBuffer;
        ZEaxSource* m_pEaxSource;
    };

    RE_VERIFY_OFFSET(_ZSoundBufferDS, m_pDSBuffer, 0x63FC);
    RE_VERIFY_OFFSET(_ZSoundBufferDS, m_pDS3DBuffer, 0x6400);
    RE_VERIFY_OFFSET(_ZSoundBufferDS, m_pEaxSource, 0x6404);
    RE_VERIFY_SIZE(_ZSoundBufferDS, 0x6408);
}
