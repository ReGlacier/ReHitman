#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    using SMouseColi = Glacier::Vector4;

    enum EWMEventType
    {
        OPEN_WINDOW = 0x100,
        CLOSE_WINDOW = 0x200,
        MOUSE_SCROLL = 0x10,
        MOUSE_LEAVE = 0x40,
        MOUSE_PRESS = 0x2,
        MOUSE_RELEASE = 0x1,
        UNKNOWN_EVENT_0 = 0x80,
        UNKNOWN_EVENT_1 =  0x1000,
        UNKNOWN_EVENT_2 =  0x100000,
    };

    struct ZWMEVENT {
        int field_0; // unknown field, maybe unused
        EWMEventType eventType;
        int objectId;
        int flag;
    };
}