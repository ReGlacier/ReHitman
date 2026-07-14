#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/GlacierFWD.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Single node stored by ZRefAlloc chains.
     *
     * Each link stores one reference value and the index of the next link in the chain.
     * Links are addressed by array index rather than pointer so chains can be serialized
     * compactly and moved together with their owning ZRefAlloc block.
     */
    struct SRefLink
    {
        /** @brief Reference value stored in this chain node. */
        unsigned int m_rRef;
        /** @brief Next link index, REFCHAIN_END for chain end, or flagged next-free index while unused. */
        unsigned int m_lNext;
    };
    RE_VERIFY_SIZE(SRefLink, 0x8);

    /**
     * @brief Fixed-size allocator for compact chains of reference values.
     *
     * ZRefAlloc owns a static array of SRefLink nodes and manages them through an index
     * based free-list. It is used by sound/runtime systems that need to attach a small
     * chain of references to another object without allocating individual heap nodes.
     *
     * Index 0 is reserved as an invalid/null index. A valid chain starts at an index in
     * the range [FIRST_REF_LINK, LAST_REF_LINK]. Chain links end with REFCHAIN_END
     * (0x80000000). Free-list entries store their next free index with FREE_LINK_FLAG set
     * in the high bit, matching the original PC/Xbox layout.
     */
    struct ZRefAlloc
    {
        /** @brief Marker stored in SRefLink::m_lNext for the end of a live chain. */
        static constexpr uint32_t REFCHAIN_END = 0x80000000u;
        /** @brief High-bit marker used on next-free indices in the free-list. */
        static constexpr uint32_t FREE_LINK_FLAG = 0x80000000u;
        /** @brief Number of link slots in this layout, including unused slot 0. */
        static constexpr uint32_t MAX_NUM_REFS = 2248u; //PS2 - 2048; PC, XBox(MiniNinjas) - 2248
        /** @brief First usable link index. Index 0 is reserved as null/invalid. */
        static constexpr uint32_t FIRST_REF_LINK = 1u;
        /** @brief Last usable link index in the static link array. */
        static constexpr uint32_t LAST_REF_LINK = MAX_NUM_REFS - 1u;

        /** @brief Number of links currently allocated into live chains. */
        int m_lNumLinks;
        /** @brief Index of the first free link, or 0 when the free-list is exhausted. */
        int m_lNextFree;
        /** @brief Static link storage. Slot 0 is reserved; slots 1..LAST_REF_LINK are allocatable. */
        SRefLink m_Links[MAX_NUM_REFS];

        /** @brief Constructs the allocator and initializes the free-list. */
        ZRefAlloc();

        /** @brief Resets all links and rebuilds the free-list. */
        void Initialize();
        /** @brief Returns true when no links are currently allocated. */
        bool IsEmpty() const;
        /**
         * @brief Allocates one link from the free-list.
         *
         * @return Allocated link index in the range [FIRST_REF_LINK, LAST_REF_LINK].
         */
        uint32_t AllocRef();
        /**
         * @brief Returns one allocated link to the free-list.
         *
         * @param lIndex Link index to free. Must not be 0 or REFCHAIN_END.
         */
        void FreeRef(uint32_t lIndex);
        /**
         * @brief Adds a reference value to a chain.
         *
         * @param lChainEntry Current chain head/index, 0 for a new chain, or REFCHAIN_END.
         * @param rValue Reference value to store.
         * @return Chain head index. Existing non-empty chains keep their original head;
         * new chains return the newly allocated link index.
         *
         * @details When adding to an existing chain, the new link is inserted directly
         * after the head link, matching the original implementation.
         */
        uint32_t AddToChain(uint32_t lChainEntry, uint32_t rValue);
        /**
         * @brief Frees every link in a chain.
         *
         * @param lChainIdx Chain head index, 0, or REFCHAIN_END.
         */
        void FreeChain(uint32_t lChainIdx);
        /** @brief Returns the link for @p lIndex, or nullptr for 0/REFCHAIN_END. */
        SRefLink* GetLink(uint32_t lIndex);
        /** @brief Const overload of GetLink(). */
        const SRefLink* GetLink(uint32_t lIndex) const;
        /** @brief Counts live links reachable from a chain head. */
        uint32_t GetChainLength(uint32_t lChainIdx) const;
        /**
         * @brief Serializes a reference chain.
         *
         * @warning Not implemented yet. The required serializer token/exchange helpers
         * have not been fully reversed.
         */
        void SaveRefChain(IOutputSerializerStream* pStream, uint32_t lChainIdx);
        /**
         * @brief Deserializes a reference chain.
         *
         * @warning Not implemented yet. The required serializer token/exchange helpers
         * have not been fully reversed.
         */
        uint32_t LoadRefChain(IInputSerializerStream* pStream);
    };
    RE_VERIFY_SIZE(ZRefAlloc, 0x4648);
}
