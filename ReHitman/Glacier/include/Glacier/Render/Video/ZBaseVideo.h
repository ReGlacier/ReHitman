#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <cstdint>


namespace Glacier
{
    struct SSubtitle
    {
        TIMETYPE tStart;
        TIMETYPE tEnd;
        char szName[32];
    };
    RE_VERIFY_SIZE(SSubtitle, 0x28);

    class ZBaseVideo
    {
    public:
        // types
        enum EVMode
        {
            eVMAuto = 0,
            eNTSC = 1,
            ePAL = 2,
        };

        // vtbl
        virtual ~ZBaseVideo();
        virtual bool EndOfFile();
        virtual void SetFileName(const char* pszFileName) = 0;
        virtual void SetSubtitleFolder(const char* pszSubtitleFolder);
        virtual void Init();
        virtual void OnBeginPlay();
        virtual void OnEndPlay();
        virtual void ShowNextFrame() = 0;
        virtual void SetSize(uint32_t lWidth, uint32_t lHeight);
        virtual bool IsLastFrame() = 0;
        virtual void UpdateSubtitles(const TIMETYPE& tTime);

        // methods
        ZBaseVideo();

        void BeginPlaying();

        // members
        int m_iNumSubtitles;
        int m_iCurrentSubtitle;
        SSubtitle *m_pSubtitles;
        REFTAB m_rtDisabledCameras;
        MYSTR m_sSubtitleFolder;
        char m_szFileName[256];
        ZBaseVideo::EVMode m_eVMode;
        bool m_bInitialized;
        bool m_bWriteDemoOnScreen;
        bool m_bEOF;
        int m_lRemMaxFrameInterval;
        int m_lVideoSize[2];
        int m_lResolution[2];
    };

    RE_VERIFY_OFFSET(ZBaseVideo, m_pSubtitles, 0xC);
    RE_VERIFY_OFFSET(ZBaseVideo, m_rtDisabledCameras, 0x10); // Verified PC ctor
    RE_VERIFY_OFFSET(ZBaseVideo, m_sSubtitleFolder, 0x2C); // Verified PC ctor
}