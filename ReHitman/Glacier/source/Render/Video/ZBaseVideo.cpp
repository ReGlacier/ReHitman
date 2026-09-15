#include <Glacier/Render/Video/ZBaseVideo.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/System/CConfiguration.h>
#include <Glacier/System/ZDllBase.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/View/IView.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>
#include <cmath>


namespace Glacier
{
    namespace
    {
        // The Blood Money sound DLL exposes PlayFMV/StopFMV at this legacy slot,
        // but the public sound base predates those two methods. Keep the ABI
        // dispatch local rather than changing the sound class layout.
        void SetFmvPlayback(ZDllBase* pSound, int lPlay)
        {
            using FmvPlayback = void (__thiscall*)(ZDllBase*, int, int);
            void** pVtable = *reinterpret_cast<void***>(pSound);
            reinterpret_cast<FmvPlayback>(pVtable[5])(pSound, lPlay, 1);
        }
    }

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
            SetFmvPlayback(g_pSysInterface->m_pSoundDll, 0);

        g_ttLastVideoEndTime = g_pSysInterface->m_fActualTime;
    }

    bool ZBaseVideo::EndOfFile()
    {
        return m_bEOF;
    }

    void ZBaseVideo::SetFileName(const char* pszFileName)
    {
        memset(m_szFileName, 0, sizeof(m_szFileName));
        if (pszFileName)
            strncpy(m_szFileName, pszFileName, sizeof(m_szFileName) - 1);
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

        int rSubtitles = 0;
        if (g_pSysInterface->m_pEngineData->GetSceneCom()->GetVal("FMVSubtitles", &rSubtitles))
        {
            ZGEOM* pSubtitles = ZGEOM::RefToPtr(rSubtitles);
            if (pSubtitles && pSubtitles->IsDerivedFrom<ZCHAROBJ>())
            {
                auto* pText = static_cast<ZCHAROBJ*>(pSubtitles);
                pText->SetAlpha(36);
                pText->SetPos(ZVector3(static_cast<float>(g_pSysInterface->m_lResolution[0]) * 0.5f,
                    static_cast<float>(g_pSysInterface->m_lResolution[1] - 40), 0.0f));
                if (pText->IsDerivedFrom<ZLINEOBJ>())
                    static_cast<ZLINEOBJ*>(pText)->SetWidth(g_pSysInterface->m_lResolution[0] - 96);
            }
        }
    }

    // PC 0x59AE20. Raises the render's max-frame-interval while the video plays and enables the
    // movie/subtitles state.
    void ZBaseVideo::Init()
    {
        m_lRemMaxFrameInterval = g_pSysInterface->WindowFirst->SetMaxFrameInterval(2);

        if (g_pSysInterface->m_pSoundDll)
            SetFmvPlayback(g_pSysInterface->m_pSoundDll, 1);

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
            IView* pView = pRender->FindView(0);
            for (int i = 0; pView; ++i)
            {
                ZCAMERA* pCamera = pView->GetCamera(i);
                if (!pCamera)
                    break;
                if (pCamera->IsActive())
                {
                    pCamera->DeactivateCam();
                    m_rtDisabledCameras.Add(pCamera->GetRef());
                }
            }

            static_cast<ZCAMERA*>(pSubtitleCamera)->ActivateCam();
            pRender->AddCamera(static_cast<ZCAMERA*>(pSubtitleCamera), 0, 0.0f);
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
            pRender->RemoveCamera(static_cast<ZCAMERA*>(pSubtitleCamera), 0);

        int rObject = 0;
        if (g_pSysInterface->m_pEngineData->GetSceneCom()->GetVal("FMVSubtitles", &rObject))
        {
            ZGEOM* pSubtitles = ZGEOM::RefToPtr(rObject);
            if (pSubtitles && pSubtitles->IsDerivedFrom<ZCHAROBJ>())
                static_cast<ZCHAROBJ*>(pSubtitles)->Clear();
        }
        if (g_pSysInterface->m_pEngineData->GetSceneCom()->GetVal("FMVTextOverlay", &rObject))
        {
            ZGEOM* pOverlay = ZGEOM::RefToPtr(rObject);
            if (pOverlay && pOverlay->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroup = static_cast<ZGROUP*>(pOverlay);
                for (ZBaseGeom* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                    if (pChild->GetGeom() && pChild->GetGeom()->IsDerivedFrom<ZCHAROBJ>())
                        static_cast<ZCHAROBJ*>(pChild->GetGeom())->Clear();
            }
        }

        // Re-enable all previously disabled cameras.
        RefRun run;
        m_rtDisabledCameras.RunInitNxtRef(&run);
        for (uint32_t rRef = m_rtDisabledCameras.RunNxtRef(&run); run; rRef = m_rtDisabledCameras.RunNxtRef(&run))
        {
            ZGEOM* pCamera = ZGEOM::RefToPtr(rRef);
            if (pCamera && (ZCAMERA::m_Mask & pCamera->GetObjectId()) == ZCAMERA::m_Id)
                static_cast<ZCAMERA*>(pCamera)->ActivateCam();
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
        int rWindow = 0;
        if (!g_pSysInterface->m_pEngineData->GetSceneCom()->GetVal("SubtitleWindow", &rWindow))
            return nullptr;

        ZGEOM* pWindow = ZGEOM::RefToPtr(rWindow);
        if (!pWindow || !pWindow->IsDerivedFrom<ZGROUP>())
            return nullptr;

        auto* pGroup = static_cast<ZGROUP*>(pWindow);
        for (ZBaseGeom* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
        {
            ZGEOM* pGeom = pChild->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZCAMERA>())
                return pGeom;
        }
        return nullptr;
    }

    // PC 0x59B270. Drives the FMV subtitle / text-overlay widgets for the current playback time.
    bool ZBaseVideo::UpdateSubtitles(const TIMETYPE& tTime)
    {
        int rObject = 0;
        CCom* pSceneCom = g_pSysInterface->m_pEngineData->GetSceneCom();
        if (pSceneCom->GetVal("FMVSubtitles", &rObject))
        {
            ZGEOM* pSubtitles = ZGEOM::RefToPtr(rObject);
            if (pSubtitles && pSubtitles->IsDerivedFrom<ZCHAROBJ>())
            {
                auto* pText = static_cast<ZCHAROBJ*>(pSubtitles);
                if (m_pSubtitles && CConfiguration::m_bSubtitles)
                {
                    if (m_iCurrentSubtitle >= 0 && m_pSubtitles[m_iCurrentSubtitle].tEnd < tTime)
                        m_iCurrentSubtitle = -1;

                    const int lNext = m_iCurrentSubtitle + 1;
                    if (lNext < m_iNumSubtitles && m_pSubtitles[lNext].tStart < tTime)
                    {
                        m_iCurrentSubtitle = lNext;
                        pText->SetTextId(static_cast<const char*>(m_sSubtitleFolder),
                            m_pSubtitles[lNext].szName);
                    }
                }
                else
                {
                    pText->Clear();
                }
            }
        }

        if (pSceneCom->GetVal("FMVTextOverlay", &rObject))
        {
            ZGEOM* pOverlay = ZGEOM::RefToPtr(rObject);
            char szResource[256];
            const char* pSlash = strrchr(m_szFileName, '/');
            const char* pBackslash = strrchr(m_szFileName, '\\');
            if (pBackslash > pSlash)
                pSlash = pBackslash;
            snprintf(szResource, sizeof(szResource), "/FMVSubtitles/DateOnAvi/%s",
                pSlash ? pSlash + 1 : m_szFileName);
            if (char* pExtension = strchr(szResource, '.'))
                *pExtension = '\0';

            if (pOverlay
                && g_pSysInterface->m_pEngineData->m_pLocaleResources->HasResource(szResource)
                && pOverlay->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroup = static_cast<ZGROUP*>(pOverlay);
                float fAlpha = tTime.secs < 1024 ? tTime.secs / 1024.0f
                    : (tTime.secs <= 5120 ? 1.0f : 1.0f - (tTime.secs - 5120) / 1024.0f);
                if (fAlpha < 0.0f)
                    fAlpha = 0.0f;
                if (pOverlay->IsDerivedFrom<ZWINGROUP>())
                    static_cast<ZWINGROUP*>(pOverlay)->SetPos(
                        static_cast<float>(g_pSysInterface->m_lResolution[0]) * 0.5f,
                        static_cast<float>(g_pSysInterface->m_lResolution[1]) * 0.6f, 0.0f);
                bool bFirst = true;
                for (ZBaseGeom* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                {
                    ZGEOM* pGeom = pChild->GetGeom();
                    if (pGeom && pGeom->IsDerivedFrom<ZCHAROBJ>())
                    {
                        auto* pText = static_cast<ZCHAROBJ*>(pGeom);
                        pText->SetTextId("", szResource);
                        pText->SetAlpha(static_cast<uint8_t>(fAlpha * 255.0f));
                        if (bFirst)
                        {
                            fAlpha *= 0.3f;
                            bFirst = false;
                        }
                    }
                }
            }
        }

        return true;
    }
}
