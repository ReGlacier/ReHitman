#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZWinEvents.h>
#include <Glacier/EventBase/ZEventBase.h>

#include <BloodMoney/Game/UI/ZWINDOW.h>

namespace Hitman::BloodMoney
{
    /**
     * @todo Think about class inheritance here. Probably,
     *       I'm not right here and partial specialization isn't a key to this solution.
     *       I guess the virtual functions table was built from template type 'T",
     *       not manually but I don't know where is the class' T data?
     * @tparam T - type of class
     */
    template <typename T> class CWinEvent;

    template <> class CWinEvent<ZWINDOW> : public Glacier::ZEventBase
    {
    public:
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

        /**
         * Base size : 0x30
         * Size      : 0x30
         *
         * Why:
         *   Classes:
         *     CIngameMap       - 0x2C4
         *     ZActionMenu      - 0x810
         *     ZActionMenuBase  - N/A (not instantiated manually)
         *     ZActionMenuFade  - 0xD0
         *     ZCheatMenu       - 0x78
         *     ZContainerMenu   - 0x658
         *     ZDropDisplay     - 0xD0
         *     ZPickupMenu      - 0x808
         *     ZXMLGUISystem    - 0x580
         *
         * If we will look for CIngameMap we will see that the first member (ZHandle) was located at 0x30, its class-specific location.
         * This means that CWinEvent<ZWINDOW> doesn't contain any internal fields or smth like that.
         */
    };
}