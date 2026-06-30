#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    struct ZBlockAlloc
    {
        int m_lNumBlocks;
        int m_lNumAllocated;
        int m_lNextFree;
        int m_lBlockSize;
        int m_lAlignment;
        char *m_pBlockMem;
        char *m_pBlockMemAligned;
        int *m_pFreeMemTab;
    };
    RE_VERIFY_SIZE(ZBlockAlloc, 0x20);
}