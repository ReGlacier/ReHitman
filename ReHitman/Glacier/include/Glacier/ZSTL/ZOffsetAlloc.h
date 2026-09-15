#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/EAllocType.h>
#include <cstdint>


namespace Glacier
{
    class REFTAB;

    /**
     * @class ZOffsetAlloc
     * @brief A lightweight, array-based offset manager for managing contiguous memory pools.
     * 
     * @details
     * ZOffsetAlloc tracks and manages free memory regions using a flat array of dynamic links (@ref ZLink).
     * Instead of tracking allocated blocks, it exclusively stores available (free) spaces. 
     * To sustain high performance, it utilizes an O(1) element removal strategy by swapping the 
     * requested element with the last item in the array, avoiding heavy memory shifts.
     * 
     * Key behaviors:
     * - Coalescing: Automatically merges adjacent free blocks during @ref Free operations to mitigate fragmentation.
     * - Dual-Strategy Allocation: Supports both Best-Fit allocation from the bottom and High-Address allocation from the top of the pool.
     */
    class ZOffsetAlloc
    {
    public:
        // types
        /** @brief One free range tracked by offset and size. */
        struct ZLink
        {
            /** @brief Start offset of the free range. */
            unsigned int m_lOffset;
            /** @brief Size of the free range in bytes/units used by the caller. */
            unsigned int m_lSize;
        };
        RE_VERIFY_SIZE(ZLink, 0x8); // Verified

        // vtbl
        /** @brief Releases the internally allocated link buffer, if this instance owns it. */
        virtual ~ZOffsetAlloc();
        /** @brief Returns the sum of all free link sizes. */
        virtual int GetFreeTotal() const;
        /**
         * @brief Appends the current free list to @p pRefTab.
         *
         * @pre @p pRefTab must not be null.
         * @details If @p pRefTab has element size 1, each range is written as two refs:
         * offset followed by size. Otherwise each element stores size as the ref value
         * and offset as the first extra user-data field.
         */
        virtual void GetFreeList(REFTAB* pRefTab) const;

        // methods
        /**
         * @brief Creates an offset allocator for the half-open range [lStartOffset, lEndOffset).
         *
         * @param lStartOffset First managed offset.
         * @param lEndOffset One-past-last managed offset; must be greater than @p lStartOffset.
         * @param lMaxNrAllocs Maximum number of simultaneously tracked free ranges.
         * @param pBuffer Optional caller-owned link buffer. When null, the allocator allocates one internally.
         * @param eType Memory type used for the internal link buffer allocation.
         *
         * @pre Neither @p lStartOffset nor @p lEndOffset may have bit 31 set.
         * @pre @p lEndOffset must be greater than @p lStartOffset.
         * @pre @p lMaxNrAllocs must be large enough to hold at least the initial free range.
         */
        ZOffsetAlloc(uint32_t lStartOffset, uint32_t lEndOffset, uint32_t lMaxNrAllocs, ZOffsetAlloc::ZLink *pBuffer, EAllocType eType);
        /**
         * @brief Clears the free-list state and replaces it with [lStartOffset, lEndOffset).
         *
         * @pre Neither offset may have bit 31 set.
         * @pre @p lEndOffset must be greater than @p lStartOffset.
         * @pre There must be room for one free link.
         */
        void Reset(uint32_t lStartOffset, uint32_t lEndOffset);
        /** @brief Returns the current number of tracked free ranges. */
        uint32_t GetNrFreeLinks() const;
        /** @brief Returns the size of the largest currently tracked free range, or 0 when none exist. */
        uint32_t GetLargestFreeBlock() const;
        /**
         * @brief Allocates a block of memory using a Best-Fit strategy from the lowest available address.
         * 
         * @details
         * This method scans the list of free blocks linearly. If an exact size match is found, 
         * it immediately claims the entire block and updates the tracker in O(1) time. 
         * Otherwise, it follows a **Best-Fit** policy, finding the smallest free block that can 
         * accommodate the requested size to minimize memory fragmentation.
         * Once chosen, the memory is sliced from the **beginning (lowest address)** of that free block.
         * 
         * @param lSize The requested allocation size.
         * @param bLargeUseHigh Present for ABI compatibility; current implementation ignores it.
         * @return void* A pointer-casted offset to the allocated space, or `(void*)-1` if allocation failed.
         */
        void* Alloc(uint32_t lSize, bool bLargeUseHigh);
        /**
         * @brief Allocates a block of memory from the highest available address space.
         * 
         * @details
         * **How it differs from Alloc:**
         * While `Alloc` searches for the smallest fitting block (Best-Fit) and carves memory from its 
         * start, `AllocHi` targets the free block with the **highest base offset** (closest to the end of the pool). 
         * Furthermore, it slices the requested space from the **end (highest address)** of that chosen block.
         * This strategy isolates specific allocations at the top of the memory pool, leaving the lower pool 
         * uninterrupted for standard, sequential allocations.
         * 
         * @param lSize The requested allocation size.
         * @return void* A pointer-casted offset to the allocated space, or `(void*)-1` if allocation failed.
         */
        void* AllocHi(uint32_t lSize);
        /**
         * @brief Returns an offset range to the free list and coalesces adjacent ranges.
         *
         * @details The allocator does not track allocated blocks, so callers must provide a valid
         * offset/size pair that was previously allocated or otherwise belongs to the managed range.
         * Adjacent free ranges are merged when the released range touches their start or end.
         *
         * @pre Adding a non-adjacent free range requires space for another @ref ZLink.
         */
        void Free(uint32_t lOffset, uint32_t lSize);
        /**
         * @brief Removes @p pLnk from the free-list by overwriting it with the last link.
         *
         * @details This keeps removal O(1), but does not preserve link ordering.
         * @pre @p pLnk must point into the active portion of @ref m_pLinks.
         */
        void CopyLastToCurrent(ZOffsetAlloc::ZLink* pLnk);
        /**
         * @brief Appends a free range without attempting to coalesce it.
         *
         * @pre The link array must not be full.
         */
        void AddFreeSpace(uint32_t lStartOffset, uint32_t lSize);

        // members
        /** @brief Array of tracked free ranges. Owned by this instance unless user supplied it. */
        ZLink* m_pLinks;
        /** @brief Number of active entries in @ref m_pLinks. */
        int m_lNrLinks;
        /** @brief Capacity of @ref m_pLinks. */
        int m_lMaxNrLinks;
        /** @brief True when @ref m_pLinks was supplied by the caller and must not be freed here. */
        bool m_bUserSuppliedLinkBuffer;
        /** @brief Padding preserved for binary compatibility. */
        bool m_pad[3];
    };
    RE_VERIFY_SIZE(ZOffsetAlloc, 0x14);
}
