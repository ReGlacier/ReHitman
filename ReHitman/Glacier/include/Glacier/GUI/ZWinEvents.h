#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    enum ZWM_MESSAGE : uint32_t
    {
        ZWM_COMMAND = 1,
        ZWM_MOUSEMOVE = 2,
        ZWM_KEYDOWN = 4,
        ZWM_KEYREPEAT = 8,
        ZWM_KEYUP = 16,
        ZWM_FOCUSRECEIVED = 32,
        ZWM_FOCUSLOST = 64,
        ZWM_CLICK = 128,
        ZWM_WINDOWOPEN = 256,
        ZWM_WINDOWCLOSE = 512,
        ZWN_FIRSTNOTIFY = 65536,
        ZWN_SLIDERCHANGE = 65536,
        ZWN_SCROLLBARCHANGE = 524288,
        ZWN_FOCUSCHANGED = 1048576,
        ZWN_STATECHANGED = 2097152,
    };

    union _evdata
    {
        uint32_t _uint;
        int32_t  _int;
        float  _float;
    };

    struct ZWMEVENT
    {
        ZREF Target;
        ZWM_MESSAGE Message;
        int Param1;
        int Param2;
        bool Return;
    };
    RE_VERIFY_SIZE(ZWMEVENT, 0x14);

    struct ZWMSUBSCRIPTION
    {
        ZREF rSubscriber;
        uint32_t dwMessages;
        _evdata Param1;
        bool bCompareParam;
    };
}
