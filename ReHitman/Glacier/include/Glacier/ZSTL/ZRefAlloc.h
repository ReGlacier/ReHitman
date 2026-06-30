#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>


namespace Glacier
{
    struct SRefLink
    {
        unsigned int m_rRef;
        unsigned int m_lNext;
    };
    RE_VERIFY_SIZE(SRefLink, 0x8);

    struct ZRefAlloc
    {
        int m_lNumLinks;
        int m_lNextFree;
        SRefLink m_Links[2248];
    };
    RE_VERIFY_SIZE(ZRefAlloc, 0x4648);
}