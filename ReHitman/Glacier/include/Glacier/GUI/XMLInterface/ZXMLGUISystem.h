#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/GUI/CWinEvent.h>
#include <Glacier/GUI/ZWINDOW.h>
#include <Glacier/RTP/PropertyTypes.h>


namespace Glacier
{
    struct SOpenWindowParams
    {
        // Size: 0x14 (20) bytes
        bool bOpenWindow;
        Glacier::zstring sWindow;
        bool bPushOnStack;
        bool bResetStack;
    };

    struct SCloseWindowParams
    {
        // Size: 0x2 (2) bytes
        bool bCloseWindow;
        bool bCloseAll;
    };

    struct SRollBackParams
    {
        // Size: 0x20 (32) bytes
        bool bRollBack;
        zstring sWindow;
        zstring sPushWindow;
        bool bKeepRollbackWindow;
    };


    class ZXMLGUISystem : public CWinEvent<ZWINDOW>
    {
    public:
        // RTTI
        DECLARE_ROUT_CLASS(ZXMLGUISystem, ZWINDOW, ZXMLGUISystem, 304, 0);

        // types
        enum EMenuLayer
        {
            EMENU = 0,
            EDIALOG = 1,
            ETRC = 2,
            NUM_OF_LAYERS = 3,
        };

        // vtbl
        ~ZXMLGUISystem() override;

        // ZEventBase
        int Command(Glacier::ZMSGID command, Glacier::ZDATA data) override;

        // CWinEvent<ZWINDOW>
        bool OnSliderChange(ZREF rSlider, uint32_t iValue) override;

        // ZXMLGUISystem
        virtual void SetFocus();
        virtual void AddOtherWindowCount(int iAmount);

        // methods
        ZXMLGUISystem();
        void SetupCameras();
        void StopAndSetMusicFlags();
        IWindowInterface* GetTopWindow();
        void ChangeLayer(EMenuLayer eLayer);
        void Cancel();
        bool WindowPush();
        bool IsOnStack(const char* pszWindow) const;
        bool IsTRCOpen() const;
        int32_t GetOpenWindowsCount() const;
        IGUIElement* GetElementInFocus();
        ZResourceManager* GetResourceManager();
        EMenuLayer GetWindowLayer(IWindowInterface* pWnd) const;
        void AddTRCWindow(IWindowInterface* pWnd);
        IWindowInterface* GetNextTRCWindow();
        void ReleaseOverlayFrame(SMenuLayer& rLayer);
        void AddOverlayFrame(SMenuLayer& rLayer);
        void AddRecieveAllInput(IGUIElement* pElement);
        void RemoveRecieveAllInput(IGUIElement* pElement);
        void SendEventToRecieveAll(uint32_t iKey, IGUIElement* pElement);
        ZWINDOWS* GetTopUIComponent();
        void OpenWindow(const char* pszWindowName, bool bPushOnStack, bool bResetStack);
        void OpenWindow(IWindowInterface* pWindowInterface, bool bPushWindow, ZWINGROUP* pParent);
        void CloseWindow(bool bCloseAll);
        void Set2DBackgroundCamClear(bool bCamClear);
        bool Get2DBackgroundCamClear() const;
        void HandleCachedFunctions();
        void RollBack(const char* pszWindowName, const char* pszPushWindowName, bool bPushWindow, bool bKeepRollbackWindow);

        void GetResources(ZGEOMREF& rResources);
        void SetResources(const ZGEOMREF& rResources);
        void GetFont(ZGEOMREF& rFont);
        void SetFont(const ZGEOMREF& rFont);

        // data
        bool m_bUnpauseGame; // +0x30
        ZWINGROUP* m_pWinGroupResources; // +0x34
        ZGROUP* m_pGroupFonts; // +0x38
        ZResourceManager m_ResourceManager; // +0x3c
        int32_t m_iLastFocusedElement; // +0x318
        int32_t m_iNumOfWindows; // +0x31c - Verified by ZXMLGUISystem::IsOnStack
        int32_t m_iNumOtherWindows; // +0x320
        IWindowInterface* m_apWindowStack[32]; // +0x324
        int32_t m_aLastIndex[32]; // +0x3a4
        ZWINGROUP* m_pWinGroupHeader; // +0x424
        ZMenuElements* m_pMenuElements; // +0x428
        int32_t m_iPlayerActivatedMenu; // +0x42c
        bool m_bInsideUpdate; // +0x430
        bool m_bCloseSystem; // +0x431 - Verified by ZXMLGUISystem::ZXMLGUISystem (init to true)
        bool m_bGot3dBackground; // +0x432
        bool m_bAddBackgroundCamAnd3DCams; // +0x433 - Verified by ZXMLGUISystem::Command
        ZCAMERA* m_pBackgroundCam; // +0x434
        ZCAMERA* m_pMenuCam; // +0x438
        ZCAMERA* m_pDialogCam; // +0x43c
        ZCAMERA* m_pTRCCam; // +0x440
        SOpenWindowParams m_OpenWindowParams; // +0x444
        SCloseWindowParams m_CloseWindowParams; // +0x458
        SRollBackParams m_RollBackParams; // +0x45c
        SMenuLayer m_aMenuLayer[3]; // +0x47c - Verified by constructor vector init at +0x480
        RE_ADD_PADDING(4); // +0x560..0x563
        int32_t m_eCurrentLayer; // +0x564 - Verified by ChangeLayer (DWORD index 0x159)
        bool m_bWindowPush; // +0x568
        bool m_bRollBack; // +0x569
        RE_ADD_PADDING(2); // +0x56A..0x56B
        IWindowInterface* m_apTRCWindows[4]; // +0x56C
        int32_t m_iNumOfTRCWindows; // +0x57C
    };
    RE_VERIFY_SIZE(ZXMLGUISystem, 0x580); // Verified PC alloc
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_bUnpauseGame, 0x30);
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_ResourceManager, 0x3C); // Verified by ZXMLGUISystem::ZXMLGUISystem
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_iNumOfWindows, 0x31C); // Verified by ZXMLGUISystem::GetTopWindow
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_iNumOtherWindows, 0x320); // Verified by ZXMLGUISystem::AddOtherWindowCount
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_apWindowStack, 0x324); // Verified by ZXMLGUISystem::IsOnStack
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_pMenuElements, 0x428); // Verified by ZXMLGUISystem::ZXMLGUISystem
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_bAddBackgroundCamAnd3DCams, 0x433); // Verified by ZXMLGUISystem::Command ("CreateViews" command impl)
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_pTRCCam, 0x440); // Verified by ZXMLGUISystem::SetupCameras
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_aMenuLayer, 0x47C); // Verified by constructor vector init at +0x480
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_eCurrentLayer, 0x564); // Verified by ChangeLayer (DWORD index 0x159)
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_bWindowPush, 0x568); // Verified by ZXMLGUISystem::ZXMLGUISystem
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_apTRCWindows, 0x56C); // Verified by ZXMLGUISystem::IsOnStack
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_iNumOfTRCWindows, 0x57C); // Verified by ZXMLGUISystem::IsOnStack
}
