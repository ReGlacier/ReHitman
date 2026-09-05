#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Draw/ZRenderDrawBase.h>
#include <Glacier/Render/View/IView.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysMem.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Animation/Manager.h>

#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        void SetStaticShadowSampleHeightLimit(float fValue)
        {
            g_fStaticShadowSampleHeightLimit = fValue;
        }
    }

    // Static LOD range table, stored right after the ZRender vtable in the PC image (0x00761688).
    static constexpr float fLODRangeTables[][8] =
    {
        { 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 128.0f },
        { 3.0f, 6.0f, 12.0f, 24.0f, 48.0f, 96.0f, 192.0f, 384.0f },
        { 6.0f, 12.0f, 24.0f, 48.0f, 96.0f, 192.0f, 384.0f, 384.0f },
        { 100000.0f, 100000.0f, 100000.0f, 100000.0f, 100000.0f, 100000.0f, 100000.0f, 100000.0f },
        { 3.0f, 8.0f, 12.0f, 24.0f, 48.0f, 96.0f, 192.0f, 384.0f },
    };

    // PC 0x00469780. Base ZRender construction: seeds the camera list and hands the instance state
    // over to Initialize (which links this render into the g_pSysInterface->WindowFirst chain).
    ZRender::ZRender(int hInstance, void* hParentWnd)
        : m_Camera(8, 0)
    {
        m_lInCutSequence = 0;
        m_bMovieRunning = 0;
        m_Size[0] = -1;
        m_Size[1] = -1;
        m_pDrawDebugRender = 0;
        Initialize(hInstance, hParentWnd);
    }

    // PC 0x004684C0. Resets the per-frame state, links this render into the window list and fills
    // the LOD lookup tables (background 7 levels, characters 6 levels).
    void ZRender::Initialize(int hInstance, void* hParentWnd)
    {
        (void)hInstance;
        (void)hParentWnd;

        m_bInitialized = false;
        m_MouseX = 0.0f;
        m_MouseY = 0.0f;
        m_MouseZ = 0.0f;
        m_ScrMouseX = 0;
        m_ScrMouseY = 0;
        m_bLeftMouseButtonDown = false;
        m_bMiddleMouseButtonDown = false;
        m_bRightMouseButtonDown = false;
        m_FPS = 60;
        m_bConsoleKey = false;
        m_lLockCount = 0;
        m_lInCutSequence = 0;
        m_fShowEditCursor = 0.0f;
        m_bEnable4_3CutOff = false;
        m_fFontSize[0] = 1.0f;
        m_fFontSize[1] = 1.0f;
        m_NxtLine = 10;
        m_RemNxtLine = 10;
        m_lMaxFrameInterval = 1;

        std::memset(&m_StatCounters, 0, sizeof(m_StatCounters));

        Bef = nullptr;
        Nxt = g_pSysInterface->WindowFirst;
        if (Nxt)
            Nxt->Bef = this;
        g_pSysInterface->WindowFirst = this;
        SubChain = nullptr;

        int lLevelOfDetails = 1;
        char* pszOption = nullptr;
        if (ZSysInterface::GetOption("LevelOfDetail", &pszOption))
        {
            const int lVal = atoi(pszOption);
            lLevelOfDetails = (lVal > 2 || !lVal) ? 1 : lVal;
        }
        SetLOD(m_aLODLookupTable, lLevelOfDetails, 7);

        ZSysInterface::GetOption("DisableCharacterLOD", nullptr);
        SetLOD(m_LODTableCharacters, 2, 6);

        m_pMemColorsDisplay = nullptr;
        m_bForceLightUpdate = false;
        m_bUseSWLight = false;
        m_pRenderDraw = nullptr;
        m_bWideScreen = false;
        m_bLetterBox = false;
        m_bSurroundGaming = false;
        m_fLightMultiplier = 1.0f;
    }

    // PC 0x004687C0 (vtable slot 0 = Release). Unlinks from the g_pSysInterface->WindowFirst
    // list, ends the draw base and lets m_Camera auto-destruct.
    ZRender::~ZRender()
    {
        if (Bef)
            Bef->Nxt = Nxt;
        else
            g_pSysInterface->WindowFirst = Nxt;

        if (Nxt)
            Nxt->Bef = Bef;

        if (m_pRenderDraw)
            m_pRenderDraw->End();
    }

    // PC 0x00468680
    void ZRender::Close()
    {
        m_pRenderDraw->RemoveViewsUsingRender(this);
        m_bInitialized = false;
    }

    // PC 0x00761534 slot (thunk 0x004F9CE0 forwarding to slot 1 = Close)
    void ZRender::CrashClose()
    {
        Close();
    }

    // PC 0x00468760
    void ZRender::SetWideScreen(bool bWideScreen)
    {
        m_bWideScreen = bWideScreen;
    }

    // PC 0x00468770
    void ZRender::SetLetterBox(bool bLetterBox)
    {
        m_bLetterBox = bLetterBox;
    }

    // PC 0x00468780
    void ZRender::SetSurroundGaming(bool bSurroundGaming)
    {
        m_bSurroundGaming = bSurroundGaming;
    }

    // PC 0x00468850
    bool ZRender::ShadowMode()
    {
        return m_bShadowMode;
    }

    // PC 0x00761548 (nullsub)
    void ZRender::SetCopyToFrontAlpha(unsigned int)
    {
        // Do nothing
    }

    // PC 0x00468860
    ZRenderDrawBase* ZRender::GetDrawBase()
    {
        return m_pRenderDraw;
    }

    // PC 0x00761550 (PC-only empty stub; ZRenderWintelD3D implements the real ColorFill)
    void ZRender::ColorFill()
    {
        // Do nothing
    }

    // PC 0x00468EC0
    bool ZRender::BeginScene()
    {
        ++g_pSysInterface->m_lFrameCount;
        if (m_pRenderDraw->ShouldFlush())
            m_pRenderDraw->Flush();

        return true;
    }

    // PC 0x00580400 (shared return-true stub)
    bool ZRender::EndScene()
    {
        return true;
    }

    // PC 0x00468FE0 (m_pRenderDraw->Invalidate)
    void ZRender::ForceAllLeave()
    {
        m_pRenderDraw->Invalidate();
    }

    // PC 0x004688A0. Builds a 256-entry LOD lookup table from the static fLODRangeTables rows:
    // table[i] = min(first level j where (i + 1) <= range[lLOD][j], lMaxLOD).
    void ZRender::SetLOD(unsigned char* pLODTable, unsigned int lLOD, unsigned int lMaxLOD)
    {
        for (uint32_t lIndex = 0; lIndex < 256; ++lIndex)
        {
            const float fDistance = static_cast<float>(lIndex + 1);

            uint32_t lLevel = 0;
            while (lLevel < 8 && fDistance > fLODRangeTables[lLOD][lLevel])
                ++lLevel;

            pLODTable[lIndex] = static_cast<uint8_t>(lLevel > lMaxLOD ? lMaxLOD : lLevel);
        }
    }

    // PC 0x00469830
    void ZRender::AllocateDrawBuffers()
    {
        const uint32_t lPrevColor = SetMemColor(0xFF60A0u);
        FreeDrawBuffers();

        if (!m_pRenderDraw->m_bInitialized)
        {
            m_pRenderDraw->Init();

            if (!FindView(0))
            {
                RefRun run;
                m_Camera.RunInitNxtRef(&run);
                for (uint32_t rRef = m_Camera.RunNxtRef(&run); run; rRef = m_Camera.RunNxtRef(&run))
                {
                    if (ZCAMERA* pCamera = static_cast<ZCAMERA*>(ZGEOM::RefToPtr(rRef)); pCamera)
                        AddCameraToView(pCamera, 0);
                }
            }
        }

        SetMemColor(lPrevColor);

        CCom* pSceneCom = g_pSysInterface->m_pEngineData->GetSceneCom();
        float fLimit = 0.0f;
        pSceneCom->GetVal("StaticShadowSampleHeightLimit", &fLimit);
        if (fLimit < 100.0f)
            fLimit = 10000.0f;
        SetStaticShadowSampleHeightLimit(fLimit);
    }

    // PC 0x004687A0 (m_pRenderDraw->End)
    void ZRender::FreeDrawBuffers()
    {
        if (m_pRenderDraw)
            m_pRenderDraw->End();
    }

    // PC 0x00468790 (m_pRenderDraw->Flush)
    void ZRender::InvalidateDraw()
    {
        m_pRenderDraw->Flush();
    }

    // PC 0x00761570 (nullsub)
    void ZRender::FadeScreen(float, unsigned int)
    {
        // Do nothing
    }

    // PC 0x004686A0
    uint32_t ZRender::LockDrawBaseGeomsBuffer(void** ppBuffer)
    {
        InvalidateDraw();
        m_pRenderDraw->Flush();

        m_lLockDrawBaseSize = m_pRenderDraw->GetLargestFreeBlock() - 51200;
        *ppBuffer = m_pRenderDraw->Alloc(m_lLockDrawBaseSize, nullptr, 0);
        m_pLockDrawBaseMemory = *ppBuffer;

        printf("ZRenderPS2::LockDrawBaseGeomsBuffer size %dKb\n", m_lLockDrawBaseSize / 1024); // PC debug leftover
        return m_lLockDrawBaseSize;
    }

    // PC 0x00468720
    void ZRender::UnlockDrawBaseGeomsBuffer()
    {
        ZASSERT(m_pLockDrawBaseMemory);

        m_pRenderDraw->Free(m_pLockDrawBaseMemory, m_lLockDrawBaseSize);
        m_pLockDrawBaseMemory = nullptr;
    }

    // PC 0x0076157C (_purecall) - pure virtual, no base implementation

    // PC 0x00761580 (empty stub)
    void ZRender::CopyFrontToBack()
    {
        // Do nothing
    }

    // PC 0x00761584 (nullsub)
    void ZRender::GetClippedPrimTriangles(REFTAB*, unsigned int, float*, unsigned int, const float*, const float*, ZCAMERA*, ZBoneModifyBase*)
    {
        // Do nothing
    }

    // PC 0x00761588 (nullsub)
    void ZRender::DrawMemory(unsigned int)
    {
        // Do nothing
    }

    // PC 0x0076158C (nullsub)
    void ZRender::DrawMemory(const unsigned int*, const unsigned int)
    {
        // Do nothing
    }

    // PC 0x00761590 (nullsub)
    void ZRender::DrawDebugObjectPrim(const SPrims*, float*, float*, unsigned int, unsigned int)
    {
        // Do nothing
    }

    // PC 0x00761594 (nullsub)
    void ZRender::ToggleMode(int)
    {
        // Do nothing
    }

    // PC 0x00761598 (nullsub)
    void ZRender::ResetFadeingLights(ZLNKOBJ*)
    {
        // Do nothing
    }

    // PC 0x0076159C (nullsub)
    void ZRender::SetKeepInside(bool)
    {
        // Do nothing
    }

    // PC 0x0069D5C0 (shared return-false stub)
    bool ZRender::GetKeepInside()
    {
        return false;
    }

    // PC 0x007615A4 (nullsub)
    void ZRender::Dump(const char*)
    {
        // Do nothing
    }

    // PC 0x00468880
    LINKSORTREFTAB* ZRender::GetCameraList()
    {
        return &m_Camera;
    }

    // PC 0x00468AB0
    IView* ZRender::CreateView(unsigned int lViewId, ZDrawSurface::TARGET lTarget)
    {
        return m_pRenderDraw->CreateView(this, lViewId, lTarget);
    }

    // PC 0x00468AD0
    IView* ZRender::FindView(unsigned int lViewId)
    {
        return m_pRenderDraw->FindView(lViewId);
    }

    // PC 0x00468AE0
    IView* ZRender::GetViewByIndex(int lIndex)
    {
        return m_pRenderDraw->GetViewByIndex(lIndex);
    }

    // PC 0x00468AF0
    bool ZRender::RemoveView(IView* pView)
    {
        return m_pRenderDraw->RemoveView(pView);
    }

    // PC 0x00469930
    void ZRender::AddCamera(ZCAMERA* pCamera, unsigned int lViewId, float fPrio)
    {
        pCamera->CameraListPri = static_cast<int>(fPrio);

        if (!m_Camera.Exists(pCamera->GetRef()))
        {
            m_Camera.AddSort(pCamera->GetRef(), fPrio, 0);
            InitCamera(pCamera);

            if (m_pRenderDraw && m_pRenderDraw->m_bInitialized)
                AddCameraToView(pCamera, lViewId);
        }
    }

    // PC 0x00468A00 (non-virtual helper)
    void ZRender::AddCameraToView(ZCAMERA* pCamera, uint32_t lViewId)
    {
        if (IView* pView = FindView(lViewId))
        {
            pView->AddCamera(pCamera);
            return;
        }

        const uint32_t lViewport[4] = { 0, 0, static_cast<uint32_t>(GetSizeX()), static_cast<uint32_t>(GetSizeY()) };

        IView* pNewView = CreateView(0, ZDrawSurface::SCREEN);
        pNewView->EnablePostfilter();
        pNewView->SetViewport(lViewport);
        pNewView->AddCamera(pCamera);
    }

    // PC 0x00468960 (pCamera->Init(this))
    void ZRender::InitCamera(ZCAMERA* pCamera)
    {
        pCamera->Init(this);
    }

    // PC 0x00468980. Cleans dead references out of the camera list, then returns the iCamIndex-th one.
    uint32_t ZRender::GetCamera(int lIndex)
    {
        RefRun run;
        m_Camera.RunInitNxtRef(&run);
        for (uint32_t rRef = m_Camera.RunNxtRef(&run); run; rRef = m_Camera.RunNxtRef(&run))
        {
            if (!ZGEOM::RefToPtr(rRef))
                m_Camera.RunDelRef(&run);
        }

        return m_Camera.GetRefNr(lIndex);
    }

    // PC 0x00468B00
    void ZRender::RemoveCamera(ZCAMERA* pCamera, unsigned int lViewId)
    {
        if (m_Camera.Exists(pCamera->GetRef()))
        {
            m_Camera.Remove(pCamera->GetRef());

            if (IView* pView = FindView(lViewId))
                pView->RemoveCamera(pCamera);
        }
    }

    // PC 0x00468B60 (m_Camera.Clear)
    void ZRender::RemoveCameras()
    {
        m_Camera.Clear();
    }

    // PC 0x00468B70
    bool ZRender::ChkCamera(ZCAMERA* pCamera)
    {
        return m_Camera.Exists(pCamera->GetRef());
    }

    // PC 0x00468B90
    void ZRender::SetCameraPrio(ZCAMERA* pCamera, float fPrio)
    {
        if (m_Camera.Exists(pCamera->GetRef()))
        {
            if (ZREF* pRef = m_Camera.Find(pCamera->GetRef()); pRef)
                m_Camera.SetSortNr(pRef, fPrio);
        }
    }

    // PC 0x00468CF0. This is the base ZRender vtable's Update implementation (base vtable
    // slot 43 at 0x007615D8 points straight here). The 0x0047E680 "ZRender::Update" wrapper
    // (checks m_bDisableRender at +0x1348, a derived-class field) is the derived override and
    // belongs to ZRenderX86/Wintel/D3D, not to the base class.
    void ZRender::Update()
    {
        if (!g_pSysInterface->m_bInitialized || !m_bInitialized || !g_pRenderDll->m_pPrimBuffer)
            return;

        m_lTriangleCount = 0;
        m_lTriangleCountBones = 0;
        m_lStripCount = 0;
        m_lPrimitiveCount = 0;
        m_lSpriteCount = 0;
        m_lSubPrimitiveCount = 0;
        m_lTextureCount = 0;
        m_lTextureSize = 0;
        m_lBoneCount = 0;
        m_lSubBoneCount = 0;

        if (BeginScene())
        {
            UpdateCameras();

            if (Animation::Manager* pAnimManager = g_pSysInterface->m_pEngineData->m_AnimationManager; pAnimManager)
            {
                if (Animation::StateCache* pStateCache = pAnimManager->m_Cache; pStateCache)
                    pStateCache->Update();
            }

            m_RemNxtLine = m_NxtLine;
            m_NxtLine = 10;
            UpdateShortCuts();

            // FPS / triangles-per-second counter (PC 0x00468CF0).
            // TIMETYPE is fixed-point at 1024 ticks/second, so elapsed * (1.0f / 1024.0f) gives seconds.
            g_lNumTris += m_StatCounters.m_lTriangleCount;
            if ((g_lTrisPerSecFlags & 1) == 0)
            {
                g_lTrisPerSecFlags |= 1;
                g_ttLastTime = {};
            }

            const double fElapsed = static_cast<double>(static_cast<int32_t>(g_pSysInterface->m_fActualTime.secs - g_ttLastTime.secs)) * (1.0 / 1024.0);
            if (fElapsed > 1.0)
            {
                g_fTrisPerSec = static_cast<double>(static_cast<uint32_t>(g_lNumTris)) / fElapsed;
                g_ttLastTime = g_pSysInterface->m_fActualTime;
                g_lNumTris = 0;
            }

            EndScene();

            if (!m_bFlipDisabled)
                Flip();
        }
        else
        {
            Flip();
        }

        std::memset(&m_StatCounters, 0, sizeof(m_StatCounters));
    }

    // PC 0x00468CA0
    void ZRender::UpdateCameras()
    {
        m_pRenderDraw->BeginFrame();
        m_pRenderDraw->Update(this);
        m_pRenderDraw->EndFrame();
    }

    // PC 0x007615E0 (empty stub)
    void ZRender::HandleShadows()
    {
        // Do nothing
    }

    // PC 0x00468C60
    int ZRender::GetSizeX()
    {
        return m_Size[0];
    }

    // PC 0x00468C70
    int ZRender::GetSizeY()
    {
        return m_Size[1];
    }

    // PC 0x00468C80
    void ZRender::SetSize(int lSizeX, int lSizeY)
    {
        m_Size[0] = lSizeX;
        m_Size[1] = lSizeY;
    }

    // PC 0x00468890
    float ZRender::ScreenAspectXY()
    {
        return static_cast<float>(m_Size[0]) / static_cast<float>(m_Size[1]);
    }

    // PC 0x00483050 (shared return-1.0f stub)
    float ZRender::PixelAspectXY()
    {
        return 1.0f;
    }

    // PC 0x00468EF0
    int ZRender::GetTextSizeX()
    {
        return static_cast<int>(static_cast<float>(GetSizeX()) / GetLetterSizeX());
    }

    // PC 0x00468F20
    int ZRender::GetTextSizeY()
    {
        return static_cast<int>(static_cast<float>(GetSizeY()) / GetLetterSizeY());
    }

    // PC 0x00483050 (shared return-1.0f stub)
    float ZRender::GetLetterSizeX()
    {
        return 1.0f;
    }

    // PC 0x00483050 (shared return-1.0f stub)
    float ZRender::GetLetterSizeY()
    {
        return 1.0f;
    }

    // PC 0x00468F50
    void ZRender::SetFontSize(const float* pSize)
    {
        if (pSize[0] > 0.0f && pSize[1] > 0.0f)
        {
            m_fFontSize[0] = pSize[0];
            m_fFontSize[1] = pSize[1];
        }
    }

    // PC 0x00468F90
    void ZRender::GetFontSize(float* pSize)
    {
        pSize[0] = m_fFontSize[0];
        pSize[1] = m_fFontSize[1];
    }

    // PC 0x00761510 (nullsub)
    void ZRender::SetEnvironment(float, int)
    {
        // Do nothing
    }

    // PC 0x00761514 (nullsub)
    void ZRender::PlotStatCounters(int)
    {
        // Do nothing
    }

    // PC 0x00468BE0
    void ZRender::GetMouse(float* pMousePos)
    {
        pMousePos[0] = m_MouseX;
        pMousePos[1] = m_MouseY;
    }

    // PC 0x00468C00
    void ZRender::GetMouse3D(float* pMousePos)
    {
        pMousePos[0] = m_MouseX;
        pMousePos[1] = m_MouseY;
        pMousePos[2] = m_MouseZ;
    }

    // PC 0x00468C30
    void ZRender::GetMouseButtons(bool* pLeft, bool* pMiddle, bool* pRight)
    {
        *pLeft = m_bLeftMouseButtonDown;
        *pMiddle = m_bMiddleMouseButtonDown;
        *pRight = m_bRightMouseButtonDown;
    }

    // PC 0x00761524 (_purecall) - pure virtual, no base implementation

    // PC 0x0076152C (empty stub)
    void ZRender::UPlotF(int, int, const char*)
    {
        // Do nothing
    }

    // PC 0x00468FB0
    void ZRender::UPlotFNxt(const char* pText)
    {
        UPlotF(0, m_NxtLine++, pText);
    }

    // PC 0x00761534 (empty stub)
    void ZRender::SetViewport(const float, const float, const float, const float)
    {
        // Do nothing
    }

    // PC 0x00761538 (nullsub)
    void ZRender::ClearViewport(const unsigned int, const unsigned int)
    {
        // Do nothing
    }

    // PC 0x00761540 (empty stub)
    void ZRender::SetState()
    {
        // Do nothing
    }

    // PC 0x00761544 (empty stub)
    void ZRender::ResetState()
    {
        // Do nothing
    }

    // PC 0x00761548 (empty stub)
    void ZRender::PushState()
    {
        // Do nothing
    }

    // PC 0x0076154C (empty stub)
    void ZRender::PopState()
    {
        // Do nothing
    }

    // PC 0x00761550 (nullsub)
    void ZRender::SetProjection(const float, const float, const float, const float, const float, const float, const bool)
    {
        // Do nothing
    }

    // PC 0x00761554 (empty stub)
    void ZRender::SetModelView(const float*, const float*, const float, const float)
    {
        // Do nothing
    }

    // PC 0x00761558 (empty stub)
    void ZRender::SetFog(const float, const float, const unsigned int)
    {
        // Do nothing
    }

    // PC 0x006B3640 (return-nullptr stub)
    const char* ZRender::GetRenderName()
    {
        return nullptr;
    }

    // PC 0x0076155C (empty stub)
    void ZRender::ProgressBar(const float, const unsigned int, const unsigned int)
    {
        // Do nothing
    }

    // PC 0x00468FF0
    void ZRender::RemoveBaseGeom(ZBaseGeom* pBaseGeom)
    {
        m_pRenderDraw->RemoveBaseGeom(pBaseGeom, true);
    }

    // PC 0x00469010
    void ZRender::UpdateBaseGeom(ZBaseGeom* pBaseGeom)
    {
        m_pRenderDraw->UpdateBaseGeom(pBaseGeom);
    }

    // PC 0x00469020
    void ZRender::ChangePrim(ZBaseGeom* pBaseGeom, unsigned int lPrimId)
    {
        m_pRenderDraw->ChangePrim(pBaseGeom, lPrimId);
    }

    // PC 0x00761560 (empty stub)
    void ZRender::BeginShadows()
    {
        // Do nothing
    }

    // PC 0x00761564 (nullsub)
    void ZRender::AddShadow(ZBaseGeom*, ZBaseGeom*, ZBaseGeom*, ZBaseGeom**, unsigned int)
    {
        // Do nothing
    }

    // PC 0x00761568 (empty stub)
    void ZRender::EndShadows()
    {
        // Do nothing
    }

    // PC 0x00468910
    void ZRender::SetEditCursor(int bVisible)
    {
        m_fShowEditCursor = (bVisible != 0) ? 1.0f : 0.0f;
    }

    // PC 0x00468940
    int ZRender::GetEditCursor()
    {
        return m_fShowEditCursor != 0.0f;
    }

    // PC 0x00468CD0
    uint32_t ZRender::SetMaxFrameInterval(uint32_t lMaxFrameInterval)
    {
        const uint32_t lOldInterval = m_lMaxFrameInterval;
        m_lMaxFrameInterval = lMaxFrameInterval;
        return lOldInterval;
    }

    // PC 0x00761568 (empty stub)
    void ZRender::UpdateShortCuts()
    {
        // Do nothing
    }
}
