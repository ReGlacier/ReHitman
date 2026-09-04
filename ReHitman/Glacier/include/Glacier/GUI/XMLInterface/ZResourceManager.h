#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZStaticVector.h>


namespace Glacier
{
    // fwds
    class ZWINGROUP;
    class ZFRAME;
    class ZTTFONT;
    class ZMenuElements;
    class IGUIElement;
    class IWindowInterface;
    class ZCAMERA;
    class ZGROUP;
    class ZGEOM;
    class IView;

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

        // methods
        void ReleaseTextGroup(ZWINGROUP* pGroup);
        void CreateMenu3DViews(int iFlags);
    };
    RE_VERIFY_SIZE(ZResourceManager, 0x2DC);
}
