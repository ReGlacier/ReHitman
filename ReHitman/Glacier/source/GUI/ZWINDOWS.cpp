#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZCAMERA.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/Control/ZCONTROL.h>
#include <Glacier/GUI/ZSlider.h>
#include <Glacier/GUI/ZWINDOW.h>
#include <Glacier/GUI/ZWinGfxHandler.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/CConfiguration.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>

#include <cmath>
#include <algorithm>


namespace Glacier
{
    STATIC_GLOBAL_VAR(int, g_lShow2D, 0x007FD8E8, -1);

    ZWINDOWS::ZWINDOWS(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINGROUP(psName, pBaseGeom)
        , m_fBgScale(1.0f)
        , m_Subscribers(32, 4)
        , m_WindowStack(8, 0)
        , m_rtFreeSpriteArrays()
        , m_Cameras(4, 0)
        , m_pGfxHandler(nullptr)
        , m_iInputQuePos(0)
        , m_bLeftMouseDown(false)
        , m_bUpdateMouseFocus(false)
        , m_pExternalMouseColiGroup(nullptr)
        , m_bShow2d(true)
        , m_bFadeEnabled(true)
        , m_fFade(0.0f)
        , m_winInput()
        , m_iAlphaBackupCount(0)
        , m_aAlphaBackup{}
    {
        m_winInput.SetWindowsPtr(this);
        g_pEngineData->GetSceneCom()->SetVal("rWindows", GetRef(), CCOM_TYPE_REF);
    }

    ZWINDOWS::~ZWINDOWS()
    {
        g_pEngineData->GetSceneCom()->SetVal("rWindows", 0u, CCOM_TYPE_REF);
    }

    const RTP::ZPropertyInfo& ZWINDOWS::GetProperties() const
    {
        return ZWINDOWS::Info;
    }

    uint32_t ZWINDOWS::GetObjectId() const
    {
        return ZWINDOWS::m_Id;
    }

    void ZWINDOWS::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWINDOWS::m_Id;
        mask = ZWINDOWS::m_Mask;
    }

    ZGEOMCLASSINFO* ZWINDOWS::GetOldClassInfo() const
    {
        return ZWINDOWS::m_OldClassInfo;
    }

    void ZWINDOWS::LoadInit()
    {
        BaseGeom()->SetControl(0x400, 0);
        SetUseZBuffer(m_bUseZBuffer);
        m_bLeftMouseDownLastFrame = false;
        m_v2glMouse = {};
        m_v2MouseDelta = {};
        m_v2Pos = {};
        m_rLastHit = 0;
        m_rLastPushed = 0;
        m_pControlInFocus = nullptr;
        m_rHitGeom = 0;
    }

    void ZWINDOWS::ClassInit()
    {
        LoadInit();
        ZWINGROUP::ClassInit();
        EnableClassCall(0x110);

        ZASSERT(m_pMainCamera);
        ZASSERT(m_pMainCamera->IsDerivedFrom<ZCAMERA>());
        m_pMainCamera->SetCameraRoot(GetRef());
        m_pMainCamera->BaseGeom()->SetControl(0x210000, 0);

        g_pSysInterface->GetConfiguration();
        SetMouseSpeed(CConfiguration::m_fMouseSpeed);
        SetShowMouse(CConfiguration::m_bUseMouse);
    }

    ZGEOM* ZWINDOWS::Duplicate(ZGROUP* pDestGroup, const char* psName, bool bRecursive)
    {
        return ZGROUP::Duplicate(pDestGroup, psName, bRecursive);
    }

    void ZWINDOWS::CopyData(const ZGEOM* pSource)
    {
        ZWINGROUP::CopyData(pSource);
    }

    bool ZWINDOWS::Notify(ZWM_MESSAGE eMessage, int evData1, int evData2, ZWINGROUP* pGroup)
    {
        ZWMEVENT event{0, eMessage, evData1, evData2, false};
        const bool bHandled = NotifySubscribers(&event);
        return bHandled || (pGroup && eMessage < ZWN_FIRSTNOTIFY && Notify(&event, pGroup));
    }

