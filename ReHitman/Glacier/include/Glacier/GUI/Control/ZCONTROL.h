#pragma once

#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    enum EFocusMove
    {
        Whatever = 0,
        Left = 1,
        Right = 2,
        Up = 3,
        Down = 4,
    };

    class ZCONTROL : public ZWINGROUP
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZCONTROL, 0x80100032u);

        // vtbl
        ~ZCONTROL() override;

        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void Hide(bool bHide) override;
        void HideRecursive(bool bHide) override;
        void ClassInit() override;
        void ClassInit2() override;
        void CopyData(const ZGEOM* pSource) override;

        bool WndMessage(ZWMEVENT* pEvent) override;

        virtual void Enable();
        virtual void Disable();
        virtual bool NotifyControl();
        virtual void SetNotifyControl(bool notify);
        virtual void SetOwner(ZREF owner);
        virtual void SetState(uint32_t state);
        virtual uint32_t GetState();
        virtual void UpdateStateGraphics(uint32_t state);
        virtual void SetAvailibleStates(uint32_t states);
        virtual void MouseLeave();
        virtual void MouseEnter(bool fromMouseMove);
        virtual void Push(bool push);
        virtual void Release();
        virtual void Click();
        virtual void SetControlId(int id);
        virtual int GetControlId(); // I'm not sure about that
        virtual void GrabFocus();
        virtual ZCONTROL* GetNextFocus(EFocusMove move);
        virtual void SetNextFocus(ZCONTROL* control, EFocusMove move);
        virtual bool OnCommand(uint32_t command);
        virtual bool OnMouseMove(float* params);
        virtual bool OnKeyUp(uint32_t key);
        virtual bool OnKeyDown(uint32_t key);
        virtual bool OnKeyPress(uint32_t key);
        virtual bool OnKeyRepeat(uint32_t key);
        virtual bool OnFocusReceived(uint32_t param);
        virtual bool OnFocusLost(uint32_t param);
        virtual bool OnClick(float* params, uint32_t param);
        virtual bool OnSliderChange(uint32_t param, uint32_t value);
        virtual bool OnScrollbarChange(uint32_t param, float value);
        virtual bool OnFocusChanged(uint32_t oldFocus, uint32_t newFocus);
        virtual bool OnStateChanged(uint32_t param, uint32_t state);

        // methods
        ZCONTROL(const char* psName, ZBaseGeom* pBaseGeom);

        void HideNonStateObject(uint32_t state, ZBaseGeom* pBaseGeom);
        void GetVisible(bool& visible);
        void SetVisible(const bool& visible);
        void GetDisabled(bool& disabled);
        void SetDisabled(const bool& disabled);
        // members
        uint32_t m_rOwner; // +0x54
        uint32_t m_dwShortcut; // +0x58
        int m_dwId; // +0x5C
        uint32_t m_dwControlStyle; // +0x60
        uint32_t m_dwControlState; // +0x64
        uint32_t m_dwStates; // +0x68
        uint32_t m_rLeftControl; // +0x6C
        uint32_t m_rRightControl; // +0x70
        uint32_t m_rUpControl; // +0x74
        uint32_t m_rDownControl; // +0x78
        bool m_bNotifyControl; // +0x7C
        bool m_bTextControl; // +0x7D
        RE_ADD_PADDING(2); // +0x7E
        uint32_t m_dwNormalColor; // +0x80
        uint32_t m_dwHoverColor; // +0x84
        uint32_t m_dwFocusColor; // +0x88
        uint32_t m_dwPushColor; // +0x8C
        uint32_t m_dwDisabledColor; // +0x90
        uint32_t m_dwCheckColor; // +0x94
    };
    RE_VERIFY_SIZE(ZCONTROL, 0x98); // Verified PC alloc
    RE_VERIFY_OFFSET(ZCONTROL, m_rOwner, 0x54);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwShortcut, 0x58);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwId, 0x5C);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwControlStyle, 0x60);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwControlState, 0x64);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwStates, 0x68);
    RE_VERIFY_OFFSET(ZCONTROL, m_rLeftControl, 0x6C);
    RE_VERIFY_OFFSET(ZCONTROL, m_rRightControl, 0x70);
    RE_VERIFY_OFFSET(ZCONTROL, m_rUpControl, 0x74);
    RE_VERIFY_OFFSET(ZCONTROL, m_rDownControl, 0x78);
    RE_VERIFY_OFFSET(ZCONTROL, m_bNotifyControl, 0x7C);
    RE_VERIFY_OFFSET(ZCONTROL, m_bTextControl, 0x7D);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwNormalColor, 0x80);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwHoverColor, 0x84);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwFocusColor, 0x88);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwPushColor, 0x8C);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwDisabledColor, 0x90);
    RE_VERIFY_OFFSET(ZCONTROL, m_dwCheckColor, 0x94);
}
