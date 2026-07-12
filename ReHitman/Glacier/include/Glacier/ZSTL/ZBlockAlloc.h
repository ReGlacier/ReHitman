#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    /**
     * @brief A fixed-size block memory allocator with support for chained allocations.
     * 
     * This allocator manages a contiguous pool of memory divided into equal-sized blocks.
     * It uses an external FAT-like allocation table (m_pFreeMemTab) instead of an intrusive 
     * linked list, allowing it to allocate multiple non-contiguous physical blocks and 
     * track them as a single logical chain.
     */
    struct ZBlockAlloc
    {
        // methods
        ZBlockAlloc() = default;
        ZBlockAlloc(int iAlignment) : m_lAlignment{iAlignment} {}
        ~ZBlockAlloc();

        void Create(int lNumBlocks, int lBlockSize);
        void* AllocBlocks(int lNumBlocks);

        // members
        int m_lNumBlocks{0};
        int m_lNumAllocated{0};
        int m_lNextFree{0};
        int m_lBlockSize{0};
        int m_lAlignment{64};
        char *m_pBlockMem{nullptr};
        char *m_pBlockMemAligned{nullptr};
        int *m_pFreeMemTab{nullptr};
    };
    RE_VERIFY_SIZE(ZBlockAlloc, 0x20);
}