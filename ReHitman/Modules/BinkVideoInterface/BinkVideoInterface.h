#pragma once

#include <cstdint>

namespace BinkVideoInterface
{
    struct BinkHandle;
    using HBINK = BinkHandle*;

    struct VideoInfo
    {
        uint32_t lWidth;
        uint32_t lHeight;
        uint32_t lFrameNumber;
        uint32_t lFrameCount;
    };

    class Interface final
    {
    public:
        static Interface& Instance();

        bool IsAvailable() const;
        HBINK Open(const char* pszFileName, uint32_t lFlags);
        bool GetInfo(HBINK pVideo, VideoInfo& rInfo) const;
        void Close(HBINK pVideo);
        bool Wait(HBINK pVideo);
        void DoFrame(HBINK pVideo);
        void CopyToBuffer(HBINK pVideo, void* pBuffer, int lPitch, int lHeight,
            int lDestX, int lDestY, uint32_t lFlags);
        void NextFrame(HBINK pVideo);
        void SetSoundSystem(void* pOpenDirectSound, void* pSoundSystem);
        void* OpenDirectSound(uint32_t lRate, uint32_t lBits, uint32_t lChannels);
        void SetVolume(HBINK pVideo, uint32_t lTrack, uint32_t lVolume);

    private:
        Interface();
        ~Interface();
        Interface(const Interface&) = delete;
        Interface& operator=(const Interface&) = delete;

        void* m_hModule;
        HBINK (*m_pOpen)(const char*, uint32_t);
        void (*m_pClose)(HBINK);
        int (*m_pWait)(HBINK);
        void (*m_pDoFrame)(HBINK);
        void (*m_pCopyToBuffer)(HBINK, void*, int, int, int, int, uint32_t);
        void (*m_pNextFrame)(HBINK);
        void (*m_pSetSoundSystem)(void*, void*);
        void* (*m_pOpenDirectSound)(uint32_t, uint32_t, uint32_t);
        void (*m_pSetVolume)(HBINK, uint32_t, uint32_t);
    };
}
