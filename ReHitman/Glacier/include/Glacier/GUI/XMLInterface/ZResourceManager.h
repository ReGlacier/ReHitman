#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>


namespace Glacier
{
    // fwds
    class ZWINGROUP;
    class ZFRAME;
    class ZTTFONT;
    class ZColorSet;
    class ZMenuElements;
    class IGUIElement;
    class IWindowInterface;
    class ZCAMERA;
    class ZGROUP;
    class ZGEOM;
    class IView;
    class ZWINOBJ;
    class ZLINEOBJ;
    class ZBUTTON;
    class ZButtonGraphic;
    class ZSlider;

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
        ZFRAME* GetFrame(const ZVector2& vPos, ZColorSet* pColorSet, ZWINGROUP* pParent,
            const ZVector2& vSize, const char* pszName, EAlignment eAlignment);
        ZWINGROUP* GetGraphic(const ZVector2& vPos, ZColorSet* pColorSet, ZWINGROUP* pParent,
            const char* pszName, EAlignment eAlignment, int iFlags);
        void ReleaseGraphic(ZWINGROUP* pGroup);
        void ReleaseFrame(ZFRAME* pFrame);
        void ReleaseTextGroup(ZWINGROUP* pGroup);
        void CreateMenu3DViews(int iFlags);

        void SetupResourceGroups(ZWINGROUP* pResources, ZGROUP* pFonts);

        ZWINGROUP* GetWingroup(ZWINGROUP* pParent);
        void ReleaseWinGroup(ZWINGROUP* pGroup);
        ZWINGROUP* GetBackgroundGroup();
        void SetBackgroundGroup(ZWINGROUP* pGroup);

        void SetColor(uint32_t iMask, ZWINOBJ* pWinObj, ZColorSet* pColorSet);
        void SetAlignment(ZWINOBJ* pWinObj, EAlignment eAlignment);

        ZLINEOBJ* GetLineObj(const ZVector2& vPos, ZColorSet* pColorSet, uint32_t iMask,
            EFontType eFontType, EAlignment eAlignment, bool bDisableAnimateAlpha);
        void AddLineObj(const ZVector2& vPos, ZColorSet* pColorSet, uint32_t iMask, ZWINGROUP* pGroup,
            EFontType eFontType, EAlignment eAlignment, ZStaticVector<ZLINEOBJ*, 8>* pLineObjs,
            bool bDisableAnimateAlpha, bool bShadow);
        ZWINGROUP* GetTextGroup(const ZVector2& vPos, ZColorSet* pColorSet, ZWINGROUP* pParent,
            uint32_t iMask, EFontType eFontType, bool bShadow, EAlignment eAlignment);
        void AddAdditionalLineObjs(ZStaticVector<ZLINEOBJ*, 8>* pLineObjs, const ZVector2& vPos,
            ZColorSet* pColorSet, ZWINGROUP* pGroup, EAlignment eAlignment, uint32_t iMask,
            EFontType eFontType, bool bShadow);

        ZBUTTON* GetButton(const ZVector2& vPos, ZColorSet* pColorSet, ZWINGROUP* pParent, int iId,
            ZStaticVector<ZWINOBJ*, 8>* pChecked, ZStaticVector<ZWINOBJ*, 8>* pUnchecked,
            EAlignment eAlignment, ZButtonGraphic* pButtonGraphic, uint32_t iType,
            EFontType eFontType, bool bShadow, bool bDisableAnimateAlpha);
        void ReleaseButton(ZBUTTON* pButton);
        void ReleaseLineObjects(ZWINGROUP* pGroup);
        void ReleaseButtonGraphic(ZWINGROUP* pGroup);

        void AddButtonGraphic(ZBUTTON* pButton, ZButtonGraphic* pButtonGraphic, ZColorSet* pColorSet,
            EAlignment eAlignment, ZStaticVector<ZWINOBJ*, 8>* pChecked,
            ZStaticVector<ZWINOBJ*, 8>* pUnchecked);
        void AddButtonGraphic(ZWINGROUP* pParent, ZButtonGraphic* pButtonGraphic, ZColorSet* pColorSet,
            EAlignment eAlignment, ZStaticVector<ZWINOBJ*, 8>* pChecked,
            ZStaticVector<ZWINOBJ*, 8>* pUnchecked, const float* pOffset);
        ZWINOBJ* GetButtonGraphic(const zstring& rName, bool bFrame);

        ZSlider* GetSlider(float* pfPos, ZColorSet* pColorSet, ZWINGROUP* pParent, int iIndex,
            int iLowerBound, int iUpperBound, int iSteps, ZButtonGraphic* pButtonGraphic, uint32_t iType,
            EFontType eFontType, bool bShadow, EAlignment eAlignment, float fSliderSize, float fSliderOffset);
        void ReleaseSlider(ZSlider* pSlider);
    };
    RE_VERIFY_SIZE(ZResourceManager, 0x2DC);
}
