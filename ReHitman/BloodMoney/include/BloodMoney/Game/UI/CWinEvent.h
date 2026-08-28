#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/CBaseEvent.h>

#include <BloodMoney/Game/UI/ZWINDOW.h>
#include <BloodMoney/Game/UI/ZWINGROUP.h>

namespace Hitman::BloodMoney
{
    template <typename T> class CWinEvent : public Glacier::CBaseEvent<T>
    {
        //vftable
        virtual ZWINDOWS* GetSystem();                              //#36 | +90
        virtual void WndMessage(Glacier::ZWMEVENT *);               //#37 | +94
        virtual void OnCommand(Glacier::ZMSGID);                    //#38 | +98
        virtual void OnMouseMove(Glacier::ZVector2*);               //#39 | +9C
        virtual void OnKeyUp(unsigned int);                         //#40 | +104
        virtual void OnKeyDown(unsigned int);                       //#41 | +108
        virtual void OnKeyPress(unsigned int);                      //#42 | +10C
        virtual void OnFocusReceived(unsigned int);                 //#43 | +110
        virtual void OnFocusLost(unsigned int);                     //#44 | +114
        virtual void OnClick(float x, float y);                     //#45 | +118
        virtual void OnWindowOpen(unsigned int, bool);              //#46 | +11C
        virtual void OnWindowClose(unsigned int, bool);             //#47 | +120
        virtual void OnSliderChange(unsigned int, unsigned int);    //#48 | +124
        virtual void OnFocusChanged(unsigned int, unsigned int);    //#49 | +128
    };
}
