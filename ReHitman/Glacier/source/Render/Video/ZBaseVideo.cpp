#include <Glacier/Render/Video/ZBaseVideo.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/System/CConfiguration.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>
#include <cmath>


namespace Glacier
{
    ZBaseVideo::ZBaseVideo()
        : m_rtDisabledCameras(16, 0)
    {
        m_iNumSubtitles = 0;
        m_iCurrentSubtitle = -1;
        m_pSubtitles = nullptr;
        m_sSubtitleFolder = MYSTR("");
        m_bInitialized = false;
        m_bWriteDemoOnScreen = false;
        m_bEOF = false;
        m_lRemMaxFrameInterval = 0;
        m_szFileName[0] = '\0';
    }

    ZBaseVideo::~ZBaseVideo()
    {
        if (m_lRemMaxFrameInterval)
            g_pSysInterface->WindowFirst->SetMaxFrameInterval(m_lRemMaxFrameInterval);

        if (m_pSubtitles)
        {
            ZUniMemory::Delete(m_pSubtitles);
            m_pSubtitles = nullptr;
        }

        g_pSysInterface->WindowFirst->m_bMovieRunning = false;

        if (g_pSysInterface->m_pSoundDll)
        {
            // TODO: Finish this place after ZSoundDll will be reversed.
            // PC (0x59ACD0): m_pSoundDll->vtbl[5](0, 1); (StopFMV)
        }

        g_ttLastVideoEndTime = g_pSysInterface->m_fActualTime;
    }

    bool ZBaseVideo::EndOfFile()
    {
        return m_bEOF;
    }

    void ZBaseVideo::SetFileName(const char* pszFileName)
    {
        // PC: strncpy/memset of the 256-byte filename buffer; the base does not validate it.
        (void)pszFileName;
    }

    // PC 0x59AEE0. Loads the subtitle list for the given locale resource, sizes each camera-based
    // FMV subtitle window and prepares the FMV-text overlay character.
    void ZBaseVideo::SetSubtitleFolder(const char* pszSubtitleFolder)
    {
        m_sSubtitleFolder = MYSTR(pszSubtitleFolder);

        m_iCurrentSubtitle = -1;
        if (m_pSubtitles)
        {
            ZUniMemory::Delete(m_pSubtitles);
            m_pSubtitles = nullptr;
        }
        m_iNumSubtitles = 0;

        const int lCount = g_pSysInterface->m_pEngineData->m_pLocaleResources->GetNumElements(pszSubtitleFolder);
        if (lCount > 0)
        {
            m_pSubtitles = ZUniMemory::NewArray<SSubtitle>(static_cast<uint32_t>(lCount));

            int lAdded = 0;
            for (int i = 0; i < lCount; ++i)
            {
                Resource res;
                g_pSysInterface->m_pEngineData->m_pLocaleResources->GetElementResource(pszSubtitleFolder, i, &res);
                const char* pszName = res.GetKey();

                // Parse the "start-end" time range encoded in the element name.
                const char* pszEnd = strchr(pszName, '-');
                if (pszEnd)
                {
                    const float fStart = static_cast<float>(atof(pszName));
                    const float fEnd = static_cast<float>(atof(pszEnd + 1));
                    if (fStart <= fEnd)
                    {
                        m_pSubtitles[lAdded].tStart.secs = static_cast<int>(fStart * 34.133335);
                        m_pSubtitles[lAdded].tEnd.secs = static_cast<int>(fEnd * 34.133335);
                        strncpy(m_pSubtitles[lAdded].szName, pszName, sizeof(m_pSubtitles[lAdded].szName) - 1);
                        m_pSubtitles[lAdded].szName[sizeof(m_pSubtitles[lAdded].szName) - 1] = '\0';
                        ++lAdded;
                    }
                }
            }
            m_iNumSubtitles = lAdded;
        }

        // TODO: Finish this place after the FMVSubtitle camera/font setup will be reversed.
        // PC (0x59AEE0): looks up the "FMVSubtitles" scene object (a ZCHAROBJ), enables the
        //     "SubtitleWindow" camera/font and positions the subtitle area.
    }

    // PC 0x59AE20. Raises the render's max-frame-interval while the video plays and enables the
    // movie/subtitles state.
    void ZBaseVideo::Init()
    {
        m_lRemMaxFrameInterval = g_pSysInterface->WindowFirst->SetMaxFrameInterval(2);

        if (g_pSysInterface->m_pSoundDll)
        {
            // TODO: Finish this place after ZSoundDll will be reversed.
            // PC (0x59AE20): m_pSoundDll->vtbl[5](1, 1); (PlayFMV)
        }

        g_pSysInterface->WindowFirst->m_bMovieRunning = true;
    }