    bool ZWINDOWS::Notify(ZWMEVENT* pEvent, ZWINGROUP* pGroup)
    {
        if (!pGroup)
            return false;

        pEvent->Return = false;
        pEvent->Target = pGroup->GetRef();
        while (!pEvent->Return && pGroup)
        {
            pGroup->ClassCommand(0x8000, pEvent);
            ZGROUP* pParent = pGroup->Parent();
            pGroup = pParent && pParent->IsDerivedFrom<ZWINGROUP>()
                ? static_cast<ZWINGROUP*>(pParent) : nullptr;
        }
        return pEvent->Return;
    }

    bool ZWINDOWS::NotifySubscribers(ZWMEVENT* pEvent)
    {
        ZWMSUBSCRIPTION subscriptions[64];
        int count = 0;
        RefRun run;
        m_Subscribers.RunInitNxtRef(&run);
        for (uint32_t* pRef = m_Subscribers.RunNxtRefPtr(&run); pRef; pRef = m_Subscribers.RunNxtRefPtr(&run))
        {
            auto* pSubscription = reinterpret_cast<ZWMSUBSCRIPTION*>(pRef);
            if ((pSubscription->dwMessages & pEvent->Message) != 0 &&
                (!pSubscription->bCompareParam || pSubscription->Param1._int == pEvent->Param1))
            {
                ZASSERT(count < 64);
                subscriptions[count++] = *pSubscription;
            }
        }

        for (int i = 0; i < count; ++i)
        {
            ZGEOM* pSubscriber = ZGEOM::RefToPtr(subscriptions[i].rSubscriber);
            if (pSubscriber && pSubscriber->IsDerivedFrom<ZWINGROUP>() &&
                Notify(pEvent, static_cast<ZWINGROUP*>(pSubscriber)))
                return true;
        }
        return false;
    }

    void ZWINDOWS::CheckCommands()
    {
        if (g_lShow2D == -1)
            return;

        if (g_lShow2D)
        {
            m_bShow2d = true;
            if (m_pMainCamera)
                m_pMainCamera->ActivateCam();
        }
        else
        {
            if (m_pMainCamera)
                m_pMainCamera->DeactivateCam();
            m_bShow2d = false;
        }
        g_lShow2D = -1;
    }

    void ZWINDOWS::SetFocusedControl(ZWINGROUP* pGroup)
    {
        if (m_pControlInFocus == pGroup)
            return;

        const ZREF rOld = m_pControlInFocus ? m_pControlInFocus->GetRef() : 0;
        const ZREF rNew = pGroup ? pGroup->GetRef() : 0;
        m_fFade = 1.0f;
        RestoreAlpha(m_pControlInFocus);
        Notify(ZWM_FOCUSLOST, static_cast<int>(rNew), 0, m_pControlInFocus);
        Notify(ZWM_FOCUSRECEIVED, static_cast<int>(rOld), 0, pGroup);
        m_pControlInFocus = pGroup;
        BackupAlpha(pGroup);
        Notify(ZWN_FOCUSCHANGED, static_cast<int>(rOld), static_cast<int>(rNew), nullptr);
    }

    ZWINGROUP* ZWINDOWS::GetFocusedControl()
    {
        return m_pControlInFocus;
    }

    void ZWINDOWS::ReleaseFocusedControl(ZWINGROUP* pGroup, bool bFocusParent)
    {
        if (m_pControlInFocus != pGroup)
            return;

        if (bFocusParent && pGroup && pGroup->IsDerivedFrom<ZCONTROL>())
        {
            if (ZCONTROL* pNext = static_cast<ZCONTROL*>(pGroup)->GetNextFocus(Whatever))
            {
                SetFocusedControl(pNext);
                return;
            }
        }
        SetFocusedControl(nullptr);
    }

    void ZWINDOWS::PushWindow(uint32_t id)
    {
        ZGEOM* pGeom = ZGEOM::RefToPtr(id);
        ZASSERT(pGeom && pGeom->IsDerivedFrom<ZWINDOW>());
        PushWindow(static_cast<ZWINDOW*>(pGeom));
    }

