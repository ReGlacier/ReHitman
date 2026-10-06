#include <Glacier/Audio/ZSoundBufferDP.h>

namespace Glacier
{
    ZSoundBufferDP::ZSoundBufferDP(ZSynth* _synth)
        : ZSoundBufferWintel(_synth)
        , m_pDPVoice(nullptr)
    {
    }

    ZSoundBufferDP::~ZSoundBufferDP() = default;

    bool ZSoundBufferDP::NeedData(int* _needed)
    {
        const int halfSize = static_cast<int>(m_lBufferSize / 2);
        if (!halfSize)
            return false;

        const int segment = GetPlayCursor() / halfSize;
        if (segment == m_lCurrentStreamSegment)
            return false;

        *_needed = halfSize * m_lCurrentStreamSegment;
        m_lCurrentStreamSegment = segment;
        return true;
    }

    bool ZSoundBufferDP::Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags)
    {
        m_pDPVoice = nullptr;
        m_lCurrentStreamSegment = 0;
        if (!ZSoundBufferWintel::Create(_wave, _bufferType, _flags))
            return false;
        // TODO: Finish this place after ZSynthDP will be reversed
        return true;
    }

    void ZSoundBufferDP::Start() {}
    void ZSoundBufferDP::Stop() { ZSoundBufferWintel::Stop(); }
    _ZSoundBuffer* ZSoundBufferDP::Duplicate() { return nullptr; }
    int ZSoundBufferDP::GetPlayCursor() { return 0; }

    void ZSoundBufferDP::Free()
    {
        // TODO: Finish this place after ZSynthDP will be reversed
        m_pDPVoice = nullptr;
        _ZSoundBuffer::Free();
    }

    void ZSoundBufferDP::Resume() {}
    void ZSoundBufferDP::Pause() {}
    void ZSoundBufferDP::Update(SStartSound2D*) {}
    void ZSoundBufferDP::Update(SStartSound3D*) {}
    void ZSoundBufferDP::Update(SStartSoundBFormat*) {}
    void ZSoundBufferDP::Update(SStartSound*) {}
    void ZSoundBufferDP::UpdateFilters() {}
    void ZSoundBufferDP::UnlockBuffer(void*, int, void*, int) {}

    bool ZSoundBufferDP::LockBuffer(int, int, void**, int*, void**, int*)
    {
        // TODO: Finish this place after ZSynthDP will be reversed
        return false;
    }
}
