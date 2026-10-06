#include <Glacier/Render/Video/ZWintelVideoBink.h>
#include <Glacier/Render/ZRenderWintelD3D.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>

namespace Glacier
{
    namespace
    {
        void NormalizeBinkName(char* pszName, size_t lCapacity)
        {
            for (size_t i = 0; i < lCapacity && pszName[i]; ++i)
                if (pszName[i] == '\\')
                    pszName[i] = '/';
        }
    }

    ZWintelVideoBink::ZWintelVideoBink()
        : ZBaseVideo()
        , m_pBink(nullptr)
        , m_lLastFrame(0)
    {
    }

    ZWintelVideoBink::~ZWintelVideoBink()
    {
        OnEndPlay();
        if (m_pBink)
            BinkVideoInterface::Interface::Instance().Close(m_pBink);
        m_pBink = nullptr;
    }

    ZWintelVideoBink* ZWintelVideoBink::BuildInstance()
    {
        return ZUniMemory::New<ZWintelVideoBink>();
    }

    bool ZWintelVideoBink::EndOfFile()
    {
        return m_bEOF || IsLastFrame();
    }

    void ZWintelVideoBink::SetFileName(const char* pszFileName)
    {
        memset(m_szFileName, 0, sizeof(m_szFileName));
        if (!pszFileName)
            return;
        strncpy(m_szFileName, pszFileName, sizeof(m_szFileName) - 1);
        NormalizeBinkName(m_szFileName, sizeof(m_szFileName));
        if (!strchr(m_szFileName, '.'))
            strncat(m_szFileName, ".bik", sizeof(m_szFileName) - strlen(m_szFileName) - 1);
    }

    void ZWintelVideoBink::Init()
    {
        auto& bink = BinkVideoInterface::Interface::Instance();
        if (!bink.IsAvailable())
        {
            std::fprintf(stderr, "ZWintelVideoBink: binkw32.dll unavailable, skipping video '%s'\n", m_szFileName);
            m_bEOF = true;
            return;
        }

        m_pBink = bink.Open(m_szFileName, 0);
        BinkVideoInterface::VideoInfo info {};
        if (!m_pBink || !bink.GetInfo(m_pBink, info))
        {
            std::fprintf(stderr, "ZWintelVideoBink: failed to open '%s'\n", m_szFileName);
            m_bEOF = true;
            return;
        }

        m_lVideoSize[0] = static_cast<int>(info.lWidth);
        m_lVideoSize[1] = static_cast<int>(info.lHeight);
        SetSize(info.lWidth, info.lHeight);
        if (auto* pRender = dynamic_cast<ZRenderWintelD3D*>(g_pSysInterface->WindowFirst))
            pRender->CreateOffscreenSurface(static_cast<int>(info.lWidth), static_cast<int>(info.lHeight));
        ZBaseVideo::Init();
        m_bInitialized = true;
    }

    void ZWintelVideoBink::OnBeginPlay()
    {
        ZBaseVideo::OnBeginPlay();
        if (m_bInitialized)
            m_lLastFrame = 0;
    }

    void ZWintelVideoBink::OnEndPlay()
    {
        if (m_bInitialized)
            ZBaseVideo::OnEndPlay();
        m_bInitialized = false;
    }

    void ZWintelVideoBink::ShowNextFrame()
    {
        if (!m_bInitialized || !m_pBink)
            return;
        auto& bink = BinkVideoInterface::Interface::Instance();
        if (bink.Wait(m_pBink))
            return;
        bink.DoFrame(m_pBink);
        if (auto* pRender = dynamic_cast<ZRenderWintelD3D*>(g_pSysInterface->WindowFirst))
        {
            unsigned int lWidth = 0;
            int lHeight = 0;
            int lPitch = 0;
            void* pSurface = pRender->LockOffscreenSurface(&lWidth, &lHeight, &lPitch, nullptr);
            if (pSurface)
                bink.CopyToBuffer(m_pBink, pSurface, lPitch, lHeight, 0, 0, 0x80000005u);
            pRender->UnlockOffscreenSurface(pSurface);
            pRender->BlitOffscreenSurface();
        }
        bink.NextFrame(m_pBink);
        ++m_lLastFrame;
        UpdateSubtitles(TIMETYPE(static_cast<float>(m_lLastFrame) / 30.0f));
    }

    void ZWintelVideoBink::SetSize(uint32_t lWidth, uint32_t lHeight)
    {
        ZBaseVideo::SetSize(lWidth, lHeight);
        m_lVideoSize[0] = static_cast<int>(lWidth);
        m_lVideoSize[1] = static_cast<int>(lHeight);
    }

    bool ZWintelVideoBink::IsLastFrame()
    {
        if (!m_pBink)
            return true;
        const auto& bink = BinkVideoInterface::Interface::Instance();
        BinkVideoInterface::VideoInfo info {};
        return !bink.GetInfo(m_pBink, info) || info.lFrameNumber >= info.lFrameCount;
    }
}
