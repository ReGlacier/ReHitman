#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZIKLNKOBJ;
    class ZGEOM;

    // types
    using IKCallBack_t = void(__thiscall*)(ZGEOM*);

    union ZIKCALLBACK
    {
        uint32_t p[4]; // 128bit repr (int128_t on other platforms)

        struct // PMF (func_ptr + this-delta + vbtable offset) + id
        {
            IKCallBack_t m_pCallback;   // +0x00 func_ptr
            int32_t m_delta;             // +0x04 this-adjustment (always 0)
            int32_t m_vbtableOffset;     // +0x08 vbtable offset (always 0)
            int32_t lCallBackId;         // +0x0C id
        };
    };
    RE_VERIFY_SIZE(ZIKCALLBACK, 0x10);

    using ZIKCallbacks = ZIKCALLBACK;
}
