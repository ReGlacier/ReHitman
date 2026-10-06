#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/LINKSORTREFTAB.h>
#include <Glacier/GUI/ZWinInput.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    // fwds
    class ZWINDOW;
    class ZWinGfxHandler;
    struct ZGEOMREF;


    class ZWINDOWS : public ZWINGROUP
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZWINDOWS, 0x100030u);

        // vtbl
        ~ZWINDOWS() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void ClassFrameUpdate() override;
        ZGEOM* Duplicate(ZGROUP* pDestGroup, const char* psName, bool bRecursive) override;
        void CopyData(const ZGEOM* pSource) override;

        // ZWINDOWS
        virtual bool Notify(ZWM_MESSAGE eMessage, int evData1, int evData2, ZWINGROUP* group);
        virtual bool Notify(ZWMEVENT* pEvent, ZWINGROUP* pGroup);
        virtual bool NotifySubscribers(ZWMEVENT* pEvent);
        virtual void CheckCommands();
        virtual void SetFocusedControl(ZWINGROUP* pGroup);
        virtual ZWINGROUP* GetFocusedControl();
        virtual void ReleaseFocusedControl(ZWINGROUP* pGroup, bool bFocusParent);
        virtual void PushWindow(uint32_t id);
        virtual void PushWindow(ZWINDOW* pWindow);
        virtual void PopWindow();
        virtual ZREF SetMousePos(const ZVector2& vPos);
        virtual void GetMousePos(ZVector2& vPos);
        virtual void SetShowMouse(bool bShowMouse);
        virtual ZREF UpdateMouse();
        virtual void SetMouseSpeed(float fSpeed);
        virtual void SetMousePosition(const ZVector2& vDt);
        virtual void SetMouseDeltaPos(const ZVector2& vDt);
        virtual ZREF GetMouseGeom();
        virtual void ForceUpdateMouse();
        virtual void AddCamera(ZCAMERA* pCam);
        virtual void RemoveCamera(ZCAMERA* pCam);
        virtual void SetUseZBuffer(bool bValue);
        virtual bool SetClearScreen(bool bValue);
        virtual void SetBackColor(int lColor);
        virtual void SetBackColor(int r, int g, int b);
        virtual void SetGfxHandler(ZWinGfxHandler* pGfx);
        virtual ZWinGfxHandler* GetGfxHandler();
        virtual void AddSubscriber(uint32_t id, int eventId, ZWINGROUP* pGroup, float fPriority, bool flag0C);
        virtual void RemoveSubscriber(uint32_t id, int eventId, ZWINGROUP* pGroup);

        // methods
        ZWINDOWS(const char* psName, ZBaseGeom* pBaseGeom);

        int GetActivateBy(int iKey);
        void InvalidateView();
        void FadeGroup(uint8_t lAlpha, ZWINGROUP* pGroup);
        void EnableFade(bool bEnable);

        // RTTI property methods
        void GetMainCamera(ZGEOMREF& rCamera);
        void SetMainCamera(const ZGEOMREF& rCamera);
        void GetGfxHandlerRef(ZGEOMREF& rHandler);
        void SetGfxHandlerRef(const ZGEOMREF& rHandler);

        // members
        ZWINGROUP* m_pControlInFocus;
        ZCAMERA* m_pMainCamera;
        ZREF m_rMouse;
        ZVector2 m_v2MousePos;
        ZVector2 m_v2glMouse;
        ZVector2 m_v2MouseDelta;
        ZVector2 m_v2Pos;
        float m_fMouseSpeed;
        float m_fBgScale;
        LINKSORTREFTAB m_Subscribers;
        LINKREFTAB m_WindowStack;
        REFTAB32 m_rtFreeSpriteArrays;
        ZREF m_rLastHit;
        ZREF m_rLastPushed;
        LINKSORTREFTAB m_Cameras;
        ZREF m_rHitGeom;
        ZWinGfxHandler* m_pGfxHandler;
        int m_aiInputQue[5];
        int m_iInputQuePos;
        bool m_bLeftMouseDownLastFrame;
        bool m_bLeftMouseDown;
        bool m_bMouseActive;
        bool m_bUseZBuffer;
        bool m_bClearScreen;
        char m_bUpdateMouseFocus;
        RE_ADD_PADDING(2);
        ZWINGROUP* m_pExternalMouseColiGroup;
        bool m_bShow2d;
        bool m_bFadeEnabled;
        RE_ADD_PADDING(2);
        float m_fFade;
        ZWinInput m_winInput;
        uint32_t m_iAlphaBackupCount;
        uint8_t m_aAlphaBackup[8];

    private:
        void LoadInit();
        void BackupAlpha(ZWINGROUP* pGroup);
        void RestoreAlpha(ZWINGROUP* pGroup);
    };
    RE_VERIFY_SIZE(ZWINDOWS, 0x36C); // Verified PC alloc
    RE_VERIFY_OFFSET(ZWINDOWS, m_pControlInFocus, 0x54);
    RE_VERIFY_OFFSET(ZWINDOWS, m_WindowStack, 0xB4);
    RE_VERIFY_OFFSET(ZWINDOWS, m_aiInputQue, 0x1C4);
    RE_VERIFY_OFFSET(ZWINDOWS, m_bLeftMouseDown, 0x1DD);
    RE_VERIFY_OFFSET(ZWINDOWS, m_bMouseActive, 0x1DE);
    RE_VERIFY_OFFSET(ZWINDOWS, m_winInput, 0x1F0);
    RE_VERIFY_OFFSET(ZWINDOWS, m_iAlphaBackupCount, 0x360);
    RE_VERIFY_OFFSET(ZWINDOWS, m_aAlphaBackup, 0x364);
}
