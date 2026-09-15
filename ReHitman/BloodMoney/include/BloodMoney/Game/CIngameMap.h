#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Action/ActionInterface.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementRaw.h>
#include <Glacier/Render/Sprite/SSpriteArray.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/GUI/CWinEvent.h>
#include <Glacier/GUI/ZWINDOW.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <BloodMoney/Game/SMapGroup.h>
#include <BloodMoney/Game/SIconBase.h>



#ifdef small // thx msvc
#   undef small
#endif

namespace Hitman::BloodMoney
{
    enum EAnimMode
    {
        none = 0,
        scale = 1,
        jump = 2,
        alpha = 4,
        saturation = 8,
        big = 16,
        small = 32
    };

    struct SIconDef
    {
        uint16_t  u;
        uint16_t  v;
        uint16_t  w;
        uint16_t  h;
        uint32_t  iColor;
        EAnimMode eAnimMode;
        bool      bRotate;
        int8_t    iMaxDifficulty;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(SIconDef, 0x14);

    struct SMapText : public SIconBase
    {
        char szText[128];
        uint32_t   iColor;
        EAnimMode eAnimMode;
        float     fSize;
        int8_t    iAlignment;
        bool      bRotate;
        RE_ADD_PADDING(2);
    }; //Size: 0x0098 (see CIngameMap::AddText at 00663E20 for details)
    RE_VERIFY_SIZE(SMapText, 0x98); // Verified

    struct SIconInstance : public SIconBase
    {
        uint32_t iIconIndex;
    };
    RE_VERIFY_SIZE(SIconInstance, 0xC);

    struct CMapIconDraw : public Glacier::ZSTDOBJ
    {
        struct SMatPos
        {
            Glacier::ZVector3 vPos;
            Glacier::ZMat3x3 mPos;
        };
        RE_VERIFY_SIZE(SMatPos, 0x30);

        struct SIconInfo
        {
            uint32_t m_iIcon;
            Glacier::ZREF m_rBaseGeom;
            SMatPos m_sLastPos;
        };
        RE_VERIFY_SIZE(SIconInfo, 0x38);

        // Size: 0x4bac (19372) bytes
        Glacier::ZStackArray<256, CMapIconDraw::SIconInfo> m_Icons;
        Glacier::ZStackArray<32, SMapText> m_Texts;
        Glacier::ZStackArray<32, Glacier::ZCHAROBJ*> m_TextObjects;
        uint32_t* m_pPrims;
        Glacier::SSpriteArray* m_pSpriteArrays;
        Glacier::SSpriteArrayElementRaw* m_pSprites;
        float m_fScale;
    };
    RE_VERIFY_SIZE(CMapIconDraw, 0x4BAC);

    class CIngameMap : public Glacier::CWinEvent<Glacier::ZWINDOW>
    {
    public:
        //vftable
        virtual void AddMapGroup(uint16_t,char const*,uint16_t);
        virtual void OpenMap(void);
        virtual void CloseMap(void);
        virtual void AddIcon(SIconInstance*,bool);
        virtual SIconInstance* GetIcon(uint16_t);
        virtual void RemoveIcon(SIconInstance*);
        virtual SMapText* AddText(SMapText const*);
        virtual void RemoveText(SMapText const*);
        virtual void NotifyUpdate(uint16_t);

        //data (total size is 0x2C4, CWinEvent<ZWINDOW> : ZEventBase size is 0x30)
        Glacier::Action::ZHandle m_ahLegend;
        Glacier::Action::ZHandle m_ahSelect;
        Glacier::Action::ZHandle m_ahCamLeft;
        Glacier::Action::ZHandle m_ahCamRight;
        Glacier::Action::ZHandle m_ahCamUp;
        Glacier::Action::ZHandle m_ahCamDown;
        Glacier::Action::ZHandle m_ahMoveUp;
        Glacier::Action::ZHandle m_ahMoveDown;
        Glacier::Action::ZHandle m_ahPrev;
        Glacier::Action::ZHandle m_ahNext;
        Glacier::Action::ZHandle m_ahMenu;
        Glacier::Action::ZHandle m_ahMap;
        Glacier::ZREF m_rBackgroundCamera;
        Glacier::ZREF m_rMapCamera;
        Glacier::ZREF m_rOverlayCamera;
        Glacier::ZREF m_rIconCamera;
        Glacier::ZREF m_rIconGroup;
        Glacier::ZREF m_rReferenceCamera;
        Glacier::ZREF m_rZoomCursor;
        Glacier::ZREF m_rPanCursor;
        Glacier::ZREF m_rBackgroundGroup;
        Glacier::ZREF m_rOverlayGroup;
        Glacier::ZREF m_rZoomBox;
        Glacier::ZREF m_rSelectionList;
        Glacier::ZREF m_rCurrentMapTitle;
        Glacier::ZREF m_rLegendButton;
        Glacier::REFTAB32 m_rtButtons;
        Glacier::ZREF m_rCompass;
        SMapGroup* m_pLegendGroup;
        Glacier::ZREF m_rReferenceMap;
        Glacier::REFTAB m_rtGroups;
        int32_t m_iCurrentMapIndexOffset;
        CMapIconDraw* m_pDraw;
        Glacier::REFTAB m_rtIcons;
        Glacier::REFTAB m_rtTexts;
        Glacier::ZVector2 m_v2MapPosition;
        Glacier::ZVector2 m_v2MousePosition;
        Glacier::ZVector3 m_vOldCamPos;
        Glacier::ZVector3 m_vOldZoomBoxPos;
        Glacier::ZVector3 m_vZoomPanMin;
        Glacier::ZVector3 m_vZoomPanMax;
        Glacier::ZMat3x3 m_mBoxMat;
        float m_fZoomBoxStartScale;
        float m_fZoomBoxEndScale;
        float m_fCurrentScale;
        float m_fScalePul;
        Glacier::ZVector2 m_v2SubScale;
        Glacier::ZVector2 m_v2MouseStartPosition;
        float m_fMouseDownTime;
        Glacier::ZGROUP* m_pCurSniperOverlay;
        bool m_bMapListOpen;
        bool m_bShowingLegend;
        bool m_bScopecameraDeactivatedOnWindowOpen;
        bool m_bMouseInside;
        bool m_bMouseMoving;
        bool m_bLMouseDown;
        bool m_bCrossHack;
        bool m_bWasEnable4_3CutOff;
        bool m_bZoom_Pressed;
        bool m_bPan_Pressed;
        RE_ADD_PADDING(2);
        Glacier::ZREF m_sound_Moving;
        uint32_t m_lRemMaxFrameInterval;
        SMapGroup* m_pCurrentMap;
        SMapGroup* m_pLastMap;
        Glacier::ZREF m_rTmpMapGroup;
        Glacier::ZREF m_rTmpWorldGroup;
        Glacier::ZREF m_rHintObject;
        Glacier::TIMETYPE m_fHintTimeout;
    };
    RE_VERIFY_SIZE(CIngameMap, 0x2C4); // Verified
}