    void ZWINDOWS::PushWindow(ZWINDOW* pWindow)
    {
        ZWINDOW* pPrevious = nullptr;
        RefRun run;
        m_WindowStack.RunInitPrevRef(&run);
        for (uint32_t* pRef = m_WindowStack.RunPrevRefPtr(&run); pRef; pRef = m_WindowStack.RunPrevRefPtr(&run))
        {
            ZGEOM* pGeom = ZGEOM::RefToPtr(*pRef);
            if (pGeom)
            {
                pPrevious = static_cast<ZWINDOW*>(pGeom);
                break;
            }
            m_WindowStack.RunDelRef(&run);
        }
        if (pPrevious)
            Notify(ZWM_WINDOWCLOSE, static_cast<int>(pWindow->GetRef()), 0, pPrevious);
        m_WindowStack.Add(pWindow->GetRef());
        Notify(ZWM_WINDOWOPEN, static_cast<int>(pPrevious ? pPrevious->GetRef() : 0u), 1, pWindow);
        m_bUpdateMouseFocus = true;
    }

    void ZWINDOWS::PopWindow()
    {
        ZWINDOW* pTop = nullptr;
        ZWINDOW* pNext = nullptr;
        RefRun run;
        m_WindowStack.RunInitPrevRef(&run);
        for (uint32_t* pRef = m_WindowStack.RunPrevRefPtr(&run); pRef; pRef = m_WindowStack.RunPrevRefPtr(&run))
        {
            ZGEOM* pGeom = ZGEOM::RefToPtr(*pRef);
            if (!pGeom)
            {
                m_WindowStack.RunDelRef(&run);
                continue;
            }
            if (!pTop)
            {
                pTop = static_cast<ZWINDOW*>(pGeom);
                m_WindowStack.RunDelRef(&run);
            }
            else
            {
                pNext = static_cast<ZWINDOW*>(pGeom);
                break;
            }
        }
        if (pTop)
            Notify(ZWM_WINDOWCLOSE, static_cast<int>(pNext ? pNext->GetRef() : 0u), 1, pTop);
        if (pNext)
            Notify(ZWM_WINDOWOPEN, static_cast<int>(pTop ? pTop->GetRef() : 0u), 0, pNext);
        m_bUpdateMouseFocus = true;
    }

    ZREF ZWINDOWS::SetMousePos(const ZVector2& vPos)
    {
        m_v2MousePos.x = std::clamp(vPos.x, 0.0f, static_cast<float>(g_pSysInterface->m_lResolution[0]));
        m_v2MousePos.y = std::clamp(vPos.y, 0.0f, static_cast<float>(g_pSysInterface->m_lResolution[1]));
        m_v2Pos = m_v2MousePos;

        if (ZGEOM* pMouse = ZGEOM::RefToPtr(m_rMouse))
        {
            ZVector3 pos{std::floor(m_v2MousePos.x + 0.5f), std::floor(m_v2MousePos.y + 0.5f), 1.0f};
            pMouse->SetPos(pos);
        }

        SMouseColi coli{};
        coli.vColi.z = 9.9999997e37f;
        ZMat3x3 identity;
        mreset(identity.data);
        for (ZREF rCamera : m_Cameras)
        {
            if (ZCAMERA* pCamera = geom_cast<ZCAMERA>(ZGEOM::RefToPtr(rCamera)))
            {
                coli.pCamera = pCamera;
                GetMouseColi(coli, ZVector3{}, identity);
                if (coli.rGeom)
                    return coli.rGeom;
            }
        }
        return 0;
    }

    void ZWINDOWS::GetMousePos(ZVector2& vPos)
    {
        vPos = m_v2MousePos;
    }

    void ZWINDOWS::SetShowMouse(bool bShowMouse)
    {
        m_bMouseActive = g_pSysInterface->m_bUseGameController ? false : bShowMouse;
        if (ZGEOM* pMouse = ZGEOM::RefToPtr(m_rMouse))
            pMouse->Hide(!m_bMouseActive);
    }

