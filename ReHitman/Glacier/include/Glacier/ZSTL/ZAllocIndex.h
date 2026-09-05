#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @class ZAllocIndex
     * @brief Index allocator with per-slot generation counters and an embedded free-list.
     *
     * @details Provides O(1) allocation and deallocation of slot indices in the range
     * [0, 2^lNrIndexBits). Every slot owns one uint32_t word in m_pArray that packs
     * two fields: the low lNrIndexBits bits store a free-list link (index of the next
     * free slot, or the slot's own index for the last free slot / while allocated),
     * the remaining high bits store a generation counter. The free-list head lives in
     * m_lFirstFreeIndex (0xFFFFFFFF when the pool is full).
     *
     * New() pops the free-list head, increments its generation and returns the bare
     * slot index (generation bits are stripped from the return value). A slot whose
     * generation wraps to zero is dropped from the pool instead of being handed out,
     * so a zero generation is never observed for an allocated slot. Free() masks the
     * incoming value down to the slot index (generation bits of stale handles are
     * ignored), keeps the current generation and pushes the slot back onto the
     * free-list head (LIFO reuse).
     *
     * Callers (e.g. CRefTab) combine the returned index with the generation stored in
     * m_pArray[slot] to build handles that detect stale references (ABA problem).
     *
     * @note Total element capacity is always a power of two: 2^lNrIndexBits.
     */
    class ZAllocIndex
    {
    public:
        // vtbl
        virtual ~ZAllocIndex();

        // methods
        /**
         * @brief Constructs the index allocator.
         * @param lNrIndexBits Bit width allocated for indexing (sets capacity to $2^{\text{lNrIndexBits}}$).
         */
        ZAllocIndex(uint32_t lNrIndexBits);

        /**
         * @brief Resets allocator state and rebuilds the free-list hierarchy.
         */
        void Clean();

        /**
         * @brief Allocates the head slot of the free-list and bumps its generation counter.
         * @return Bare slot index in [0, 2^lNrIndexBits); generation bits are stripped.
         * @note Triggers a ZASSERT if maximum active capacity is reached.
         */
        uint32_t New();

        /**
         * @brief Returns a slot back to the free-list head, preserving its generation counter.
         * @param lIndex Value identifying the slot; only the low lNrIndexBits bits are used.
         * @note Triggers a ZASSERT when no slot is currently allocated.
         */
        void Free(uint32_t lIndex);

        /**
         * @brief Gets the total count of currently allocated indices.
         * @return Number of active slots.
         */
        uint32_t Count() const;

        // members
        uint32_t* m_pArray; ///< Buffer storing free-list links and generation counters.
        int32_t m_lNrElements; ///< Total element capacity (always a power of two).
        uint32_t m_lFirstFreeIndex; ///< Head index of the free-list (0xFFFFFFFF if empty).
        uint32_t m_lActiveCount; ///< Total number of currently active/allocated slots.
    };
}