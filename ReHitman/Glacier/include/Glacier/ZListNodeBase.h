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

    template <typename T, size_t U0>
    struct ZListIterator
    {
        ZListNode<T, U0>* m_Node;
    };

    struct ZListBase
    {
        ZListNodeBase m_Head;
    };
    RE_VERIFY_SIZE(ZListBase, 0x8);

    template <typename T, size_t U0, size_t U1>
    struct ZList : ZListBase {};
}