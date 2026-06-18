#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct ZListNodeBase
    {
        struct ZListNodeBase* m_Next;
        struct ZListNodeBase* m_Prev;
    };

    RE_VERIFY_SIZE(ZListNodeBase, 0x8);

    template <typename T, size_t N>
    struct ZListNode : ZListNodeBase
    {
    };
}