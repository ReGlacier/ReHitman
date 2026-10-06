#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief A single contiguous free memory range tracked by ZMallocSimple.
     *
     * Used links are kept packed at the front of ZMallocSimple::m_pFreeLinks;
     * removal is done by swap-with-last (see ZMallocSimple::FreeFreeLink).
     */
    struct SFreeLink
    {
        // methods
        SFreeLink();
        SFreeLink(char* pFreeRam, uint32_t lFreeSize);

        // members
        char* m_pFreeRam { nullptr };   ///< Start address of the free range.
        uint32_t m_lFreeSize { 0u };    ///< Size of the free range in bytes.
    };
    RE_VERIFY_SIZE(SFreeLink, 0x8);

    /**
     * @brief Simple best-fit allocator over a caller-provided memory pool.
     *
     * The pool is registered block by block via AddBlock(); each block becomes
     * an SFreeLink entry. Alloc() carves whole-block-sized chunks out of the
     * best-fitting free link (smallest remainder, remainder < 8 bytes is
     * absorbed into the allocation), Free() puts the chunk back and coalesces
     * it with adjacent free links.
     *
     * Allocation layout (see the note at the top of ZMallocSimple.cpp):
     * every allocation is preceded by an m_lAlignment-byte header storing the
     * size of the WHOLE block (header included). The public pointer addresses
     * the payload right after the header.
     */
    class ZMallocSimple
    {
    public:
        // vtbl
        // methods

        /**
         * @brief Constructs the allocator with an empty pool.
         * @param lMaxNrFreeLinks Maximum number of tracked free links (0x800 on PC).
         * @param lAlignment      Size-header length in bytes (4 on PC).
         */
        ZMallocSimple(uint32_t lMaxNrFreeLinks = 0x800, uint32_t lAlignment = 4);
        ~ZMallocSimple();

        /**
         * @brief Registers a memory block as a free range.
         * @param pRamPtr  Start of the block.
         * @param lFreeSize Size of the block in bytes.
         */
        void AddBlock(char* pRamPtr, uint32_t lFreeSize);

        /**
         * @brief Unregisters a block previously passed to AddBlock().
         *
         * Only valid while the block is still entirely free (nothing was
         * allocated from it), so its free link still starts at pRamPtr.
         * @param pRamPtr Start of the block to remove.
         */
        void RemoveBlock(char* pRamPtr);

        /**
         * @brief Checks that everything allocated so far has been freed.
         * @return True if the free links again cover the whole registered pool.
         */
        bool CheckAllFreed();

        /**
         * @brief Sums the free space over all free links.
         * @return Total free bytes in the pool (fragmentation included).
         */
        uint32_t GetFreeTotal();

        /**
         * @brief Allocates memory from the pool (best-fit over the free links).
         * @param lSize       Requested payload size in bytes.
         * @param pBlockStart Out: start of the whole block (size header included).
         * @param pBlockEnd   Out: end of the whole block (size header included).
         * @return Pointer to the payload (block start + m_lAlignment),
         *         or nullptr when no free link can satisfy the request.
         */
        char* Alloc(uint32_t, char*&, char*&);

        /**
         * @brief Frees a pointer previously returned by Alloc().
         *
         * Coalesces the block with adjacent free links; allocates a new free
         * link when there are no adjacent ones.
         * @param pRam Payload pointer (nullptr is accepted and ignored).
         */
        void Free(char*);

        /**
         * @brief Shrinks an allocation in place (not supported here).
         * @return Always false.
         */
        bool Shrink(char*, uint32_t);

        /**
         * @brief Returns the payload capacity of an allocation.
         * @param pRam Payload pointer previously returned by Alloc().
         * @return Block size from the header minus the m_lAlignment header.
         */
        uint32_t AllocSize(const char*);

        /**
         * @brief Takes the next free link slot at the end of the used range.
         * @return Pointer to the new (uninitialized) link entry.
         */
        SFreeLink* AllocFreeLink();

        /**
         * @brief Releases a free link slot (swap-remove with the last slot),
         *        keeping used links packed at the front of m_pFreeLinks.
         * @param pLink Link entry to release.
         */
        void FreeFreeLink(SFreeLink*);

        // members (unlike PS2, this store is aligned)
        uint32_t m_lNrFreeLinks;        ///< Number of used entries in m_pFreeLinks.
        uint32_t m_lTotalSize;          ///< Total bytes registered via AddBlock().
        uint32_t m_lAlignment;          ///< Size-header length in bytes.
        uint32_t m_lMaxNrFreeLinks;     ///< Capacity of m_pFreeLinks.
        SFreeLink m_pFreeLinks[2048];   ///< Free link table (used part is [0, m_lNrFreeLinks)).
    };
    
    RE_VERIFY_OFFSET(ZMallocSimple, m_lAlignment, 0x8); // Verified in ctor
    RE_VERIFY_OFFSET(ZMallocSimple, m_lMaxNrFreeLinks, 0xC); // Verified in ctor
    RE_VERIFY_SIZE(ZMallocSimple, 0x4010); // 4 dwords + 2048 links; matches PC .data block (0x1004 dwords)
}