    ZREF ZWINDOWS::UpdateMouse()
    {
        m_v2MouseDelta.x *= m_fMouseSpeed;
        m_v2MouseDelta.y *= m_fMouseSpeed;
        ZVector2 pos{m_v2MousePos.x + m_v2MouseDelta.x, m_v2MousePos.y + m_v2MouseDelta.y};
        const bool bMoved = m_v2MouseDelta.x != 0.0f || m_v2MouseDelta.y != 0.0f || m_bUpdateMouseFocus;
        m_v2MouseDelta = {};
        m_bUpdateMouseFocus = false;
        return bMoved ? SetMousePos(pos) : m_rHitGeom;
    }

    void ZWINDOWS::SetMouseSpeed(float fSpeed)
    {
        m_fMouseSpeed = fSpeed;
    }

    void ZWINDOWS::SetMousePosition(const ZVector2& vDt)
    {
        SetMousePos(vDt);
    }

    void ZWINDOWS::SetMouseDeltaPos(const ZVector2& vDt)
    {
        m_v2MouseDelta = vDt;
    }

    ZREF ZWINDOWS::GetMouseGeom()
    {
        return m_rMouse;
    }

    void ZWINDOWS::ForceUpdateMouse()
    {
        m_bUpdateMouseFocus = true;
    }

    void ZWINDOWS::AddCamera(ZCAMERA* pCam)
    {
        const ZREF ref = pCam->GetRef();
        if (!m_Cameras.Exists(ref))
            m_Cameras.AddSort(ref, -static_cast<float>(pCam->CameraListPri), 0);
    }

    void ZWINDOWS::RemoveCamera(ZCAMERA* pCam)
    {
        m_Cameras.RemoveIfExists(pCam->GetRef());
    }

    void ZWINDOWS::SetUseZBuffer(bool bValue)
    {
        if (!m_pMainCamera)
            return;
        m_pMainCamera->CameraType = m_bClearScreen ? 0 : 5;
        if (bValue && !m_bClearScreen)
            m_pMainCamera->CameraCon |= 0x8000;
        else
            m_pMainCamera->CameraCon &= ~0x8000;
    }

    bool ZWINDOWS::SetClearScreen(bool bValue)
    {
        const bool bPrevious = m_bClearScreen;
        m_bClearScreen = bValue;
        SetUseZBuffer(m_bUseZBuffer);
        return bPrevious;
    }

    void ZWINDOWS::SetBackColor(int lColor)
    {
        if (m_pMainCamera)
            m_pMainCamera->SetBackColor(lColor);
    }

    void ZWINDOWS::SetBackColor(int r, int g, int b)
    {
        SetBackColor(b | ((g | (r << 8)) << 8));
    }

    void ZWINDOWS::SetGfxHandler(ZWinGfxHandler* pGfx)
    {
        if (m_pGfxHandler)
            m_pGfxHandler->Delete();
        m_pGfxHandler = pGfx;
    }

    ZWinGfxHandler* ZWINDOWS::GetGfxHandler()
    {
        return m_pGfxHandler;
    }

    void ZWINDOWS::AddSubscriber(uint32_t id, int eventId, ZWINGROUP* pGroup, float fPriority, bool flag0C)
    {
        RemoveSubscriber(id, eventId, pGroup);
        auto* pSubscription = reinterpret_cast<ZWMSUBSCRIPTION*>(m_Subscribers.AddSort(pGroup->GetRef(), fPriority, 0));
        pSubscription->rSubscriber = pGroup->GetRef();
        pSubscription->dwMessages = id;
        pSubscription->Param1._int = eventId;
        pSubscription->bCompareParam = flag0C;
    }

    void ZWINDOWS::RemoveSubscriber(uint32_t id, int eventId, ZWINGROUP* pGroup)
    {
        const ZREF ref = pGroup->GetRef();
        RefRun run;
        m_Subscribers.RunInitNxtRef(&run);
        for (uint32_t* pRef = m_Subscribers.RunNxtRefPtr(&run); pRef; pRef = m_Subscribers.RunNxtRefPtr(&run))
        {
            auto* pSubscription = reinterpret_cast<ZWMSUBSCRIPTION*>(pRef);
            if (pSubscription->rSubscriber == ref &&
                (!pSubscription->bCompareParam || pSubscription->Param1._int == eventId))
            {
                const uint32_t oldMessages = pSubscription->dwMessages;
                pSubscription->dwMessages &= ~id;
                if ((oldMessages & ~id) == 0)
                    m_Subscribers.RunDelRef(&run);
            }
        }
    }

