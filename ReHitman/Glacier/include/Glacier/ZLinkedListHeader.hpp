#pragma once

#include <Glacier/Glacier.h>

namespace Glacier
{
    template <typename T> struct ZLinkedListHeader
    {
        using ItemType = T;

        ItemType* prev;
        ItemType* next;

        ItemType* begin() { return reinterpret_cast<T*>(reinterpret_cast<std::intptr_t*>(next) - 1); }
        ItemType* end() { return reinterpret_cast<T*>(reinterpret_cast<std::intptr_t*>(prev) - 1); }
    };

    struct ZLinkedListNodeBase
    {
        ZLinkedListNodeBase* next;
        ZLinkedListNodeBase* prev;
        int32_t ID;
        char __PAD10__[4];
    };
}