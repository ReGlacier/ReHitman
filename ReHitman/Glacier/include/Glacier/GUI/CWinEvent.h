#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/GUI/ZWINDOW.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/CBaseEvent.h>


namespace Glacier
{
    template <typename T> class CWinEvent : public CBaseEvent<T>
    {
        //vtbl
        virtual ZWINDOWS* GetSystem();
        virtual void WndMessage(ZWMEVENT* pEvent);
        virtual void OnCommand(ZMSGID lMsg);
        virtual void OnMouseMove(ZVector2& vPos);
        virtual void OnKeyUp(uint32_t);
        virtual void OnKeyDown(uint32_t);
        virtual void OnKeyPress(uint32_t);
        virtual void OnFocusReceived(uint32_t);
        virtual void OnFocusLost(uint32_t);
        virtual void OnClick(float x, float y);
        virtual void OnWindowOpen(uint32_t, bool);
        virtual void OnWindowClose(uint32_t, bool);
        virtual void OnSliderChange(uint32_t, uint32_t);
        virtual void OnFocusChanged(uint32_t, uint32_t);
    };
}
