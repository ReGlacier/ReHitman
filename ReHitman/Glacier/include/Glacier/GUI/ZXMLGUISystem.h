#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/GUI/CWinEvent.h>
#include <Glacier/GUI/ZWINDOW.h>


namespace Glacier
{
    // fwds
    class ZWINGROUP;
    class ZFRAME;
    class ZTTFONT;
    class ZMenuElements;
    struct IGUIElement;
    struct IWindowInterface;
    class ZCAMERA;
    class ZGROUP;
    class ZGEOM;
    struct IView;


    struct SMenuLayer
    {
        ZWINGROUP* pParent;
        ZStaticVector<IGUIElement*, 16> vReceiveAllInput;
        ZFRAME* pOverlayFrame;
    };
    RE_VERIFY_SIZE(SMenuLayer, 0x4C); // Verified

    struct ZMapping
    {
        bool m_bFlip;
        bool m_bMirror;
        bool m_bRunWhenPause;
        float m_fCurVal;
        float m_fDstVal;
        float m_fStartVal;
        TIMETYPE m_StartTime;
        TIMETYPE m_TimeInterval;

        virtual float MapFunction();
    };
    RE_VERIFY_SIZE(ZMapping, 0x1C);

    struct ZEaseIn : public ZMapping
    {
        int32_t m_iPolyDegree;
    };
    RE_VERIFY_SIZE(ZEaseIn, 0x20);

    struct ZMenu3DCam
    {
        enum EState
        {
            eANIMATE_OUT = 0,
            ANIMATE_IN = 1,
            ANIMATE_IN2 = 2,
            IDLE = 3,
            JOYSTICK = 4
        };

        // Size: 0xac (172) bytes
        ZCAMERA* m_pCamera;
        IView* m_pIView;
        ZGEOM* m_pGeom;
        TIMETYPE m_LastActualTime;
        bool m_bReserved;
        ZEaseIn m_AnimateAngleY;
        ZEaseIn m_AnimateAngleZ;
        ZEaseIn m_AnimateCamPos;
        EState m_eState;
        TIMETYPE m_LastJoystickMove;
        ZGROUP* m_pReleaseGroup;
        ZGEOM* m_pNextGeom;
        float m_fDefaultCamPosX;
        ZMat3x3 m_DefaultMat;
    };
    RE_VERIFY_SIZE(ZMenu3DCam, 0xAC);

    struct ZResourceManager
    {
        ZWINGROUP* m_pWinGroupLineObjs;
        ZWINGROUP* m_pWinGroupButtons;
        ZWINGROUP* m_pWinGroupSlider;
        ZWINGROUP* m_pWinGroupGroups;
        ZWINGROUP* m_pWinGroupGraphic;
        ZWINGROUP* m_pWinGroupButtonGraphic;
        ZWINGROUP* m_pWinGroupFrames;
        ZWINGROUP* m_pWinGroupBackground;
        ZTTFONT* m_pFonts[3];
        ZMenu3DCam m_aMenu3DCam[4]; // Count approved by ZResourceManager::CreateMenu3DViews (while loop)
    };
    RE_VERIFY_SIZE(ZResourceManager, 0x2DC);

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
        virtual void SetFocus();
        virtual void AddOtherWindowCount(int);

        // api
        ZWINDOW* GetTopWindow();

        // data
        bool m_bUnpauseGame; // +0x30
        ZWINGROUP* m_pWinGroupResources; // +0x34
        ZGROUP* m_pGroupFonts; // +0x38
        ZResourceManager m_ResourceManager; // +0x3c
        int32_t m_iLastFocusedElement; // +0x318
        int32_t m_iNumOfWindows; // +0x31c
        int32_t m_iNumOtherWindows; // +0x320
        IWindowInterface* m_apWindowStack[32]; // +0x324
        int32_t m_aLastIndex[32]; // +0x3a4
        ZWINGROUP* m_pWinGroupHeader; // +0x424
        ZMenuElements* m_pMenuElements; // +0x428
        int32_t m_iPlayerActivatedMenu; // +0x42c
        bool m_bInsideUpdate; // +0x430
        bool m_bCloseSystem; // +0x431
        bool m_bGot3dBackground; // +0x432
        bool m_bAddBackgroundCamAnd3DCams; // +0x433
        ZCAMERA* m_pBackgroundCam; // +0x434
        ZCAMERA* m_pMenuCam; // +0x438
        ZCAMERA* m_pDialogCam; // +0x43c
        ZCAMERA* m_pTRCCam; // +0x440
        SOpenWindowParams m_OpenWindowParams; // +0x444
        SCloseWindowParams m_CloseWindowParams; // +0x458
        SRollBackParams m_RollBackParams; // +0x45c
        SMenuLayer m_aMenuLayer[3]; // +0x47c
        EMenuLayer m_eCurrentLayer; // +0x560
        bool m_bWindowPush; // +0x564
        bool m_bRollBack; // +0x565
        RE_ADD_PADDING(4); // Idk what's happening here
        IWindowInterface* m_apTRCWindows[4]; // +0x56C
        int m_iNumOfTRCWindows; // +0x57C
    };
    RE_VERIFY_SIZE(ZXMLGUISystem, 0x580); // Verified
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_bUnpauseGame, 0x30);
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_ResourceManager, 0x3C); // Verified by ZXMLGUISystem::ZXMLGUISystem
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_iNumOfWindows, 0x31C); // Verified by ZXMLGUISystem::GetTopWindow
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_iNumOtherWindows, 0x320); // Verified by ZXMLGUISystem::AddOtherWindowCount
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_apWindowStack, 0x324); // Verified by ZXMLGUISystem::IsOnStack
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_pMenuElements, 0x428); // Verified by ZXMLGUISystem::ZXMLGUISystem
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_bAddBackgroundCamAnd3DCams, 0x433); // Verified by ZXMLGUISystem::Command ("CreateViews" command impl)
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_pTRCCam, 0x440); // Verified by ZXMLGUISystem::SetupCameras
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_apTRCWindows, 0x56C); // Verified by ZXMLGUISystem::IsOnStack
    RE_VERIFY_OFFSET(ZXMLGUISystem, m_iNumOfTRCWindows, 0x57C); // Verified by ZXMLGUISystem::IsOnStack
}
