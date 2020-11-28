#pragma once

#include <cstdint>

namespace Glacier
{
    struct ZLinkedListNodeBase
    {
        struct ZLinkedListNodeBase* pNext;     // +0
        struct ZLinkedListNodeBase* pPrev;     // +4
        int32_t mId;					// +8
        char pad_0010[4];				// +C
    };
}