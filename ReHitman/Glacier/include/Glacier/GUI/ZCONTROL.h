#pragma once

#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/ReGlacier.h>


namespace Glacier
{
    enum EFocusMove {
        Whatever = 0,
        Left = 1,
        Right = 2,
        Up = 3,
        Down = 4,
    };

    class ZCONTROL : public ZWINGROUP
    {
    public:    
        unsigned int m_rOwner; // +0x54
        unsigned int m_dwShortcut; // +0x58
        int m_dwId; // +0x5C
        unsigned int m_dwControlStyle; // +0x60
        unsigned int m_dwControlState; // +0x64
        unsigned int m_dwStates; // +0x68
        unsigned int m_rLeftControl; // +0x6C
        unsigned int m_rRightControl; // +0x70
        unsigned int m_rUpControl; // +0x74
        unsigned int m_rDownControl; // +0x78
        bool m_bNotifyControl; // +0x7C
        bool m_bTextControl; // +0x7D
        RE_ADD_PADDING(2); // +0x7E
        unsigned int m_dwNormalColor; // +0x80
        unsigned int m_dwHoverColor; // +0x84
        unsigned int m_dwFocusColor; // +0x88
        unsigned int m_dwPushColor; // +0x8C
        unsigned int m_dwDisabledColor; // +0x90
        unsigned int m_dwCheckColor; // +0x94

        // vtable
        virtual void Enable();
        virtual void Disable();
        virtual void NotifyControl();
        virtual void SetNotifyControl(bool notify);
        virtual void SetOwner(unsigned long long owner);
        virtual void SetState(unsigned int state, bool enable);
        virtual unsigned int GetState();
        virtual void UpdateStateGraphics(unsigned int state);
        virtual void SetAvailibleStates(unsigned int states);
        virtual void MouseLeave();
        virtual void MouseEnter(bool fromMouseMove);
        virtual void Push(bool push);
        virtual void Release();
        virtual void Click(unsigned int param);
        virtual void SetControlId(int id);
        virtual int GetControlId(); // I'm not sure about that
        virtual void GrabFocus();
        virtual ZCONTROL* GetNextFocus(EFocusMove move);
        virtual void SetNextFocus(ZCONTROL* control, EFocusMove move);
        virtual void OnCommand(unsigned int command);
        virtual void OnMouseMove(float* params);
        virtual void OnKeyUp(unsigned int key);
        virtual void OnKeyDown(unsigned int key);
        virtual void OnKeyPress(unsigned int key);
        virtual void OnKeyRepeat(unsigned int key);
        virtual void OnFocusReceived(unsigned long long param);
        virtual void OnFocusLost(unsigned long long param);
        virtual void OnClick(float* params, unsigned long long param);
        virtual void OnSliderChange(unsigned long long param, unsigned int value);
        virtual void OnScrollbarChange(unsigned long long param, float value);
        virtual void OnFocusChanged(unsigned long long oldFocus, unsigned long long newFocus);
        virtual void OnStateChanged(unsigned long long param, unsigned int state);
    }; // Verified size is 0x98
    RE_VERIFY_SIZE(ZCONTROL, 0x98);
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
