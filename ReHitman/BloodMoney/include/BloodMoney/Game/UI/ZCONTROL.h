#pragma once

#include <BloodMoney/Game/UI/ZWINGROUP.h>

namespace Hitman::BloodMoney {
    enum class EFocusMove {
        TryToFindFocusFromTopValueToBottom = 0,
        Value_1 = 1,
        Value_2 = 2,
        Value_3 = 3,
        Value_4 = 4,
    };

    enum class EControlState {
        CS_RELEASED = 1u,
        CS_PUSHED = 2u,
        CS_HOVERED = 8u,
        CS_DISABLED = 32u
    };

    enum EControlMask {
        CM_DISABLED = 1 << 1
    };

    class ZCONTROL : public ZWINGROUP {
    public:
        //vftable
        virtual void Enable();
        virtual void Disable();
        virtual bool NotifyControl();
        virtual void SetNotifyControl(bool);
        virtual void SetOwner(Glacier::uint);
        virtual void SetState(Glacier::uint);
        virtual Glacier::uint GetState();
        virtual void UpdateStateGraphics(Glacier::uint);
        virtual void SetAvailableStates(Glacier::uint);
        virtual void MouseLeave();
        virtual void MouseEnter(bool);
        virtual void Push(bool);
        virtual void Release();
        virtual void Click();
        virtual void SetControlId(int id);
        virtual int GetControlId();
        virtual void GrabFocus();
        virtual ZCONTROL* GetNextFocus(EFocusMove eFocusMove);
        virtual void SetNextFocus(ZCONTROL* pControl, EFocusMove eFocusMove);
        virtual void OnCommand(Glacier::uint);
        virtual void OnMouseMove(Glacier::Vector2* pMousePos);
        virtual void OnKeyUp(Glacier::uint);
        virtual void OnKeyDown(Glacier::uint);
        virtual void OnKeyPress(Glacier::uint);
        virtual void OnKeyRepeat(Glacier::uint);
        virtual void OnFocusReceived(Glacier::uint);
        virtual void OnFocusLost(Glacier::uint);
        virtual void OnClick(Glacier::ZVector2* pPos, Glacier::uint);
        virtual void OnSliderChange(Glacier::uint, Glacier::uint);
        virtual void OnScrollbarChange(Glacier::uint, Glacier::uint);
        virtual void OnFocusChanged(Glacier::uint, Glacier::uint);
        virtual void OnStateChanged(Glacier::uint, Glacier::uint);

        //data (total size is 0x98, base size is 0x54)
        Glacier::ZREF m_rOwner;
        int m_field58;
        int m_Id;
        int m_stateMask;
        EControlState m_currentState;
        int m_availableStates;
        int m_rControlToFocus1;
        int m_rControlToFocus2;
        int m_rControlToFocus3;
        int m_rControlToFocus4;
        bool m_bNotifyControl;
        bool m_field7D;
        bool m_field7E;
        bool m_field7F;
        int m_field80;
        int m_field84;
        int m_field88;
        int m_field8C;
        int m_field90;
        int m_field94;
    };
}