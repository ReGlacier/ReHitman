#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Com/CCOMType.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/GUI/Frame/ZFRAME.h>
#include <Glacier/GUI/Font/ZTTFONT.h>
#include <Glacier/GUI/ZLINEOBJ.h>
#include <Glacier/GUI/ZWINPIC.h>
#include <Glacier/GUI/ZSlider.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphicPart.h>
#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Render/View/IView.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZDllBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZMessageResolver.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        ZMessageResolver g_msgCreateViews("CreateViews");

        void PauseMenuAudio(bool bPause)
        {
            if (ZSoundDllBase* pSoundDll = g_pSysInterface->GetSoundDll())
                pSoundDll->Pause(bPause, true);
        }
    }

    ZXMLGUISystem::ZXMLGUISystem()
        : CWinEvent<ZWINDOW>()
    {
        m_bUnpauseGame = true;
        m_pWinGroupResources = nullptr;
        m_pGroupFonts = nullptr;
        m_iLastFocusedElement = -1;
        m_iNumOfWindows = 0;
        m_iNumOtherWindows = 0;
        m_pWinGroupHeader = nullptr;
        m_pMenuElements = g_pGameData->m_pMenuElements;
        m_iPlayerActivatedMenu = 0;
        m_bInsideUpdate = false;
        m_bCloseSystem = true;
        m_bGot3dBackground = false;
        m_bAddBackgroundCamAnd3DCams = false;
        m_pBackgroundCam = nullptr;
        m_pMenuCam = nullptr;
        m_pDialogCam = nullptr;
        m_pTRCCam = nullptr;
        m_eCurrentLayer = 0;
        m_bWindowPush = false;
        m_bRollBack = false;
        m_iNumOfTRCWindows = 0;
    }

    ZXMLGUISystem::~ZXMLGUISystem() = default;

    int ZXMLGUISystem::Command(Glacier::ZMSGID command, Glacier::ZDATA data)
    {
        if (g_msgCreateViews == command)
        {
            if (data)
                m_bAddBackgroundCamAnd3DCams = *static_cast<const bool*>(data);
            return 1;
        }

        if (command == 0x8000 && WndMessage(static_cast<ZWMEVENT*>(data)))
            static_cast<ZWMEVENT*>(data)->Return = true;

        return 0;
    }

    bool ZXMLGUISystem::OnSliderChange(ZREF rSlider, uint32_t iValue)
    {
        ZGEOM* pGeom = ZGEOM::RefToPtr(rSlider);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZCONTROL>());

        ZCONTROL* pControl = static_cast<ZCONTROL*>(pGeom);
        if (IGUIElement* pElement = m_pMenuElements->GetGUIElement(pControl->GetControlId()))
            pElement->SetValue(static_cast<int>(iValue));

        return true;
    }

    void ZXMLGUISystem::SetFocus()
    {
        GetSystem()->SetFocusedControl(static_cast<ZWINGROUP*>(GetGeom()));
    }

    void ZXMLGUISystem::AddOtherWindowCount(int iAmount)
    {
        m_iNumOtherWindows += iAmount;
    }

    void ZXMLGUISystem::SetupCameras()
    {
        CCom* pSceneCom = g_pEngineData->GetSceneCom();
        ZRender* pRender = g_pSysInterface->WindowFirst;
        const uint32_t aViewport[4] = {
            0,
            0,
            static_cast<uint32_t>(pRender->GetSizeX()),
            static_cast<uint32_t>(pRender->GetSizeY())
        };

        bool b3dCamExists = false;
        pSceneCom->GetVal("3dBackgroundCamExists", &b3dCamExists);

        if (!b3dCamExists)
        {
            int iRef = 0;
            pSceneCom->GetVal("3dBackgroundCam", &iRef);
            ZGEOM* p3dCam = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (p3dCam)
            {
                IView* pView = pRender->CreateView(1, ZDrawSurface::SCREEN);
                pView->SetViewport(aViewport);
                pView->AddCamera(static_cast<ZCAMERA*>(p3dCam));
                pView->EnablePostfilter();
            }
        }

        m_bGot3dBackground = !b3dCamExists;

        int iRef = 0;
        pSceneCom->GetVal("BackgroundCam", &iRef);
        ZCAMERA* pBgCam = static_cast<ZCAMERA*>(ZGEOM::RefToPtr(static_cast<ZREF>(iRef)));
        m_pBackgroundCam = pBgCam;

        if (pBgCam)
        {
            IView* pView = pRender->CreateView(2, ZDrawSurface::SCREEN);
            pView->SetViewport(aViewport);

            ZASSERT(pBgCam->IsDerivedFrom<ZCAMERA>());
            pView->AddCamera(pBgCam);

            ZBaseGeom* pParentBase = pBgCam->BaseGeom()->m_pParent;
            if (pParentBase)
            {
                ZGEOM* pParentGroup = reinterpret_cast<ZGEOM*>(pParentBase);
                pParentGroup->m_baseGeom->m_lControl |= 0x400u;
                pBgCam->BaseGeom()->m_pParent = nullptr;
                pBgCam->m_lGeomControl |= 0x200000u;
                if (!m_bGot3dBackground)
                    pBgCam->m_lGeomControl |= 0x8000u;
                pBgCam->MakeActive();
            }
        }

        m_ResourceManager.CreateMenu3DViews(0x85);

        iRef = 0;
        pSceneCom->GetVal("MenuCam", &iRef);
        ZCAMERA* pMenuCam = static_cast<ZCAMERA*>(ZGEOM::RefToPtr(static_cast<ZREF>(iRef)));
        m_pMenuCam = pMenuCam;

        if (pMenuCam)
        {
            IView* pView = pRender->CreateView(3, ZDrawSurface::SCREEN);
            pView->SetViewport(aViewport);
            ZASSERT(pMenuCam->IsDerivedFrom<ZCAMERA>());
            pView->AddCamera(pMenuCam);
            pMenuCam->MakeActive();
            ZASSERT(GetSystem()->m_pSystem != nullptr);
            m_aMenuLayer[0].pParent = static_cast<ZWINGROUP*>(GetSystem());
        }

        iRef = 0;
        pSceneCom->GetVal("DialogCam", &iRef);
        ZCAMERA* pDialogCam = static_cast<ZCAMERA*>(ZGEOM::RefToPtr(static_cast<ZREF>(iRef)));
        m_pDialogCam = pDialogCam;

        if (pDialogCam)
        {
            IView* pView = pRender->CreateView(177, ZDrawSurface::SCREEN);
            pView->SetViewport(aViewport);
            ZASSERT(pDialogCam->IsDerivedFrom<ZCAMERA>());
            pView->AddCamera(pDialogCam);
            pDialogCam->MakeActive();

            ZBaseGeom* pParentBase = pDialogCam->BaseGeom()->m_pParent;
            if (pParentBase)
            {
                ZGEOM* pParentGroup = reinterpret_cast<ZGEOM*>(pParentBase);
                pParentGroup->m_baseGeom->m_lControl |= 0x400u;
                pDialogCam->m_lGeomControl |= 0x208000u;
                pDialogCam->BaseGeom()->m_pParent = nullptr;
            }
        }

        iRef = 0;
        pSceneCom->GetVal("TRCCam", &iRef);
        ZCAMERA* pTRCCam = static_cast<ZCAMERA*>(ZGEOM::RefToPtr(static_cast<ZREF>(iRef)));
        m_pTRCCam = pTRCCam;

        if (pTRCCam)
        {
            IView* pView = pRender->CreateView(178, ZDrawSurface::SCREEN);
            pView->SetViewport(aViewport);
            ZASSERT(pTRCCam->IsDerivedFrom<ZCAMERA>());
            pView->AddCamera(pTRCCam);
            pTRCCam->MakeActive();

            ZBaseGeom* pParentBase = pTRCCam->BaseGeom()->m_pParent;
            if (pParentBase)
            {
                ZGEOM* pParentGroup = reinterpret_cast<ZGEOM*>(pParentBase);
                pParentGroup->m_baseGeom->m_lControl |= 0x400u;
                pTRCCam->m_lGeomControl |= 0x208000u;
                pTRCCam->BaseGeom()->m_pParent = nullptr;
            }
        }

        iRef = 0;
        pSceneCom->GetVal("MouseCam", &iRef);
        ZCAMERA* pMouseCam = static_cast<ZCAMERA*>(ZGEOM::RefToPtr(static_cast<ZREF>(iRef)));

        if (pMouseCam)
        {
            IView* pView = pRender->CreateView(179, ZDrawSurface::SCREEN);
            pView->SetViewport(aViewport);
            ZASSERT(pMouseCam->IsDerivedFrom<ZCAMERA>());
            pView->AddCamera(pMouseCam);
            pMouseCam->MakeActive();

            ZBaseGeom* pParentBase = pMouseCam->BaseGeom()->m_pParent;
            if (pParentBase)
            {
                pMouseCam->m_lGeomControl |= 0x208000u;
                reinterpret_cast<ZGEOM*>(pParentBase)->m_baseGeom->m_lControl |= 0x400u;
                pMouseCam->BaseGeom()->m_pParent = nullptr;
            }
        }

        iRef = 0;
        pSceneCom->GetVal("MenuZWindows", &iRef);
        ZGEOM* pMenuZWindows = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
        ZASSERT(pMenuZWindows && (ZWINDOWS::m_Mask & pMenuZWindows->GetObjectId()) == ZWINDOWS::m_Id);
    }

    IWindowInterface* ZXMLGUISystem::GetTopWindow()
    {
        if (!m_iNumOfWindows)
            return nullptr;

        // Original: *(this + 4 * m_iNumOfWindows + 0x320)
        // This is equivalent to m_apWindowStack[m_iNumOfWindows - 1]
        return m_apWindowStack[m_iNumOfWindows - 1];
    }

    void ZXMLGUISystem::ChangeLayer(EMenuLayer eLayer)
    {
        int32_t iOldLayer = m_eCurrentLayer;
        if (eLayer == iOldLayer)
            return;

        if (eLayer <= iOldLayer)
        {
            m_aMenuLayer[iOldLayer].vReceiveAllInput.clear();
            m_eCurrentLayer = eLayer;
            if (m_aMenuLayer[eLayer].pOverlayFrame)
            {
                ReleaseOverlayFrame(m_aMenuLayer[eLayer]);
                m_aMenuLayer[eLayer].pOverlayFrame = nullptr;
            }
        }
        else
        {
            AddOverlayFrame(m_aMenuLayer[iOldLayer]);
            m_aMenuLayer[eLayer].vReceiveAllInput.clear();
            m_eCurrentLayer = eLayer;
        }

        ZWINDOWS* pWindows = GetSystem();
        pWindows->m_pControlInFocus = m_aMenuLayer[m_eCurrentLayer].pParent;
    }

    void ZXMLGUISystem::Cancel()
    {
        if (m_iNumOfWindows)
        {
            m_apWindowStack[m_iNumOfWindows - 1]->Cancel();
        }
    }

    int32_t ZXMLGUISystem::GetOpenWindowsCount() const
    {
        return m_iNumOfWindows;
    }

    IGUIElement* ZXMLGUISystem::GetElementInFocus()
    {
        ZWINGROUP* pFocused = GetSystem()->GetFocusedControl();
        if (!pFocused || !pFocused->IsDerivedFrom<ZCONTROL>())
            return nullptr;

        const int iControlId = static_cast<ZCONTROL*>(pFocused)->GetControlId();
        return iControlId == 5000 ? nullptr : m_pMenuElements->GetGUIElement(iControlId);
    }

    void ZXMLGUISystem::AddTRCWindow(IWindowInterface* pWnd)
    {
        int32_t iPriority = pWnd->GetTRCPriority();
        int32_t iInsertIndex = 0;

        if (m_iNumOfTRCWindows > 0)
        {
            for (int32_t i = 0; i < m_iNumOfTRCWindows; ++i)
            {
                if (m_apTRCWindows[i]->GetTRCPriority() == iPriority)
                {
                    iInsertIndex = i;
                    break;
                }
            }
        }

        if (m_iNumOfTRCWindows >= 4)
            ZASSERT(false);

        if (iInsertIndex >= m_iNumOfTRCWindows)
            iInsertIndex = m_iNumOfTRCWindows;

        for (int32_t i = m_iNumOfTRCWindows - 1; i >= iInsertIndex; --i)
            m_apTRCWindows[i + 1] = m_apTRCWindows[i];

        m_apTRCWindows[iInsertIndex] = pWnd;
        ++m_iNumOfTRCWindows;
    }

    void ZXMLGUISystem::ReleaseOverlayFrame(SMenuLayer& rLayer)
    {
        if (rLayer.pOverlayFrame)
        {
            m_ResourceManager.m_pWinGroupFrames->AttachGeom(rLayer.pOverlayFrame, true);
            rLayer.pOverlayFrame = nullptr;
        }
    }

    void ZXMLGUISystem::AddOverlayFrame(SMenuLayer& rLayer)
    {
        if (rLayer.pOverlayFrame)
            return;

        ZVector2 vPos {};
        ZVector2 vSize {
            static_cast<float>(g_pSysInterface->m_lResolution[0]),
            static_cast<float>(g_pSysInterface->m_lResolution[1])
        };

        if (m_iNumOfWindows <= 1)
        {
            rLayer.pParent->SetPos(0.0f, 0.0f, 0.0f);
        }
        else
        {
            IWindowInterface* pCoveredWindow = m_apWindowStack[m_iNumOfWindows - 2];
            if (pCoveredWindow->IsTRC() || pCoveredWindow->IsDialog())
            {
                vSize = pCoveredWindow->m_v2WindowSize;
            }
            else
            {
                const ZVector3& vParentPos = rLayer.pParent->Pos();
                vPos.x = -vParentPos.x;
                vPos.y = -vParentPos.y;
            }
        }

        ZColorSet colorSet;
        rLayer.pOverlayFrame = m_ResourceManager.GetFrame(
            vPos, &colorSet, rLayer.pParent, vSize, "MenuOverlay*", ELEFT);
    }

    void ZXMLGUISystem::AddRecieveAllInput(IGUIElement* pElement)
    {
        ZStaticVector<IGUIElement*, 16>& rVec = m_aMenuLayer[m_eCurrentLayer].vReceiveAllInput;

        if (rVec.m_iSize >= 16)
            ZASSERT(false);

        rVec.m_Data[rVec.m_iSize++] = pElement;
    }

    void ZXMLGUISystem::RemoveRecieveAllInput(IGUIElement* pElement)
    {
        ZStaticVector<IGUIElement*, 16>& rVec = m_aMenuLayer[m_eCurrentLayer].vReceiveAllInput;

        uint32_t iSize = rVec.m_iSize;
        for (uint32_t i = 0; i < iSize; ++i)
        {
            if (rVec.m_Data[i] == pElement)
            {
                for (uint32_t j = i; j < iSize - 1; ++j)
                    rVec.m_Data[j] = rVec.m_Data[j + 1];
                --rVec.m_iSize;
                return;
            }
        }
    }

    void ZXMLGUISystem::SendEventToRecieveAll(uint32_t iKey, IGUIElement* pElement)
    {
        ZStaticVector<IGUIElement*, 16>& rVec = m_aMenuLayer[m_eCurrentLayer].vReceiveAllInput;

        for (uint32_t i = 0; i < rVec.m_iSize; ++i)
        {
            if (rVec.m_Data[i] != pElement)
                rVec.m_Data[i]->Click(static_cast<eZWUserEvents>(iKey), 0, this);
        }
    }

    void ZXMLGUISystem::StopAndSetMusicFlags()
    {
        CCom* pSceneCom = g_pEngineData->GetSceneCom();

        int iRef = 0;
        ZGEOM* pGeom = nullptr;
        ZSoundObject* pSnd = nullptr;

        pSceneCom->GetVal("CreditsMusic", &iRef);
        pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
        if (pGeom && (pSnd = static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject()))
            pSnd->SetActive(false);

        iRef = 0;
        pSceneCom->GetVal("TitleMusic", &iRef);
        pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
        if (pGeom && (pSnd = static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject()))
            pSnd->SetActive(false);

        iRef = 0;
        pSceneCom->GetVal("TraningInfoAmbient", &iRef);
        pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
        if (pGeom && (pSnd = static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject()))
            pSnd->SetActive(false);

        iRef = 0;
        pSceneCom->GetVal("PremissionAmbient", &iRef);
        pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
        if (pGeom && (pSnd = static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject()))
            pSnd->SetActive(false);

        iRef = 0;
        pSceneCom->GetVal("MissionBriefing", &iRef);
        pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
        if (pGeom && (pSnd = static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject()))
            pSnd->SetActive(false);
    }

    ZWINDOWS* ZXMLGUISystem::GetTopUIComponent()
    {
        ZWINDOWS* pWindows = GetSystem();
        ZWINGROUP* pFocused = pWindows->GetFocusedControl();
        if (!pFocused)
            return pWindows;

        ZWINGROUP* pTop = pFocused;
        while (pTop->BaseGeom()->m_pParent)
            pTop = reinterpret_cast<ZWINGROUP*>(pTop->BaseGeom()->m_pParent);

        return static_cast<ZWINDOWS*>(pTop);
    }

    void ZXMLGUISystem::HandleCachedFunctions()
    {
        if (m_OpenWindowParams.bOpenWindow)
        {
            m_OpenWindowParams.bOpenWindow = false;
            OpenWindow(m_OpenWindowParams.sWindow.c_str(), m_OpenWindowParams.bPushOnStack, m_OpenWindowParams.bResetStack);
        }

        if (m_CloseWindowParams.bCloseWindow)
        {
            m_CloseWindowParams.bCloseWindow = false;
            CloseWindow(m_CloseWindowParams.bCloseAll);
        }

        if (m_RollBackParams.bRollBack)
        {
            m_RollBackParams.bRollBack = false;
            const char* pszPush = m_RollBackParams.sPushWindow.c_str();
            bool bPush = (m_RollBackParams.sPushWindow != m_RollBackParams.sWindow);
            RollBack(m_RollBackParams.sWindow.c_str(), pszPush, bPush, m_RollBackParams.bKeepRollbackWindow);
        }
    }

    void ZXMLGUISystem::OpenWindow(const char* pszWindowName, bool bPushOnStack, bool bResetStack)
    {
        if (m_bInsideUpdate)
        {
            m_OpenWindowParams.bOpenWindow = true;
            m_OpenWindowParams.sWindow = pszWindowName;
            m_OpenWindowParams.bPushOnStack = bPushOnStack;
            m_OpenWindowParams.bResetStack = bResetStack;
            return;
        }

        if (!m_bRollBack)
            m_bWindowPush = true;
        m_bRollBack = true;

        IWindowInterface* pWindow = m_pMenuElements->GetWindow(pszWindowName);
        if (!pWindow)
        {
            m_iLastFocusedElement = -1;
            m_bWindowPush = false;
            m_bRollBack = false;
            return;
        }

        if (pWindow->m_bPauseEngine)
        {
            if (!m_bCloseSystem)
            {
                g_pEngineData->m_bPause = true;
                PauseMenuAudio(true);
            }
        }
        else if (m_bUnpauseGame)
        {
            g_pEngineData->m_bPause = false;
            PauseMenuAudio(false);
        }

        if (g_pGameData)
        {
            g_pGameData->GetAudioOSDInterface().StopCurrentSound();
            ZSoundObject* pSnd = g_pGameData->GetAudioOSDInterface().PlaySound(10);
            if (pSnd)
                pSnd->SetActive(false);
            g_pGameData->GetAudioEffectsInterface().StopCurrentSound();
        }

        StopAndSetMusicFlags();

        g_pEngineData->GetSceneCom()->SetVal("TraningInfoDialog", 0u, CCOM_TYPE_REF);

        IWindowInterface* pTopWindow = m_iNumOfWindows > 0 ? m_apWindowStack[m_iNumOfWindows - 1] : nullptr;
        EMenuLayer eLayer = pWindow->IsTRC() ? ETRC : (pWindow->IsDialog() ? EDIALOG : EMENU);

        if (pTopWindow)
        {
            if (eLayer == EDIALOG)
            {
                if (pTopWindow->IsTRC())
                    ZASSERT(false);
            }
            else if (eLayer == EMENU)
            {
                if (pTopWindow->IsTRC())
                    ZASSERT(false);

                ZWINDOWS* pWindows = GetSystem();
                ZASSERT(pWindows->m_pSystem != nullptr);
                pWindows->m_pSystem = pWindows;
                pTopWindow->CloseWindow(GetResourceManager(), false);

                if (pTopWindow->IsDialog())
                {
                    ChangeLayer(EMENU);
                    --m_iNumOfWindows;
                    if (m_iNumOfWindows > 0)
                        m_apWindowStack[m_iNumOfWindows - 1]->CloseWindow(GetResourceManager(), false);
                }
            }
            else
            {
                if (eLayer == GetWindowLayer(pTopWindow))
                {
                    int32_t iTopPriority = pTopWindow->GetTRCPriority();
                    int32_t iNewPriority = pWindow->GetTRCPriority();

                    if (iTopPriority <= iNewPriority)
                    {
                        pTopWindow->CloseWindow(GetResourceManager(), false);
                        if (iTopPriority < iNewPriority)
                            AddTRCWindow(pTopWindow);
                    }
                    else
                    {
                        AddTRCWindow(pWindow);
                    }
                }
            }

            CCom* pSC = g_pEngineData->GetSceneCom();
            int iRef = 0;
            ZGEOM* pG = nullptr;
            ZSoundObject* pS = nullptr;

            pSC->GetVal("CreditsMusic", &iRef);
            pG = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (pG && (pS = static_cast<ZSNDOBJ*>(pG)->GetSoundObject()))
                pS->SetActive(true);

            iRef = 0;
            pSC->GetVal("TitleMusic", &iRef);
            pG = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (pG && (pS = static_cast<ZSNDOBJ*>(pG)->GetSoundObject()))
                pS->SetActive(true);

            iRef = 0;
            pSC->GetVal("TraningInfoAmbient", &iRef);
            pG = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (pG && (pS = static_cast<ZSNDOBJ*>(pG)->GetSoundObject()))
                pS->SetActive(true);

            iRef = 0;
            pSC->GetVal("PremissionAmbient", &iRef);
            pG = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (pG && (pS = static_cast<ZSNDOBJ*>(pG)->GetSoundObject()))
                pS->SetActive(true);

            iRef = 0;
            pSC->GetVal("MissionBriefing", &iRef);
            pG = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (pG && (pS = static_cast<ZSNDOBJ*>(pG)->GetSoundObject()))
                pS->SetActive(true);
        }

        if (bResetStack)
        {
            m_iNumOfWindows = 0;
            for (int32_t i = 0; i < 3; ++i)
                m_aMenuLayer[i].vReceiveAllInput.clear();
        }
        else if (!bPushOnStack)
        {
            m_apWindowStack[m_iNumOfWindows] = pWindow;
        }
        else
        {
            if (m_iNumOfWindows >= 32)
                ZASSERT(false);

            if (m_iNumOfWindows > 0)
                m_aLastIndex[m_iNumOfWindows] = m_iLastFocusedElement;

            m_apWindowStack[m_iNumOfWindows] = pWindow;
            ++m_iNumOfWindows;
        }

        ChangeLayer(eLayer);
        OpenWindow(pWindow, true, m_aMenuLayer[m_eCurrentLayer].pParent);

        ZWINDOWS* pWindows = GetSystem();
        pWindows->InvalidateView();
        pWindows->InvalidateView();

        m_iLastFocusedElement = -1;
        ZWINDOWS* pTopUI = GetTopUIComponent();
        if (pTopUI)
        {
            ZWINGROUP* pFocused = pTopUI->GetFocusedControl();
            if (pFocused && pFocused->IsDerivedFrom<ZCONTROL>())
            {
                ZCONTROL* pCtrl = static_cast<ZCONTROL*>(pFocused);
                m_iLastFocusedElement = pCtrl->GetControlId();
            }
        }

        m_bWindowPush = false;
        m_bRollBack = false;
        if (m_iNumOfWindows > 0)
        {
            IWindowInterface* pTop = m_apWindowStack[m_iNumOfWindows - 1];
            m_bWindowPush = pTop->IsDialog();
        }
    }

    void ZXMLGUISystem::OpenWindow(IWindowInterface* pWindowInterface, bool bPushWindow, ZWINGROUP* pParent)
    {
        ZVector2 vViewport;
        vViewport.x = static_cast<float>(g_pSysInterface->m_lResolution[0]);
        vViewport.y = static_cast<float>(g_pSysInterface->m_lResolution[1]);

        ZVector2 vResult {};
        pWindowInterface->GetTopLeftPos(vResult, vViewport);

        pParent->SetPos(vResult.x, vResult.y, 0.0f);

        pWindowInterface->OpenWindow(GetResourceManager(), bPushWindow, pParent, true);
    }

    void ZXMLGUISystem::CloseWindow(bool bCloseAll)
    {
        if (m_bInsideUpdate)
        {
            m_CloseWindowParams.bCloseWindow = true;
            m_CloseWindowParams.bCloseAll = bCloseAll;
            return;
        }

        if (m_iNumOfWindows > 0)
        {
            StopAndSetMusicFlags();

            IWindowInterface* pTopWindow = m_apWindowStack[m_iNumOfWindows - 1];
            pTopWindow->CloseWindow(GetResourceManager(), true);

            ZWINDOWS* pWindows = GetSystem();

            if (bCloseAll)
            {
                ZSDOwner& rOSD = g_pGameData->GetAudioOSDInterface();
                if (pTopWindow->m_bPopOnBack)
                {
                    if (m_iNumOfWindows - 1 > 0)
                        m_apWindowStack[m_iNumOfWindows - 1]->CloseWindow(GetResourceManager(), true);
                }
                m_iNumOfWindows = 0;
                ChangeLayer(EMENU);
            }
            else
            {
                --m_iNumOfWindows;
                g_pGameData->GetAudioEffectsInterface().PlaySound(13);
            }

            if (m_iNumOfWindows > 0)
            {
                ZWINDOWS* pTopUI = GetSystem();
                ZASSERT(pTopUI->m_pSystem != nullptr);
                pTopUI->m_pSystem = pTopUI;

                IWindowInterface* pNewTop = m_apWindowStack[m_iNumOfWindows - 1];
                ChangeLayer(GetWindowLayer(pNewTop));

                if (pNewTop->m_bPopOnBack || pNewTop->m_bRollBackMark)
                {
                    m_OpenWindowParams.bOpenWindow = false;
                    m_CloseWindowParams.bCloseWindow = false;
                    m_RollBackParams.bRollBack = false;
                    m_bInsideUpdate = true;
                    m_apWindowStack[m_iNumOfWindows - 1]->Invalidate();
                    m_bInsideUpdate = false;
                }
                else
                {
                    OpenWindow(pNewTop, false, m_aMenuLayer[m_eCurrentLayer].pParent);
                }

                if (pNewTop->m_bPauseEngine)
                {
                    if (!m_bCloseSystem)
                    {
                        g_pEngineData->m_bPause = true;
                        PauseMenuAudio(true);
                        StopAndSetMusicFlags();
                    }
                }
                else if (m_bUnpauseGame)
                {
                    g_pEngineData->m_bPause = false;
                    PauseMenuAudio(false);
                }

                m_iLastFocusedElement = m_aLastIndex[m_iNumOfWindows - 1];
                if (m_iLastFocusedElement != -1)
                {
                    IGUIElement* pElement = m_pMenuElements->GetGUIElement(m_iLastFocusedElement);
                    if (pElement)
                        pElement->Use();
                }
            }
            else if (pTopWindow->m_bRollBackMark)
            {
                ChangeLayer(EMENU);
            }

            (void)GetSystem()->GetObjectId();
        }

        if (m_bCloseSystem && m_iNumOfWindows == 0 && m_iNumOfTRCWindows <= 0)
        {
            m_ResourceManager.ReleaseTextGroup(m_pWinGroupResources);
            m_pWinGroupResources = nullptr;

            ZWINDOWS* pWindows = GetSystem();
            pWindows->m_pExternalMouseColiGroup = nullptr;
            pWindows->PopWindow();
            if (!m_iNumOtherWindows)
            {
                pWindows->SetShowMouse(false);
                pWindows->DisableClassCall(16);
                if (pWindows->m_pMainCamera)
                    pWindows->m_pMainCamera->DeactivateCam();
                if (m_pMenuCam)
                    m_pMenuCam->DeactivateCam();
            }

            DeactivateFrameUpdate();

            if (m_bUnpauseGame)
            {
                g_pEngineData->m_bPause = false;
                PauseMenuAudio(false);
                if (m_bUnpauseGame)
                    g_pGameData->GetAudioEffectsInterface().PlaySound(10);
            }

            CCom* pSceneCom = g_pEngineData->GetSceneCom();
            int iRef = 0;
            pSceneCom->GetVal("TraningInfoAmbient", &iRef);
            ZGEOM* pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (pGeom)
                static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject()->SetActive(false);

            if (m_pBackgroundCam)
                m_pBackgroundCam->DeactivateCam();
            if (m_pDialogCam)
                m_pDialogCam->DeactivateCam();
            if (m_pTRCCam)
                m_pTRCCam->DeactivateCam();

            Action::instance->DisableInputTimer(0.3);
        }

        if (m_iNumOfTRCWindows > 0)
        {
            IWindowInterface* pTRC = GetNextTRCWindow();
            if (pTRC)
                OpenWindow(pTRC, true, nullptr);
        }

        if (m_iNumOfWindows > 0)
            HandleCachedFunctions();
    }

    void ZXMLGUISystem::Set2DBackgroundCamClear(bool bCamClear)
    {
        if (bCamClear)
        {
            m_pBackgroundCam->CameraCon |= 0x8000u;
        }
        else
        {
            m_pBackgroundCam->CameraCon &= ~0x8000u;
        }
    }

    bool ZXMLGUISystem::Get2DBackgroundCamClear() const
    {
        return (m_pBackgroundCam->GetCameraCon() & 0x8000) == 0;
    }

    IWindowInterface* ZXMLGUISystem::GetNextTRCWindow()
    {
        if (!m_iNumOfTRCWindows)
            return nullptr;

        int32_t iBestIndex = 0;
        int32_t iBestPriority = 0;

        for (int32_t i = 0; i < m_iNumOfTRCWindows; ++i)
        {
            int32_t iPriority = m_apTRCWindows[i]->GetTRCPriority();
            ZASSERT(iPriority > 0);
            if (iPriority > iBestPriority)
            {
                iBestPriority = iPriority;
                iBestIndex = i;
            }
        }

        IWindowInterface* pResult = m_apTRCWindows[iBestIndex];
        if (iBestIndex != m_iNumOfTRCWindows - 1)
            m_apTRCWindows[iBestIndex] = m_apTRCWindows[m_iNumOfTRCWindows - 1];

        --m_iNumOfTRCWindows;
        return pResult;
    }

    ZResourceManager* ZXMLGUISystem::GetResourceManager()
    {
        return &m_ResourceManager;
    }

    ZXMLGUISystem::EMenuLayer ZXMLGUISystem::GetWindowLayer(IWindowInterface* pWnd) const
    {
        if (pWnd->IsTRC())
        {
            return ETRC;
        }

        return pWnd->IsDialog() ? EDIALOG : EMENU;
    }

    void ZXMLGUISystem::RollBack(const char* pszWindowName, const char* pszPushWindowName, bool bPushWindow, bool bKeepRollbackWindow)
    {
        if (m_bInsideUpdate)
        {
            m_RollBackParams.bRollBack = true;
            m_RollBackParams.sWindow = pszWindowName;
            m_RollBackParams.sPushWindow = bPushWindow ? pszPushWindowName : "";
            m_RollBackParams.bKeepRollbackWindow = bKeepRollbackWindow;
            return;
        }

        int32_t iNumWindows = m_iNumOfWindows;
        if (iNumWindows < 1)
            return;

        int32_t iTarget = iNumWindows - 1;

        if (pszWindowName && pszWindowName[0] != '\0')
        {
            while (iTarget >= 0)
            {
                if (strcmp(pszWindowName, m_apWindowStack[iTarget]->GetName()) == 0)
                    break;
                --iTarget;
            }
            if (iTarget < 0)
                ZASSERT(false);
        }
        else
        {
            while (iTarget >= 0)
            {
                if (m_apWindowStack[iTarget]->m_bRollBackMark)
                    break;
                --iTarget;
            }
            if (iTarget < 0)
                ZASSERT(false);
        }

        while (m_iNumOfWindows > iTarget)
        {
            --m_iNumOfWindows;
            m_apWindowStack[m_iNumOfWindows]->CloseWindow(GetResourceManager(), true);

            IWindowInterface* pTop = m_apWindowStack[m_iNumOfWindows];
            EMenuLayer eLayer = pTop->m_bRollBackMark ? EDIALOG : (pTop->IsDialog() ? EDIALOG : EMENU);
            m_aMenuLayer[eLayer].vReceiveAllInput.clear();
            if (m_aMenuLayer[eLayer].pOverlayFrame)
            {
                ReleaseOverlayFrame(m_aMenuLayer[eLayer]);
                m_aMenuLayer[eLayer].pOverlayFrame = nullptr;
            }

            if (m_iNumOfWindows > 0 && m_apWindowStack[m_iNumOfWindows - 1]->m_bPopOnBack)
                continue;
            break;
        }

        ChangeLayer(EMENU);
        m_iNumOfWindows = iTarget;

        if (bPushWindow)
        {
            if (bKeepRollbackWindow)
            {
                IWindowInterface* pPushWin = m_pMenuElements->GetWindow(pszPushWindowName);
                ZASSERT(pPushWin != nullptr);
                if (pPushWin->m_bPauseEngine)
                {
                    OpenWindow(pszWindowName, true, false);
                    OpenWindow(pszPushWindowName, true, false);
                    return;
                }
                ++m_iNumOfWindows;
            }
            OpenWindow(pszPushWindowName, true, false);
        }
        else
        {
            m_bRollBack = true;
            OpenWindow(pszWindowName, true, false);
            m_bRollBack = false;
        }
    }

    void ZXMLGUISystem::GetResources(ZGEOMREF& rResources)
    {
        // Do nothing
    }

    void ZXMLGUISystem::SetResources(const ZGEOMREF& rResources)
    {
        m_pWinGroupResources = ref_cast<ZWINGROUP>(rResources.GetRef());
    }

    void ZXMLGUISystem::GetFont(ZGEOMREF& rFont)
    {
        // Do nothing
    }

    void ZXMLGUISystem::SetFont(const ZGEOMREF& rFont)
    {
        m_pGroupFonts = ref_cast<ZGROUP>(rFont.GetRef());
    }

    ZFRAME* ZResourceManager::GetFrame(const ZVector2& vPos, ZColorSet*, ZWINGROUP* pParent,
        const ZVector2& vSize, const char* pszName, EAlignment)
    {
        ZGEOM* pGeom = m_pWinGroupFrames->FindGeom(pszName, nullptr);
        ZASSERT(!pGeom || pGeom->IsDerivedFrom<ZFRAME>());
        if (!pGeom)
            return nullptr;

        ZFRAME* pFrame = static_cast<ZFRAME*>(pGeom);
        pParent->AttachGeom(pFrame, true);
        pFrame->SetPos(vPos.x, vPos.y, 0.0f);
        pFrame->SetSize(static_cast<int>(vSize.x), static_cast<int>(vSize.y));
        pFrame->Hide(false);
        return pFrame;
    }

    ZWINGROUP* ZResourceManager::GetGraphic(const ZVector2& vPos, ZColorSet* pColorSet, ZWINGROUP* pParent,
        const char* pszName, EAlignment, int iPriority)
    {
        ZGEOM* pGeom = m_pWinGroupGraphic->FindGeom(pszName, nullptr);
        ZASSERT(!pGeom || pGeom->IsDerivedFrom<ZWINGROUP>());
        if (!pGeom)
            return nullptr;

        ZWINGROUP* pGroup = static_cast<ZWINGROUP*>(pGeom);
        pParent->AttachGeom(pGroup, true);
        pGroup->SetPos(vPos.x, vPos.y, 0.0f);

        for (ZBaseGeom* pBaseGeom = pGroup->BaseGeom(); pBaseGeom; pGroup->RecurGetNext(&pBaseGeom))
        {
            ZGEOM* pChildGeom = pBaseGeom->GetGeom();
            if (!pChildGeom || !pChildGeom->IsDerivedFrom<ZWINOBJ>())
                continue;

            ZWINOBJ* pWinObj = static_cast<ZWINOBJ*>(pChildGeom);
            if (pColorSet)
                SetColor(1, pWinObj, pColorSet);

            if (iPriority != -1)
            {
                ZASSERT(iPriority > 0 && iPriority < 16);
                pWinObj->SetPriority(static_cast<uint8_t>(iPriority));
            }
        }

        return pGroup;
    }

    void ZResourceManager::ReleaseGraphic(ZWINGROUP* pGroup)
    {
        if (!pGroup)
            return;

        m_pWinGroupGraphic->AttachGeom(pGroup, true);
    }

    void ZResourceManager::ReleaseFrame(ZFRAME* pFrame)
    {
        if (!pFrame)
            return;

        m_pWinGroupFrames->AttachGeom(pFrame, true);
    }

    void ZResourceManager::SetupResourceGroups(ZWINGROUP* pResources, ZGROUP* pFonts)
    {
        ZGEOM* pGeom = pResources->FindGeom("LineObjs", nullptr);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        m_pWinGroupLineObjs = static_cast<ZWINGROUP*>(pGeom);

        pGeom = pResources->FindGeom("Buttons", nullptr);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        m_pWinGroupButtons = static_cast<ZWINGROUP*>(pGeom);

        pGeom = pResources->FindGeom("Sliders", nullptr);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        m_pWinGroupSlider = static_cast<ZWINGROUP*>(pGeom);

        pGeom = pResources->FindGeom("Groups", nullptr);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        m_pWinGroupGroups = static_cast<ZWINGROUP*>(pGeom);

        pGeom = pResources->FindGeom("Graphic", nullptr);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        m_pWinGroupGraphic = static_cast<ZWINGROUP*>(pGeom);

        pGeom = pResources->FindGeom("ButtonGraphic", nullptr);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        m_pWinGroupButtonGraphic = static_cast<ZWINGROUP*>(pGeom);

        pGeom = pResources->FindGeom("Frames", nullptr);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        m_pWinGroupFrames = static_cast<ZWINGROUP*>(pGeom);

        static const char* s_apFontNames[3] = { "Header", "Menu", "Text" };
        for (int i = 0; i < 3; ++i)
        {
            ZGEOM* pFontGeom = pFonts->FindGeom(s_apFontNames[i], nullptr);
            ZASSERT(pFontGeom && pFontGeom->IsDerivedFrom<ZTTFONT>());
            m_pFonts[i] = static_cast<ZTTFONT*>(pFontGeom);
        }
    }

    ZWINGROUP* ZResourceManager::GetWingroup(ZWINGROUP* pParent)
    {
        if (m_pWinGroupGroups->m_NrAttachGeom == 0)
            return nullptr;

        ZGEOM* pGeom = m_pWinGroupGroups->m_pGroupFirst->GetGeom();
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINGROUP>());
        if (!pGeom)
            return nullptr;

        ZWINGROUP* pWinGroup = static_cast<ZWINGROUP*>(pGeom);
        pParent->AttachGeom(pWinGroup, true);
        pWinGroup->Hide(false);
        pWinGroup->SetPos(0.0f, 0.0f, 0.0f);
        return pWinGroup;
    }

    void ZResourceManager::ReleaseWinGroup(ZWINGROUP* pGroup)
    {
        if (!pGroup)
            return;

        m_pWinGroupGroups->AttachGeom(pGroup, true);
    }

    ZWINGROUP* ZResourceManager::GetBackgroundGroup()
    {
        return m_pWinGroupBackground;
    }

    void ZResourceManager::SetBackgroundGroup(ZWINGROUP* pGroup)
    {
        m_pWinGroupBackground = pGroup;
    }

    void ZResourceManager::SetColor(uint32_t iMask, ZWINOBJ* pWinObj, ZColorSet* pColorSet)
    {
        if (!pColorSet)
        {
            pWinObj->SetColor(0);
            pWinObj->SetAlpha(255);
            return;
        }

        switch (iMask)
        {
        case 1:
        case 2:
        case 128:
        case 129:
        {
            const uint32_t iColor = pColorSet->GetColor(ZColorSet::NormalColor);
            pWinObj->SetColor(iColor);
            pWinObj->SetAlpha(static_cast<uint8_t>(iColor >> 24));
            break;
        }
        case 8:
        case 10:
        {
            const uint32_t iColor = pColorSet->GetColor(ZColorSet::FocusColor);
            pWinObj->SetColor(iColor);
            pWinObj->SetAlpha(static_cast<uint8_t>(iColor >> 24));
            break;
        }
        case 32:
        {
            const uint32_t iColor = pColorSet->GetColor(ZColorSet::DisableColor);
            pWinObj->SetColor(iColor);
            pWinObj->SetAlpha(static_cast<uint8_t>(iColor >> 24));
            break;
        }
        default:
            break;
        }
    }

    void ZResourceManager::SetAlignment(ZWINOBJ* pWinObj, EAlignment eAlignment)
    {
        const uint8_t uFlags = static_cast<uint8_t>(pWinObj->GetAlignment() & 0xF0);

        switch (eAlignment)
        {
        case ERIGHT:
            pWinObj->SetAlignment(static_cast<uint8_t>(uFlags | 0x02));
            break;
        case ELEFT:
            pWinObj->SetAlignment(static_cast<uint8_t>(uFlags | 0x01));
            break;
        case ECENTER:
            pWinObj->SetAlignment(static_cast<uint8_t>(uFlags | 0x04));
            break;
        default:
            break;
        }
    }

    ZLINEOBJ* ZResourceManager::GetLineObj(const ZVector2& vPos, ZColorSet* pColorSet, uint32_t iMask,
        EFontType eFontType, EAlignment, bool bDisableAnimateAlpha)
    {
        if (m_pWinGroupLineObjs->m_NrAttachGeom == 0)
            return nullptr;

        ZGEOM* pGeom = m_pWinGroupLineObjs->m_pGroupFirst->GetGeom();
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZLINEOBJ>());
        if (!pGeom)
            return nullptr;

        ZLINEOBJ* pLineObj = static_cast<ZLINEOBJ*>(pGeom);
        pLineObj->RemoveGeometry();
        pLineObj->SetPos(vPos.x, vPos.y, 0.0f);
        pLineObj->SetFont(m_pFonts[eFontType]);
        pLineObj->m_bAnimateAlpha = false;
        pLineObj->SetWidth(400);
        pLineObj->SetLineSpacing(0);
        pLineObj->SetSpacingAdd(0);
        pLineObj->SetPriority(static_cast<uint8_t>(pColorSet ? 13 : 12));
        SetColor(iMask, pLineObj, pColorSet);

        if (iMask == 8)
            iMask = 10;

        pLineObj->SetType(static_cast<uint8_t>(iMask));

        if ((iMask & 8) != 0 && !bDisableAnimateAlpha)
            pLineObj->m_bAnimateAlpha = true;

        return pLineObj;
    }

    void ZResourceManager::AddLineObj(const ZVector2& vPos, ZColorSet* pColorSet, uint32_t iMask,
        ZWINGROUP* pGroup, EFontType eFontType, EAlignment eAlignment,
        ZStaticVector<ZLINEOBJ*, 8>* pLineObjs, bool bDisableAnimateAlpha, bool bShadow)
    {
        static const uint32_t s_aMasks[4] = { 1, 8, 32, 128 };

        for (int i = 0; i < 4; ++i)
        {
            if ((iMask & s_aMasks[i]) == 0)
                continue;

            ZLINEOBJ* pLineObj = GetLineObj(vPos, pColorSet, s_aMasks[i], eFontType, eAlignment, bDisableAnimateAlpha);
            if (!pLineObj)
                continue;

            pLineObj->Hide(false);
            pGroup->AttachGeom(pLineObj, true);
            SetAlignment(pLineObj, eAlignment);

            if (pLineObjs && pLineObjs->size() < pLineObjs->capacity())
                pLineObjs->push_back(pLineObj);
        }

        if (bShadow)
        {
            const ZVector2 vShadowPos { vPos.x + 1.0f, vPos.y + 1.0f };
            ZLINEOBJ* pLineObj = GetLineObj(vShadowPos, nullptr, 128, eFontType, eAlignment, bDisableAnimateAlpha);
            if (pLineObj)
            {
                pLineObj->Hide(false);
                pGroup->AttachGeom(pLineObj, true);
                SetAlignment(pLineObj, eAlignment);

                if (pLineObjs && pLineObjs->size() < pLineObjs->capacity())
                    pLineObjs->push_back(pLineObj);
            }
        }
    }

    ZWINGROUP* ZResourceManager::GetTextGroup(const ZVector2& vPos, ZColorSet* pColorSet, ZWINGROUP* pParent,
        uint32_t iMask, EFontType eFontType, bool bShadow, EAlignment eAlignment)
    {
        ZWINGROUP* pGroup = GetWingroup(pParent);
        if (!pGroup)
            return nullptr;

        pParent->AttachGeom(pGroup, true);
        pGroup->SetPos(vPos.x, vPos.y, 0.0f);

        ZVector2 vOffset { 0.0f, 0.0f };
        AddLineObj(vOffset, pColorSet, iMask, pGroup, eFontType, eAlignment, nullptr, false, false);

        if (bShadow)
        {
            vOffset.x += 1.0f;
            vOffset.y += 1.0f;
            AddLineObj(vOffset, nullptr, 128, pGroup, eFontType, eAlignment, nullptr, false, false);
        }

        return pGroup;
    }

    void ZResourceManager::AddAdditionalLineObjs(ZStaticVector<ZLINEOBJ*, 8>* pLineObjs, const ZVector2& vPos,
        ZColorSet* pColorSet, ZWINGROUP* pGroup, EAlignment eAlignment, uint32_t iMask,
        EFontType eFontType, bool bShadow)
    {
        AddLineObj(vPos, pColorSet, iMask, pGroup, eFontType, eAlignment, pLineObjs, false, false);

        if (bShadow)
        {
            const ZVector2 vShadowPos { vPos.x + 1.0f, vPos.y + 1.0f };
            AddLineObj(vShadowPos, nullptr, 128, pGroup, eFontType, eAlignment, pLineObjs, false, false);
        }

        if (pGroup->IsDerivedFrom<ZBUTTON>())
        {
            ZBUTTON* pButton = static_cast<ZBUTTON*>(pGroup);
            pButton->SetState(8);
            pButton->SetState(1);
        }
    }

    ZBUTTON* ZResourceManager::GetButton(const ZVector2& vPos, ZColorSet* pColorSet, ZWINGROUP* pParent, int iId,
        ZStaticVector<ZWINOBJ*, 8>* pChecked, ZStaticVector<ZWINOBJ*, 8>* pUnchecked,
        EAlignment eAlignment, ZButtonGraphic* pButtonGraphic, uint32_t iType,
        EFontType eFontType, bool bShadow, bool bDisableAnimateAlpha)
    {
        if (m_pWinGroupButtons->m_NrAttachGeom == 0)
            return nullptr;

        ZGEOM* pGeom = m_pWinGroupButtons->m_pGroupFirst->GetGeom();
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZBUTTON>());
        if (!pGeom)
            return nullptr;

        ZBUTTON* pButton = static_cast<ZBUTTON*>(pGeom);
        pButton->Hide(false);

        ZVector2 vOffset { 0.0f, 0.0f };
        bool bAddText = true;
        if (pButtonGraphic)
        {
            pButtonGraphic->GetTextOffSet(&vOffset);
            eAlignment = pButtonGraphic->GetTextAlignment();

            const float aButtonOffset[2] = { 0.0f, 0.0f };
            AddButtonGraphic(pButton, pButtonGraphic, pColorSet, ELEFT, pChecked, pUnchecked, aButtonOffset);

            bAddText = !pButtonGraphic->GraphcisOnly();
        }

        if (bAddText)
        {
            AddLineObj(vOffset, pColorSet, iType, pButton, eFontType, eAlignment, nullptr, bDisableAnimateAlpha, false);

            if (bShadow)
            {
                vOffset.x += 1.0f;
                vOffset.y += 1.0f;
                AddLineObj(vOffset, nullptr, 128, pButton, eFontType, eAlignment, nullptr, bDisableAnimateAlpha, false);
            }
        }

        pButton->Enable();
        pButton->SetControlId(iId);
        pButton->SetAvailibleStates(iType | 0x22);
        pButton->SetState(8);
        pButton->SetState(1);
        pParent->AttachGeom(pButton, true);
        pButton->SetOwner(pParent->GetRef());
        pButton->SetPos(vPos.x, vPos.y, 0.0f);
        pButton->SetNextFocus(nullptr, Up);
        pButton->SetNextFocus(nullptr, Down);
        pButton->SetNextFocus(nullptr, Left);
        pButton->SetNextFocus(nullptr, Right);

        (void)pChecked;
        (void)pUnchecked;

        return pButton;
    }

    void ZResourceManager::ReleaseButton(ZBUTTON* pButton)
    {
        if (!pButton)
            return;

        ReleaseLineObjects(pButton);
        ReleaseButtonGraphic(pButton);
        m_pWinGroupButtons->AttachGeom(pButton, true);
    }

    void ZResourceManager::ReleaseLineObjects(ZWINGROUP* pGroup)
    {
        ZLINEOBJ* apLineObjects[32];
        int iLineObjects = 0;

        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupLast; ForNotGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZLINEOBJ>())
            {
                ZASSERT(iLineObjects < 32);
                apLineObjects[iLineObjects++] = static_cast<ZLINEOBJ*>(pGeom);
            }
        }

        for (int i = 0; i < iLineObjects; ++i)
        {
            apLineObjects[i]->SetText("");
            apLineObjects[i]->RemoveGeometry();
            m_pWinGroupLineObjs->AttachGeom(apLineObjects[i], true);
        }
    }

    void ZResourceManager::ReleaseButtonGraphic(ZWINGROUP* pGroup)
    {
        ZWINPIC* apWinPics[8];
        int iWinPics = 0;
        ZFRAME* apFrames[8];
        int iFrames = 0;

        for (ZBaseGeom* pBaseGeom = pGroup->m_pGroupLast; ForNotGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom)
                continue;

            const char* pszName = pGeom->Name();
            if (!pszName)
                pszName = "<NONAME>";

            if (strcmp(pszName, "SliderBackground") == 0)
                continue;

            if (pGeom->IsDerivedFrom<ZWINPIC>())
            {
                ZASSERT(iWinPics < 8);
                apWinPics[iWinPics++] = static_cast<ZWINPIC*>(pGeom);
            }

            if (pGeom->IsDerivedFrom<ZFRAME>())
            {
                ZASSERT(iFrames < 8);
                apFrames[iFrames++] = static_cast<ZFRAME*>(pGeom);
            }
        }

        for (int i = 0; i < iWinPics; ++i)
            m_pWinGroupButtonGraphic->AttachGeom(apWinPics[i], true);

        for (int i = 0; i < iFrames; ++i)
            m_pWinGroupFrames->AttachGeom(apFrames[i], true);
    }

    ZWINOBJ* ZResourceManager::GetButtonGraphic(const zstring& rName, bool bFrame)
    {
        if (bFrame)
        {
            ZGEOM* pGeom = m_pWinGroupFrames->FindGeom(rName.c_str(), nullptr);
            ZASSERT(!pGeom || pGeom->IsDerivedFrom<ZFRAME>());
            if (!pGeom)
                return nullptr;

            return static_cast<ZWINOBJ*>(static_cast<ZFRAME*>(pGeom));
        }

        ZGEOM* pGeom = m_pWinGroupButtonGraphic->FindGeom(rName.c_str(), nullptr);
        ZASSERT(!pGeom || pGeom->IsDerivedFrom<ZWINPIC>());
        if (!pGeom)
            return nullptr;

        return static_cast<ZWINOBJ*>(static_cast<ZWINPIC*>(pGeom));
    }

    void ZResourceManager::AddButtonGraphic(ZWINGROUP* pParent, ZButtonGraphic* pButtonGraphic,
        ZColorSet* pColorSet, EAlignment eAlignment, ZStaticVector<ZWINOBJ*, 8>* pChecked,
        ZStaticVector<ZWINOBJ*, 8>* pUnchecked, const float* pOffset)
    {
        if (pChecked)
            pChecked->clear();

        if (pUnchecked)
            pUnchecked->clear();

        const int32_t iNumOfGraphicElements = pButtonGraphic->GetNumOfGraphicElements();
        for (int32_t i = 0; i < iNumOfGraphicElements; ++i)
        {
            ZButtonGraphicPart* pGraphicPart = pButtonGraphic->GetGraphicPart(i);
            if (!pGraphicPart)
                continue;

            const zstring sName = zstring(pGraphicPart->GetName()) + "*";

            ZWINOBJ* pWinObj = GetButtonGraphic(sName, pGraphicPart->m_bFrame);
            if (!pWinObj)
                continue;

            if (pGraphicPart->m_eCheckStatus == eCHECK_OFF && pChecked &&
                pChecked->size() < pChecked->capacity())
            {
                pChecked->push_back(pWinObj);
            }

            if (pGraphicPart->m_eCheckStatus == eCHECK_ON && pUnchecked &&
                pUnchecked->size() < pUnchecked->capacity())
            {
                pUnchecked->push_back(pWinObj);
            }

            pWinObj->Hide(false);
            pWinObj->SetPriority(static_cast<uint8_t>(pGraphicPart->m_iPriority));
            pWinObj->m_bAnimateAlpha = pGraphicPart->m_bAnimateAlpha;

            ZVector2 vPos{};
            pGraphicPart->GetPos(&vPos);
            vPos.x += pOffset[0];
            vPos.y += pOffset[1];

            pWinObj->SetPos(vPos.x, vPos.y, 30.0f - static_cast<float>(pGraphicPart->m_iPriority));
            pParent->AttachGeom(pWinObj, true);

            if (pColorSet)
                SetColor(pGraphicPart->m_iType, pWinObj, pColorSet);

            SetAlignment(pWinObj, eAlignment);
            pWinObj->SetType(static_cast<uint8_t>(pGraphicPart->m_iType));

            if (pGraphicPart->m_bFrame)
            {
                static_cast<ZFRAME*>(pWinObj)->SetSize(static_cast<int>(pGraphicPart->m_v2Size.x),
                    static_cast<int>(pGraphicPart->m_v2Size.y));
            }
        }
    }

    void ZResourceManager::AddButtonGraphic(ZBUTTON* pButton, ZButtonGraphic* pButtonGraphic,
        ZColorSet* pColorSet, EAlignment eAlignment, ZStaticVector<ZWINOBJ*, 8>* pChecked,
        ZStaticVector<ZWINOBJ*, 8>* pUnchecked)
    {
        const float aOffset[2] = { 0.0f, 0.0f };
        AddButtonGraphic(pButton, pButtonGraphic, pColorSet, eAlignment, pChecked, pUnchecked, aOffset);

        pButton->UpdateStateGraphics(pButton->GetState());
    }

    ZSlider* ZResourceManager::GetSlider(float* pfPos, ZColorSet* pColorSet, ZWINGROUP* pParent, int iIndex,
        int iLowerBound, int iUpperBound, int iSteps, ZButtonGraphic* pButtonGraphic, uint32_t iType,
        EFontType eFontType, bool bShadow, EAlignment eAlignment, float fSliderSize, float fSliderOffset)
    {
        if (m_pWinGroupSlider->m_NrAttachGeom == 0)
            return nullptr;

        ZGEOM* pGeom = m_pWinGroupSlider->m_pGroupFirst->GetGeom();
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZSlider>());
        if (!pGeom)
            return nullptr;

        ZSlider* pSlider = static_cast<ZSlider*>(pGeom);
        pSlider->SetSliderSize(fSliderSize);

        ZVector2 vTextPos{};
        EAlignment eTextAlignment = eAlignment;
        if (pButtonGraphic)
        {
            pButtonGraphic->GetTextOffSet(&vTextPos);
            eTextAlignment = pButtonGraphic->GetTextAlignment();
        }

        vTextPos.x -= fSliderOffset;

        AddLineObj(vTextPos, pColorSet, iType, pSlider, eFontType, eTextAlignment, nullptr, false, true);

        static const uint32_t s_aMasks[4] = { 1, 8, 32, 128 };

        ZVector2 vPos = vTextPos;
        vPos.x = fSliderSize + 10.0f;

        for (int i = 0; i < 4; ++i)
        {
            const uint32_t iMask = s_aMasks[i];
            if (iMask == 128)
            {
                if (!bShadow)
                    break;

                vPos.x += 1.0f;
                vPos.y += 1.0f;
            }

            ZLINEOBJ* pLineObj = GetLineObj(vPos, iMask == 128 ? nullptr : pColorSet, iMask,
                eFontType, ELEFT, false);
            if (!pLineObj)
                continue;

            SetAlignment(pLineObj, ELEFT);
            pSlider->AttachGeom(pLineObj, true);
            pSlider->AddExtraText(pLineObj);

            if (iMask == 8)
                pLineObj->m_bAnimateAlpha = true;
        }

        if (pButtonGraphic)
        {
            const float aOffset[2] = { -fSliderOffset, 0.0f };
            AddButtonGraphic(pSlider, pButtonGraphic, pColorSet, ELEFT, nullptr, nullptr, aOffset);
        }

        pSlider->Enable();
        pSlider->SetAvailibleStates(iType);
        pSlider->SetControlId(iIndex);
        pSlider->SetRange(iLowerBound, iUpperBound);
        pSlider->SetSteps(iSteps);
        pSlider->SetPos(pfPos[0] + fSliderOffset, pfPos[1], 0.0f);
        pSlider->SetState(8);
        pSlider->SetState(1);
        pParent->AttachGeom(pSlider, true);

        for (ZBaseGeom* pBaseGeom = pSlider->m_pGroupFirst; ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->Next())
        {
            ZGEOM* pChildGeom = pBaseGeom->GetGeom();
            if (pChildGeom && pChildGeom->IsDerivedFrom<ZBUTTON>())
                static_cast<ZBUTTON*>(pChildGeom)->SetControlId(iIndex);
        }

        return pSlider;
    }

    void ZResourceManager::ReleaseSlider(ZSlider* pSlider)
    {
        if (!pSlider)
            return;

        pSlider->ClearExtraText();
        ReleaseButtonGraphic(pSlider);
        ReleaseLineObjects(pSlider);
        m_pWinGroupSlider->AttachGeom(pSlider, true);
    }

    void ZResourceManager::ReleaseTextGroup(ZWINGROUP* pGroup)
    {
        if (!pGroup)
            return;

        ReleaseLineObjects(pGroup);
        m_pWinGroupGroups->AttachGeom(pGroup, true);
    }

    void ZResourceManager::CreateMenu3DViews(int iFirstViewId)
    {
        char szCameraName[] = "Menu3DCam0X";
        for (int i = 0; i < 4; ++i)
        {
            szCameraName[10] = static_cast<char>('1' + i);
            int iRef = 0;
            g_pEngineData->GetSceneCom()->GetVal(szCameraName, &iRef);
            ZGEOM* pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (!pGeom)
                continue;

            ZASSERT(pGeom->IsDerivedFrom<ZCAMERA>());
            ZCAMERA* pCamera = static_cast<ZCAMERA*>(pGeom);
            ZMenu3DCam& rMenuCam = m_aMenu3DCam[i];
            rMenuCam.m_pCamera = pCamera;
            rMenuCam.m_fDefaultCamPosX = pCamera->Pos().x;
            rMenuCam.m_pIView = g_pSysInterface->WindowFirst->CreateView(
                static_cast<unsigned int>(iFirstViewId + i), ZDrawSurface::SCREEN);
            rMenuCam.m_pIView->AddCamera(pCamera);

            ZGROUP* pParent = pCamera->BaseGeom()->ParentGroup();
            pParent->m_lGroupCon |= 0x400u;
            pCamera->SetCameraRoot(pParent->GetRef());
            pCamera->CameraCon = 0x298000;
            pCamera->CameraType = 0;
            pCamera->DeactivateCam();
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        RTP::ZVirtualProperty<ZGEOMREF> rFont
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "rFont",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__5,
            .m_Get = &ZXMLGUISystem::GetFont,
            .m_Set = &ZXMLGUISystem::SetFont
        };

        RTP::ZVirtualProperty<ZGEOMREF> rResources
        {
            .m_Node = {
                .m_Next = rFont,
                .m_Name = "rResources",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__5,
            .m_Get = &ZXMLGUISystem::GetResources,
            .m_Set = &ZXMLGUISystem::SetResources
        };
    }

    DEFINE_ROUT_CLASS(
        ZXMLGUISystem,
        ZWINDOW,
        ZXMLGUISystem,
        304,
        0,
        0x0080FE30,
        cProperties::rResources,
        ZWINDOW
    );
#   pragma endregion
}