    // PC 0x59B670. On begin play: disables every enabled camera in the currently active view and
    // adds it to the disabled-camera list, then starts the subtitle window.
    void ZBaseVideo::OnBeginPlay()
    {
        ZGEOM* pSubtitleCamera = FindSubtitleCamera();
        g_pSysInterface->WindowFirst->m_lInCutSequence++;

        if (pSubtitleCamera)
        {
            ZRender* pRender = g_pSysInterface->WindowFirst;
            for (uint32_t i = 0; i < pRender->GetCamera(-1 ? pRender->GetCameraList()->GetRefNr(0) : 0); ++i)
            {
                // TODO: Finish this place after the view/camera enumeration will be reversed.
                // PC (0x59B670): iterates all cameras of the first view; cameras with the "enabled"
                //     flag (byte +44 bit 0x20) are hidden and added to m_rtDisabledCameras.
            }

            // TODO: Finish this place after the subtitle-window activation will be reversed.
            // PC (0x59B670): looks up "SubtitleWindow", enables it and sets the subtitle folder.
        }
    }

    // PC 0x59B7C0. On end play: decrements the in-cut-sequence counter, re-enables the disabled
    // cameras and clears the subtitle / text-overlay objects.
    void ZBaseVideo::OnEndPlay()
    {
        ZRender* pRender = g_pSysInterface->WindowFirst;
        ZASSERT(pRender->m_lInCutSequence != 0);
        pRender->m_lInCutSequence--;

        ZGEOM* pSubtitleCamera = FindSubtitleCamera();
        if (pSubtitleCamera)
        {
            // TODO: Finish this place after the camera re-enable will be reversed.
        }

        // TODO: Finish this place after the FMVSubtitles / FMVTextOverlay text clearing will be reversed.

        // Re-enable all previously disabled cameras.
        RefRun run;
        m_rtDisabledCameras.RunInitNxtRef(&run);
        for (uint32_t rRef = m_rtDisabledCameras.RunNxtRef(&run); run; rRef = m_rtDisabledCameras.RunNxtRef(&run))
        {
            ZGEOM* pCamera = ZGEOM::RefToPtr(rRef);
            if (pCamera && (ZCAMERA::m_Mask & pCamera->GetObjectId()) == ZCAMERA::m_Id)
            {
                // TODO: Finish this place after ZCAMERA::Init will be reversed.
                // PC (0x59B7C0): camera->Init(); (re-activate the camera)
            }
        }
        m_rtDisabledCameras.Clear();
    }

    // PC 0x59AEAE (Release; base pure-virtual EndOfFile etc. left abstract in the concrete classes).
    void ZBaseVideo::SetSize(uint32_t lWidth, uint32_t lHeight)
    {
        (void)lWidth;
        (void)lHeight;
        // Do nothing (base)
    }

    // PC 0x59B5C0. Finds the "SubtitleWindow" camera: resolves the scene object named
    // "SubtitleWindow" (a ZGROUP) and returns its first ZCAMERA child.
    ZGEOM* ZBaseVideo::FindSubtitleCamera()
    {
        // TODO: Finish this place after the ZGEOM group-child traversal will be reversed.
        // PC (0x59B5C0): GetVal("SubtitleWindow", &id) -> ZGEOM::RefToPtr(id), asserted to be a
        //     ZGROUP; walks its children and returns the first one whose object-id matches ZCAMERA.
        return nullptr;
    }

    // PC 0x59B270. Drives the FMV subtitle / text-overlay widgets for the current playback time.
    bool ZBaseVideo::UpdateSubtitles(const TIMETYPE& tTime)
    {
        // TODO: Finish this place after the FMV overlay text widgets will be reversed.
        // PC (0x59B270): switches the active subtitle (m_iCurrentSubtitle) when its start time
        //     passes, fades the "FMVTextOverlay" text-in widget based on the elapsed seconds and
        //     iterates its CHAROBJ children to set/clear the date-on-avi overlay text.

        if (m_pSubtitles && CConfiguration::m_bSubtitles)
        {
            if (m_iCurrentSubtitle >= 0 && m_pSubtitles[m_iCurrentSubtitle].tEnd < tTime)
                m_iCurrentSubtitle = -1;

            const int lNext = m_iCurrentSubtitle + 1;
            if (lNext < m_iNumSubtitles && m_pSubtitles[lNext].tStart < tTime)
                m_iCurrentSubtitle = lNext;
        }

        return true;
    }
}
