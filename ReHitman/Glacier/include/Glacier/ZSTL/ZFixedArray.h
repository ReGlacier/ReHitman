#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @class ZFixedArray
     * @brief Fixed-capacity inline pool of elements with an embedded free-list.
     *
     * @tparam T Element type stored inline.
     * @tparam CAPACITY Maximum number of elements.
     *
     * @details ZFixedArray owns no heap memory: all CAPACITY elements live inside the
     * inline m_Array storage. Slots that are not in use are chained into a singly
     * linked free-list: the first two bytes of every free slot store the index of the
     * next free slot, m_lFirstFreeEntry points at the list head and 0xFFFF marks the
     * end of the list.
     *
     * Add() pops the head of the free-list, default-constructs the element in place
     * (placement new) and returns a pointer to it, or nullptr when the pool is
     * exhausted. Remove() pushes the slot back onto the free-list head, so freed
     * slots are reused in LIFO order; the element destructor is NOT invoked. Both
     * operations are O(1).
     *
     * GetIndex()/GetEntry() convert between element pointers and slot indices,
     * Validate() reports whether a pointer refers to a currently allocated slot
     * (i.e. not present in the free-list), Reset() re-initializes the pool and
     * links every slot back into the free-list.
     */
    template <typename T, size_t CAPACITY>
    class ZFixedArray
    {
    public:
        // methods
        /**
         * @brief Initializes the pool; equivalent to a fresh Reset().
         */
        ZFixedArray()
        {
            Reset();
        }

        /**
         * @brief Allocates a slot from the free-list and default-constructs the element in place.
         * @return Pointer to the new element, or nullptr when all CAPACITY slots are in use.
         */
        T* Add()
        {
            if (m_lFirstFreeEntry == 0xFFFF)
                return nullptr;

            T* pEntry = reinterpret_cast<T*>(&m_Array[sizeof(T) * m_lFirstFreeEntry]);
            m_lFirstFreeEntry = *reinterpret_cast<uint16_t*>(pEntry);
            znew_placement<T>(pEntry);
            ++m_lCount;
            return pEntry;
        }

        /**
         * @brief Returns a slot to the free-list (becomes the new list head).
         * @param pEntry Element previously obtained from Add().
         * @note The element destructor is not called.
         */
        void Remove(T* pEntry)
        {
            --m_lCount;
            *reinterpret_cast<uint16_t*>(pEntry) = m_lFirstFreeEntry;
            m_lFirstFreeEntry = static_cast<uint16_t>((reinterpret_cast<char*>(pEntry) - reinterpret_cast<char*>(m_Array)) / sizeof(T));
        }

        /**
         * @brief Gets the total slot capacity of the pool.
         * @return CAPACITY.
         */
        uint32_t TotalNrEntries() const
        {
            return CAPACITY;
        }

        /**
         * @brief Gets the number of currently allocated slots.
         * @return Count of live elements.
         */
        uint16_t Count() const
        {
            return m_lCount;
        }

        /**
         * @brief Checks whether a pointer refers to an allocated (non-free) slot.
         * @param pEntry Pointer to a slot inside the pool.
         * @return true when the slot is currently allocated, false when it sits in the free-list.
         */
        bool Validate(const T* pEntry) const
        {
            const uint16_t lIndex = GetIndex(pEntry);

            for (uint16_t i = m_lFirstFreeEntry; i != 0xFFFF; i = *reinterpret_cast<const uint16_t*>(&m_Array[sizeof(T) * i]))
            {
                if (lIndex == i)
                    return false;
            }

            return true;
        }

        /**
         * @brief Converts an element pointer into its slot index.
         * @param pEntry Pointer to a slot inside the pool (must be exactly slot-aligned).
         * @return Zero-based slot index.
         */
        uint16_t GetIndex(const T* pEntry) const
        {
            ZASSERT(!((reinterpret_cast<const char*>(pEntry) - reinterpret_cast<const char*>(m_Array)) % sizeof(T)));
            const uint16_t lIndex = static_cast<uint16_t>((reinterpret_cast<const char*>(pEntry) - reinterpret_cast<const char*>(m_Array)) / sizeof(T));
            ZASSERT(GetEntry(lIndex) == pEntry);
            return lIndex;
        }

        /**
         * @brief Converts a slot index into an element pointer.
         * @param lIndex Zero-based slot index (must be less than CAPACITY).
         * @return Pointer to the slot storage.
         */
        T* GetEntry(uint16_t lIndex) const
        {
            ZASSERT(lIndex < CAPACITY);
            return fuck_cast<T>(&m_Array[sizeof(T) * lIndex]);
        }

        /**
         * @brief Drops all elements and links every slot back into the free-list.
         */
        void Reset()
        {
            m_lCount = 0;
            m_lFirstFreeEntry = 0;

            uint16_t* pEntry = reinterpret_cast<uint16_t*>(m_Array);
            for (size_t i = 0; i != CAPACITY - 1; ++i)
            {
                *pEntry = static_cast<uint16_t>(i + 1);
                pEntry = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(pEntry) + sizeof(T));
            }
            *pEntry = 0xFFFF;
        }

        // members
        uint8_t m_Array[sizeof(T) * CAPACITY]; ///< Inline slot storage; free slots hold free-list links in their first two bytes.
        uint16_t m_lCount; ///< Number of currently allocated slots.
        uint16_t m_lFirstFreeEntry; ///< Head index of the free-list (0xFFFF when the pool is full).
    };
}