    int ZWINDOWS::GetActivateBy(int iKey)
    {
        return m_winInput.GetActivatedBy(iKey);
    }

    void ZWINDOWS::InvalidateView()
    {
        for (ZBaseGeom* pBaseGeom = BaseGeom(); pBaseGeom; RecurGetNext(&pBaseGeom))
        {
            if (ZWINOBJ* pWinObj = geom_cast<ZWINOBJ>(pBaseGeom->GetGeom()))
                pWinObj->SetModified(true);
        }
    }

    void ZWINDOWS::FadeGroup(uint8_t lAlpha, ZWINGROUP* pGroup)
    {
        if (!pGroup || (!pGroup->IsDerivedFrom<ZBUTTON>() && !pGroup->IsDerivedFrom<ZSlider>()))
            return;
        for (ZBaseGeom* pBaseGeom = pGroup->BaseGeom(); pBaseGeom; pGroup->RecurGetNext(&pBaseGeom))
        {
            if (ZWINOBJ* pWinObj = geom_cast<ZWINOBJ>(pBaseGeom->GetGeom()); pWinObj && pWinObj->m_bAnimateAlpha)
                pWinObj->SetAlpha(lAlpha);
        }
    }

    void ZWINDOWS::EnableFade(bool bEnable)
    {
        if (m_bFadeEnabled == bEnable)
            return;
        m_bFadeEnabled = bEnable;
        if (bEnable)
            m_fFade = 1.0f;
        else
            FadeGroup(0xFE, m_pControlInFocus);
    }

    void ZWINDOWS::BackupAlpha(ZWINGROUP* pGroup)
    {
        if (!pGroup)
            return;
        m_iAlphaBackupCount = 0;
        for (ZBaseGeom* pBaseGeom = pGroup->BaseGeom(); pBaseGeom && m_iAlphaBackupCount < 8; pGroup->RecurGetNext(&pBaseGeom))
        {
            if (ZWINOBJ* pWinObj = geom_cast<ZWINOBJ>(pBaseGeom->GetGeom()); pWinObj && pWinObj->m_bAnimateAlpha)
                m_aAlphaBackup[m_iAlphaBackupCount++] = pWinObj->m_dwAmount;
        }
    }

    void ZWINDOWS::RestoreAlpha(ZWINGROUP* pGroup)
    {
        if (!pGroup)
            return;
        uint32_t index = 0;
        for (ZBaseGeom* pBaseGeom = pGroup->BaseGeom(); pBaseGeom && index < m_iAlphaBackupCount; pGroup->RecurGetNext(&pBaseGeom))
        {
            if (ZWINOBJ* pWinObj = geom_cast<ZWINOBJ>(pBaseGeom->GetGeom()); pWinObj && pWinObj->m_bAnimateAlpha)
                pWinObj->SetAlpha(m_aAlphaBackup[index++]);
        }
    }

    void ZWINDOWS::GetMainCamera(ZGEOMREF&)
    {
        ZASSERT(false);
    }

    void ZWINDOWS::SetMainCamera(const ZGEOMREF& rCamera)
    {
        m_pMainCamera = geom_cast<ZCAMERA>(ZGEOM::RefToPtr(rCamera.m_Value));
        ZASSERT(m_pMainCamera);
        AddCamera(m_pMainCamera);
    }

    void ZWINDOWS::GetGfxHandlerRef(ZGEOMREF&)
    {
        ZASSERT(false);
    }

    void ZWINDOWS::SetGfxHandlerRef(const ZGEOMREF& rHandler)
    {
        m_pGfxHandler = geom_cast<ZWinGfxHandler>(ZGEOM::RefToPtr(rHandler.m_Value));
    }

