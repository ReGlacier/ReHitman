#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Com/CCOMType.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/GUI/Frame/ZFRAME.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZSNDOBJ.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZDllBase.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
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

    ZXMLGUISystem::~ZXMLGUISystem()
    {
        // TODO: Finish me
    }

    int ZXMLGUISystem::Command(Glacier::ZMSGID command, Glacier::ZDATA data)
    {
        // TODO: Finish me
        return 0;
    }

    bool ZXMLGUISystem::OnSliderChange(ZREF rSlider, uint32_t iValue)
    {
        // TODO: Finish me
        return false;
    }

    void ZXMLGUISystem::AddOtherWindowCount(int iAmount)
    {
        m_iNumOtherWindows += iAmount;
    }

    void ZXMLGUISystem::SetupCameras()
    {
        CCom* pSceneCom = g_pEngineData->GetSceneCom();

        bool b3dCamExists = false;
        pSceneCom->GetVal("3dBackgroundCamExists", &b3dCamExists);

        if (!b3dCamExists)
        {
            int iRef = 0;
            pSceneCom->GetVal("3dBackgroundCam", &iRef);
            ZGEOM* p3dCam = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (p3dCam)
            {
                // TODO: Finish me - render view creation for 3dBackgroundCam (view id 1)
                // g_pSysInterface->WindowFirst->CreateRenderView(1, 1) + SetCamera + SetGeom
            }
        }

        m_bGot3dBackground = !b3dCamExists;

        int iRef = 0;
        pSceneCom->GetVal("BackgroundCam", &iRef);
        ZCAMERA* pBgCam = static_cast<ZCAMERA*>(ZGEOM::RefToPtr(static_cast<ZREF>(iRef)));
        m_pBackgroundCam = pBgCam;

        if (pBgCam)
        {
            // TODO: Finish me - render view creation for BackgroundCam (view id 2)
            // g_pSysInterface->WindowFirst->CreateRenderView(2, 1) + SetCamera + SetGeom

            ZASSERT(pBgCam->IsDerivedFrom<ZCAMERA>());

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
            // TODO: Finish me - render view creation for MenuCam (view id 3)
            ZASSERT(pMenuCam->IsDerivedFrom<ZCAMERA>());
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
            // TODO: Finish me - render view creation for DialogCam (view id 177)
            ZASSERT(pDialogCam->IsDerivedFrom<ZCAMERA>());
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
            // TODO: Finish me - render view creation for TRCCam (view id 178)
            ZASSERT(pTRCCam->IsDerivedFrom<ZCAMERA>());
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
            // TODO: Finish me - render view creation for MouseCam (view id 179)
            ZASSERT(pMouseCam->IsDerivedFrom<ZCAMERA>());
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
        // TODO: Finish me
        return nullptr;
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

        // TODO: Finish me - ZResourceManager::GetFrame("MenuOverlay*", ...) call
        // Creates overlay frame for the layer when switching to a deeper layer
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
                // TODO: Finish me - ZDllBase::PushScene vtable call (pause/mute scene transition)
            }
        }
        else if (m_bUnpauseGame)
        {
            g_pEngineData->m_bPause = false;
            // TODO: Finish me - ZDllBase::PushScene vtable call (unpause scene transition)
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
                        // TODO: Finish me - ZDllBase::PushScene vtable call (pause)
                        StopAndSetMusicFlags();
                    }
                }
                else if (m_bUnpauseGame)
                {
                    g_pEngineData->m_bPause = false;
                    // TODO: Finish me - ZDllBase::PushScene vtable call (unpause)
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

        if (m_bCloseSystem && m_iNumOfWindows == 0)
        {
            m_ResourceManager.ReleaseTextGroup(m_pWinGroupResources);
            m_pWinGroupResources = nullptr;
            // TODO: Finish me - ReleaseOverlayFrame for all 3 layers + DeactivateFrameUpdate

            if (m_bUnpauseGame)
            {
                g_pEngineData->m_bPause = false;
                // TODO: Finish me - ZDllBase::PushScene vtable call (unpause)
                if (m_bUnpauseGame)
                    g_pGameData->GetAudioEffectsInterface().PlaySound(10);
            }

            CCom* pSceneCom = g_pEngineData->GetSceneCom();
            int iRef = 0;
            pSceneCom->GetVal("TraningInfoAmbient", &iRef);
            ZGEOM* pGeom = ZGEOM::RefToPtr(static_cast<ZREF>(iRef));
            if (pGeom)
                static_cast<ZSNDOBJ*>(pGeom)->GetSoundObject()->SetActive(false);

            if (m_pMenuCam)
                m_pMenuCam->MakeActive();
            if (m_pDialogCam)
                m_pDialogCam->MakeActive();
            if (m_pTRCCam)
                m_pTRCCam->MakeActive();

            Action::instance->DisableInputTimer(0x0C000032);
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
