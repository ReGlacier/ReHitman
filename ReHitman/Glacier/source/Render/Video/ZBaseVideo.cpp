#include <Glacier/Render/Video/ZBaseVideo.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZBaseVideo::ZBaseVideo()
        : m_rtDisabledCameras(16, 0)
    {
        m_iNumSubtitles = 0;
        m_iCurrentSubtitle = -1;
        m_pSubtitles = nullptr;
        m_bInitialized = false;
        m_bEOF = false;
        m_lRemMaxFrameInterval = 0;
        m_szFileName[0] = '\0';
    }

    ZBaseVideo::~ZBaseVideo()
    {
        if (m_lRemMaxFrameInterval)
        {
            // TODO: Finish when ZRender will be reversed
        }

        if (m_pSubtitles)
        {
            ZUniMemory::Delete(m_pSubtitles);
            m_pSubtitles = nullptr;
        }

        // TODO: Finish when ZRender will be reversed

        if (g_pSysInterface->m_pSoundDll)
        {
            // TODO: Finish when ZSoundDll will be reversed
        }

        g_ttLastVideoEndTime = g_pSysInterface->m_fActualTime;
    }

    bool ZBaseVideo::EndOfFile()
    {
        return m_bEOF;
    }

    void ZBaseVideo::SetSubtitleFolder(const char* pszSubtitleFolder)
    {
        // TODO: Finish me
    }

    void ZBaseVideo::Init()
    {
        // TODO: Finish after ZRender reversed
        if (g_pSysInterface->m_pSoundDll)
        {
            // TODO: Finish after ZSoundDll reversed
        }
        // TODO: Finish after ZRender reversed
    }

    void ZBaseVideo::OnBeginPlay()
    {
        // TODO: Finish me
    }

    void ZBaseVideo::OnEndPlay()
    {
        // TODO: Finish me
    }

    void ZBaseVideo::SetSize(uint32_t lWidth, uint32_t lHeight)
    {
        // Do nothing
    }

    void ZBaseVideo::UpdateSubtitles(const TIMETYPE& tTime)
    {
        // TODO: Finish me
    }
}