    void ZWINDOWS::ClassFrameUpdate()
    {
        CheckCommands();
        m_winInput.Update();
        for (int i = 0; i < m_iInputQuePos; ++i)
        {
            const int input = m_aiInputQue[i];
            Notify((input & 0x2000000) ? ZWM_KEYDOWN : ZWM_KEYUP,
                input & 0xFFFF, 0, m_pControlInFocus);
        }
        m_iInputQuePos = 0;

        if (m_bMouseActive)
        {
            ZGEOM* pLastHit = ZGEOM::RefToPtr(m_rLastHit);
            m_rHitGeom = UpdateMouse();
            ZGEOM* pHit = ZGEOM::RefToPtr(m_rHitGeom);
            ZWINGROUP* pControl = nullptr;
            for (ZGROUP* pParent = pHit ? pHit->Parent() : nullptr; pParent; pParent = pParent->Parent())
            {
                if (pParent->IsDerivedFrom<ZCONTROL>())
                {
                    pControl = static_cast<ZWINGROUP*>(pParent);
                    break;
                }
            }

            if (pHit)
                Notify(ZWM_MOUSEMOVE, static_cast<int>(m_v2MousePos.x),
                    static_cast<int>(m_v2MousePos.y), pHit->IsDerivedFrom<ZWINGROUP>() ? static_cast<ZWINGROUP*>(pHit) : pControl);

            if (m_bLeftMouseDown && !m_bLeftMouseDownLastFrame && pControl)
            {
                SetFocusedControl(pControl);
                m_rLastPushed = pControl->GetRef();
            }
            else if (!m_bLeftMouseDown && m_bLeftMouseDownLastFrame)
            {
                ZGEOM* pPushed = ZGEOM::RefToPtr(m_rLastPushed);
                if (pPushed && pPushed == pControl)
                    Notify(ZWM_CLICK, static_cast<int>(m_rHitGeom), 0, pControl);
                m_rLastPushed = 0;
            }

            if (pLastHit != pHit)
                m_rLastHit = pHit ? pHit->GetRef() : 0;
            m_bLeftMouseDownLastFrame = m_bLeftMouseDown;
        }

        if (m_bFadeEnabled)
        {
            m_fFade += (g_pSysInterface->m_fActualTimeDelta * 2.0f) / g_pSysInterface->GetTimeMultiplier();
            m_fFade = std::fmod(m_fFade, 2.0f);
            FadeGroup(static_cast<uint8_t>(std::fabs(m_fFade - 1.0f) * 200.0f + 50.0f), m_pControlInFocus);
        }
    }


#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<ZGEOMREF> GfxHandler
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "GfxHandler", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Virtual_ZGEOMREF,
            .m_Get = &ZWINDOWS::GetGfxHandlerRef,
            .m_Set = &ZWINDOWS::SetGfxHandlerRef
        };
        static RTP::ZVirtualProperty<ZGEOMREF> MainCamera
        {
            .m_Node = { .m_Next = GfxHandler, .m_Name = "MainCamera", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Virtual_ZGEOMREF,
            .m_Get = &ZWINDOWS::GetMainCamera,
            .m_Set = &ZWINDOWS::SetMainCamera
        };
        static RTP::ZDataProperty<bool> ClearScreen
        {
            .m_Node = { .m_Next = MainCamera, .m_Name = "m_bClearScreen", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZWINDOWS, m_bClearScreen)
        };
        static RTP::ZDataProperty<bool> UseZBuffer
        {
            .m_Node = { .m_Next = ClearScreen, .m_Name = "m_bUseZBuffer", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZWINDOWS, m_bUseZBuffer)
        };
        static RTP::ZDataProperty<ZGEOMREF> Mouse
        {
            .m_Node = { .m_Next = UseZBuffer, .m_Name = "m_rMouse", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZWINDOWS, m_rMouse))
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZWINDOWS,
        ZWINGROUP,
        0x009A2A30,
        "ZWINDOWS",
        0x0077D768,
        cProperties::Mouse,
        0x00813F44,
        0x009A29E0,
        0x009A29E4
    );
#   pragma endregion
}
