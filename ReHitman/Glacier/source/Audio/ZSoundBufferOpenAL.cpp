#include <Glacier/Audio/ZSoundBufferOpenAL.h>

namespace Glacier
{
    ZSoundBufferOpenAL::ZSoundBufferOpenAL(ZSynth* _synth)
        : ZSoundBufferWintel(_synth)
        , m_lALSource(0)
        , m_lALBuffers{0, 0}
    {
    }

    bool ZSoundBufferOpenAL::NeedData(int* _needed)
    {
        *_needed = 0;
        return GetNumStreamSamples() != 0;
    }

    bool ZSoundBufferOpenAL::Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags)
    {
        if (!ZSoundBufferWintel::Create(_wave, _bufferType, _flags))
            return false;
        // TODO: Finish this place after ZSynthOpenAL will be reversed
        return true;
    }

    void ZSoundBufferOpenAL::Start() {}
    void ZSoundBufferOpenAL::Stop() { ZSoundBufferWintel::Stop(); }
    _ZSoundBuffer* ZSoundBufferOpenAL::Duplicate() { return nullptr; }
    int ZSoundBufferOpenAL::GetPlayCursor() { return 0; }

    void ZSoundBufferOpenAL::Free()
    {
        // TODO: Finish this place after ZSynthOpenAL will be reversed
        m_lALSource = 0;
        m_lALBuffers[0] = 0;
        m_lALBuffers[1] = 0;
        _ZSoundBuffer::Free();
    }

    void ZSoundBufferOpenAL::Resume() {}
    void ZSoundBufferOpenAL::Pause() {}
    int ZSoundBufferOpenAL::GetNumStreamSamples() { return 0; }
    void ZSoundBufferOpenAL::Update(SStartSound2D*) {}
    void ZSoundBufferOpenAL::Update(SStartSound3D*) {}
    void ZSoundBufferOpenAL::Update(SStartSoundBFormat*) {}
    void ZSoundBufferOpenAL::Update(SStartSound*) {}
    void ZSoundBufferOpenAL::UpdateFilters() {}
    void ZSoundBufferOpenAL::UnlockBuffer(void*, int, void*, int) {}

    bool ZSoundBufferOpenAL::LockBuffer(int, int, void**, int*, void**, int*)
    {
        // TODO: Finish this place after ZSynthOpenAL will be reversed
        return false;
    }
}
