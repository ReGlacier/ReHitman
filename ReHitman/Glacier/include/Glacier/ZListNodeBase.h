#pragma once

#include <cstdint>

namespace Glacier
{
    struct ZListNodeBase
    {
        struct ZListNodeBase* pNext;     // +0
        struct ZListNodeBase* pPrev;     // +4
        int32_t mId;					 // +8
        float   m_unk0;                  // +C
    };
}