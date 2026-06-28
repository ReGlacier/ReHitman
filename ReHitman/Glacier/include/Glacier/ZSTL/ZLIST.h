#pragma once

#include <Glacier/ZListNodeBase.h>

namespace Glacier
{ 
    struct ZListBase
    {
        ZListNodeBase m_Head;
    };

    template <typename T, bool A, int B>
    struct ZList : ZListBase {};
}