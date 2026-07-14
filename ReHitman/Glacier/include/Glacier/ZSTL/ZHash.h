#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>
#include <cstring>
#include <type_traits>


namespace Glacier
{
    namespace Detail
    {
        inline uint32_t ZHashRotateLeft(uint32_t value, int shift)
        {
            shift &= 31;
            return (value << shift) | (value >> ((32 - shift) & 31));
        }

        /**
         * @brief Converts a C string key into the two 32-bit seeds used by ZPStrHash.
         *
         * @param pKey Pointer to the string pointer used as the hash key.
         * @param pSeed1 Receives the high seed accumulated with 8-bit rotates.
         * @param pSeed2 Receives the low seed accumulated with 9-bit rotates.
         *
         * @details This mirrors the original StrToI64 helper found in the console builds.
         * It hashes string contents rather than the pointer value so different pointers
         * containing the same text compare and hash identically.
         */
        inline void StrToI64(const char* const* pKey, unsigned int* pSeed1, unsigned int* pSeed2)
        {
            const unsigned char* p = reinterpret_cast<const unsigned char*>(*pKey);

            *pSeed1 = 0;
            *pSeed2 = 0;

            while (*p)
            {
                *pSeed1 = ZHashRotateLeft(*pSeed1 ^ *p, 8);
                *pSeed2 = ZHashRotateLeft(*pSeed2 ^ *p, 9);
                ++p;
            }
        }
    }

    /**
     * @brief Common base for Glacier open-addressed hash tables.
     *
     * ZHashBase stores table sizing state, occupancy bits, and shared probing helpers.
     * Concrete typed hashes provide the backing node array and implement SetSize(),
     * GetArray(), and NodeSize(). The hash table uses double hashing: hf1 selects the
     * initial bucket and hf2 returns an odd probe step so every bucket can be visited
     * when the table size is a power of two.
     */
    struct ZHashBase
    {
        // vtbl
        /**
         * @brief Resizes the concrete hash table storage.
         *
         * @param lSize New table size, expected to be a power of two.
         *
         * @details Implemented by concrete hashes because the node array element type
         * depends on the key/value template parameters.
         */
        virtual void SetSize(unsigned int) = 0;
        /** @brief Releases the occupancy bitset owned by the base class. */
        virtual ~ZHashBase();
        /** @brief Returns the concrete node array storage. */
        virtual const void* GetArray() = 0;
        /** @brief Returns the size in bytes of one concrete hash node. */
        virtual unsigned int NodeSize() = 0;

        // members
        /** @brief Bitset marking occupied table slots. One uint32_t stores 32 slots. */
        uint32_t* m_aTaken;
        /** @brief Current number of table slots. */
        uint32_t m_lSize;
        /** @brief Minimum table size allowed when shrinking. */
        uint32_t m_lMinSize;
        /** @brief Number of elements currently stored in the table. */
        uint32_t m_lCount;
        /** @brief Longest probe sequence observed/maintained by the concrete table. */
        uint32_t m_lLongestSequence;

        // methods
        /**
         * @brief Initializes shared hash table state.
         *
         * @param lMinSize Requested minimum size. Values below 32 are clamped to 32,
         * then rounded up to the next power of two.
         */
        explicit ZHashBase(uint32_t lMinSize);
        /**
         * @brief Computes the first bucket for a seed.
         *
         * @param iSeed Hash seed produced by the concrete key hashing logic.
         * @return Initial bucket index in the range [0, m_lSize).
         */
        uint32_t hf1(uint32_t iSeed) const;
        /**
         * @brief Computes the probe step for a seed.
         *
         * @param iSeed Hash seed produced by the concrete key hashing logic.
         * @return Odd probe step used by double hashing.
         */
        uint32_t hf2(uint32_t iSeed) const;
        /** @brief Shrinks the table by half when it is above its minimum size. */
        void Shrink();
        /** @brief Doubles the table size through the concrete SetSize() implementation. */
        void Grow();
        /** @brief Returns the number of elements currently stored in the table. */
        uint32_t Count() const;
        /**
         * @brief Clears the occupancy bit for a table slot.
         *
         * @param iIndex Slot index whose bit should be cleared in m_aTaken.
         */
        void ClearTaken(uint32_t iIndex);
        bool IsTaken(uint32_t iIndex) const;
        void SetTaken(uint32_t iIndex);
    };
    RE_VERIFY_SIZE(ZHashBase, 0x18);

    /**
     * @brief Key/value storage node used by ZHash.
     *
     * @tparam K Key type stored in the node.
     * @tparam V Value type associated with the key.
     *
     * @details The original binaries instantiate explicit constructors/destructors for
     * MYSTR-keyed nodes. The generic C++ member construction/destruction here provides
     * the same behavior: m_Key is constructed and destroyed according to K, then m_Value
     * according to V.
     */
    template <typename K, typename V>
    struct _SHashNode
    {
        /** @brief Hash key stored in this node. */
        K m_Key;
        /** @brief Value associated with m_Key. */
        V m_Value;
    };

    /**
     * @brief Typed open-addressed hash table used by Glacier containers.
     *
     * @tparam K Key type. The default IntValue() implementation supports integral and
     * pointer keys. Other key types should override IntValue() and Equals().
     * @tparam V Value type stored for each key.
     *
     * @details ZHash stores nodes in a flat array and tracks occupied slots with the
     * ZHashBase bitset. Insertion and lookup use double hashing: IntValue() produces two
     * seeds, ZHashBase::hf1() selects the initial slot, and ZHashBase::hf2() selects an
     * odd probe step. The table size is kept as a power of two so probing can wrap with
     * a bit mask. Keys are passed by pointer to match the original API and to avoid
     * copying large key objects while hashing or comparing.
     */
    template <typename K, typename V> struct ZHash : ZHashBase
    {
        // vtbl
        /** @brief Rebuilds the table with @p lSize slots and reinserts existing nodes. */
        void SetSize(unsigned int lSize) override;
        /** @brief Destroys all constructed nodes and releases the node array. */
        ~ZHash() override;
        /** @brief Returns the raw node array used by this hash table. */
        const void* GetArray() override;
        /** @brief Returns sizeof(_SHashNode<K, V>). */
        unsigned int NodeSize() override;
        /**
         * @brief Converts a key into the two hash seeds used by double hashing.
         *
         * @param pKey Pointer to the key to hash.
         * @param pSeed1 Receives the seed used by hf1().
         * @param pSeed2 Receives the seed used by hf2().
         */
        virtual void IntValue(const K* pKey, unsigned int* pSeed1, unsigned int* pSeed2);
        /** @brief Compares two keys for table equality. */
        virtual bool Equals(const K* pKey1, const K* pKey2);

        // methods
        /**
         * @brief Creates a hash table with at least @p lMinSize slots.
         *
         * @param lMinSize Minimum requested slot count. Values below 32 are clamped by
         * ZHashBase, then rounded up to a power of two.
         */
        explicit ZHash(uint32_t lMinSize);
        /** @brief Removes all entries and resets storage back to the minimum table size. */
        void Clear();
        /**
         * @brief Inserts or updates a key/value pair.
         *
         * @param pKey Pointer to the key to insert.
         * @param value Value to store for the key.
         * @param bMayGrow When true, the table may grow if probing cannot find a slot.
         * @return True if the value was inserted or updated; false only when growth is
         * disabled and no slot can be found.
         */
        bool Put(const K* pKey, const V& value, bool bMayGrow = true);
        /** @brief Convenience overload for inserting or updating a key passed by reference. */
        bool Put(const K& key, const V& value, bool bMayGrow = true) { return Put(&key, value, bMayGrow); }
        /** @brief Finds the node for @p pKey, or nullptr when the key is not present. */
        _SHashNode<K, V>* Find(const K* pKey);
        /** @brief Finds the node for @p pKey without updating lookup statistics. */
        const _SHashNode<K, V>* Find(const K* pKey) const;
        /** @brief Convenience overload for finding a key passed by reference. */
        _SHashNode<K, V>* Find(const K& key) { return Find(&key); }
        /** @brief Convenience const overload for finding a key passed by reference. */
        const _SHashNode<K, V>* Find(const K& key) const { return Find(&key); }
        /** @brief Returns a pointer to the value for @p pKey, or nullptr when missing. */
        V* Get(const K* pKey);
        /** @brief Returns a const pointer to the value for @p pKey, or nullptr when missing. */
        const V* Get(const K* pKey) const;
        /** @brief Convenience overload for retrieving a value by key reference. */
        V* Get(const K& key) { return Get(&key); }
        /** @brief Convenience const overload for retrieving a value by key reference. */
        const V* Get(const K& key) const { return Get(&key); }
        /** @brief Returns true when @p pKey is present in the table. */
        bool Contains(const K* pKey) const { return Find(pKey) != nullptr; }
        /** @brief Convenience overload for checking a key passed by reference. */
        bool Contains(const K& key) const { return Contains(&key); }
        /** @brief Returns the typed mutable node array. */
        _SHashNode<K, V>* GetTypedArray() { return m_pArray; }
        /** @brief Returns the typed read-only node array. */
        const _SHashNode<K, V>* GetTypedArray() const { return m_pArray; }
        /**
         * @brief Starts iteration and returns the next iteration index.
         *
         * @param ppKey Receives a pointer to the first occupied key.
         * @param ppValue Receives a pointer to the first occupied value when non-null.
         * @return Next index to pass to Next(), or 0 when the table is empty.
         */
        uintptr_t First(K** ppKey, V** ppValue);
        /** @brief Const overload of First() for read-only iteration. */
        uintptr_t First(const K** ppKey, const V** ppValue) const;
        /**
         * @brief Continues iteration from @p lIndex.
         *
         * @return Next index to pass to Next(), or 0 when there are no more entries.
         */
        uintptr_t Next(uint32_t lIndex, K** ppKey, V** ppValue);
        /** @brief Const overload of Next() for read-only iteration. */
        uintptr_t Next(uint32_t lIndex, const K** ppKey, const V** ppValue) const;
        /**
         * @brief Low-level iterator helper that advances @p pIndex in place.
         *
         * @return The next occupied node, or nullptr at the end of the table.
         */
        _SHashNode<K, V>* Next(uint32_t* pIndex);
        /** @brief Const low-level iterator helper that advances @p pIndex in place. */
        const _SHashNode<K, V>* Next(uint32_t* pIndex) const;
        /** @brief Removes @p pKey if it exists and shrinks the table when sparse. */
        void Remove(const K* pKey);
        /** @brief Convenience overload for removing a key passed by reference. */
        void Remove(const K& key) { Remove(&key); }

        // members
        /** @brief Flat node array with m_lSize entries. */
        _SHashNode<K, V>* m_pArray;
        /** @brief Exponential moving average of observed probe lengths. */
        float m_fLoad;
    };

    using ZIntHash = ZHash<int, unsigned int>;
    RE_VERIFY_SIZE(ZIntHash, 0x20);

    /**
     * @brief ZHash specialization for null-terminated C string keys.
     *
     * @tparam V Value type stored for each string key.
     *
     * @details The key type is always const char*. Unlike the generic pointer-key hash,
     * ZPStrHash hashes and compares pointed-to string contents. This makes lookups work
     * for distinct string pointers that contain equal text. The stored key remains a raw
     * const char* and is not copied or owned by the table.
     */
    template <typename V>
    struct ZPStrHash : ZHash<const char*, V>
    {
        /** @brief Creates a string-keyed hash table with at least @p lMinSize slots. */
        explicit ZPStrHash(uint32_t lMinSize);
        /** @brief Hashes string contents into the two seeds used by double hashing. */
        void IntValue(const char* const* pKey, unsigned int* pSeed1, unsigned int* pSeed2) override;
        /** @brief Compares string contents with strcmp semantics. */
        bool Equals(const char* const* pKey1, const char* const* pKey2) override;
    };

    template <typename K, typename V>
    ZHash<K, V>::ZHash(uint32_t lMinSize) : ZHashBase(lMinSize)
    {
        m_pArray = nullptr;
        m_fLoad = 0.0f;
        Clear();
    }

    template <typename K, typename V>
    ZHash<K, V>::~ZHash()
    {
        if (m_pArray)
        {
            for (uint32_t i = 0; i < m_lSize; ++i)
                m_pArray[i].~_SHashNode<K, V>();

            ZUniMemory::Free(m_pArray);
            m_pArray = nullptr;
        }
    }

    template <typename K, typename V>
    void ZHash<K, V>::SetSize(unsigned int lSize)
    {
        if (lSize < m_lCount)
            return;

        _SHashNode<K, V>* pOldArray = m_pArray;
        uint32_t* pOldTaken = m_aTaken;
        uint32_t lOldCount = m_lCount;
        uint32_t lOldSize = m_lSize;

        m_fLoad = 0.0f;
        m_lSize = lSize;
        m_lCount = 0;
        m_lLongestSequence = 0;

        m_pArray = static_cast<_SHashNode<K, V>*>(ZUniMemory::Allocate(sizeof(_SHashNode<K, V>) * m_lSize));
        for (uint32_t i = 0; i < m_lSize; ++i)
            new (&m_pArray[i]) _SHashNode<K, V>();

        m_aTaken = static_cast<uint32_t*>(ZUniMemory::Allocate(sizeof(uint32_t) * (m_lSize >> 5)));
        std::memset(m_aTaken, 0, sizeof(uint32_t) * (m_lSize >> 5));

        uint32_t iIndex = 0;
        while (lOldCount)
        {
            while ((pOldTaken[iIndex >> 5] & (1u << (iIndex & 31))) == 0)
                ++iIndex;

            Put(&pOldArray[iIndex].m_Key, pOldArray[iIndex].m_Value, false);
            ++iIndex;
            --lOldCount;
        }

        if (pOldArray)
        {
            for (uint32_t i = 0; i < lOldSize; ++i)
                pOldArray[i].~_SHashNode<K, V>();

            ZUniMemory::Free(pOldArray);
        }

        ZUniMemory::Free(pOldTaken);
    }

    template <typename K, typename V>
    const void* ZHash<K, V>::GetArray()
    {
        return m_pArray;
    }

    template <typename K, typename V>
    unsigned int ZHash<K, V>::NodeSize()
    {
        return sizeof(_SHashNode<K, V>);
    }

    template <typename K, typename V>
    void ZHash<K, V>::IntValue(const K* pKey, unsigned int* pSeed1, unsigned int* pSeed2)
    {
        static_assert(std::is_integral_v<K> || std::is_pointer_v<K>, "ZHash::IntValue requires an override for this key type");

        uintptr_t value = 0;
        if constexpr (std::is_pointer_v<K>)
            value = reinterpret_cast<uintptr_t>(*pKey);
        else
            value = static_cast<uintptr_t>(*pKey);

        *pSeed1 = static_cast<unsigned int>(value);
        *pSeed2 = static_cast<unsigned int>(value >> 16);
    }

    template <typename K, typename V>
    bool ZHash<K, V>::Equals(const K* pKey1, const K* pKey2)
    {
        return *pKey1 == *pKey2;
    }

    template <typename K, typename V>
    void ZHash<K, V>::Clear()
    {
        if (m_lSize != m_lMinSize)
        {
            if (m_lSize)
            {
                if (m_pArray)
                {
                    for (uint32_t i = 0; i < m_lSize; ++i)
                        m_pArray[i].~_SHashNode<K, V>();

                    ZUniMemory::Free(m_pArray);
                }

                ZUniMemory::Free(m_aTaken);
            }

            m_pArray = static_cast<_SHashNode<K, V>*>(ZUniMemory::Allocate(sizeof(_SHashNode<K, V>) * m_lMinSize));
            for (uint32_t i = 0; i < m_lMinSize; ++i)
                new (&m_pArray[i]) _SHashNode<K, V>();

            m_aTaken = static_cast<uint32_t*>(ZUniMemory::Allocate(sizeof(uint32_t) * (m_lMinSize >> 5)));
            m_lSize = m_lMinSize;
        }

        m_lCount = 0;
        m_lLongestSequence = 0;
        m_fLoad = 0.0f;

        if constexpr (std::is_trivially_copyable_v<_SHashNode<K, V>>)
        {
            std::memset(m_pArray, 0, sizeof(_SHashNode<K, V>) * m_lSize);
        }
        else
        {
            for (uint32_t i = 0; i < m_lSize; ++i)
                m_pArray[i] = _SHashNode<K, V>();
        }

        std::memset(m_aTaken, 0, sizeof(uint32_t) * (m_lSize >> 5));
    }

    template <typename K, typename V>
    bool ZHash<K, V>::Put(const K* pKey, const V& value, bool bMayGrow)
    {
        if (bMayGrow && m_lCount >= m_lSize)
            Grow();

        unsigned int iSeed1 = 0;
        unsigned int iSeed2 = 0;
        IntValue(pKey, &iSeed1, &iSeed2);

        uint32_t iIndex = hf1(iSeed1);
        uint32_t iStep = hf2(iSeed2);
        uint32_t lSequence = 0;

        if (auto* pFound = Find(pKey))
        {
            pFound->m_Value = value;
            return true;
        }

        while (IsTaken(iIndex) && !Equals(&m_pArray[iIndex].m_Key, pKey))
        {
            iIndex = (iIndex + iStep) & (m_lSize - 1);

            if (m_lSize < ++lSequence)
            {
                if (!bMayGrow)
                    return false;

                Grow();
                return Put(pKey, value, true);
            }
        }

        if (m_lLongestSequence < lSequence)
            m_lLongestSequence = lSequence;

        m_fLoad = m_fLoad * 0.9f + static_cast<float>(lSequence) * 0.1f;
        m_pArray[iIndex].m_Key = *pKey;
        m_pArray[iIndex].m_Value = value;

        if (!IsTaken(iIndex))
        {
            SetTaken(iIndex);
            ++m_lCount;
        }

        return true;
    }

    template <typename K, typename V>
    _SHashNode<K, V>* ZHash<K, V>::Find(const K* pKey)
    {
        unsigned int iSeed1 = 0;
        unsigned int iSeed2 = 0;
        IntValue(pKey, &iSeed1, &iSeed2);

        uint32_t iIndex = hf1(iSeed1);
        uint32_t iStep = hf2(iSeed2);
        uint32_t lSequence = 0;

        while (!IsTaken(iIndex) || !Equals(&m_pArray[iIndex].m_Key, pKey))
        {
            iIndex = (iIndex + iStep) & (m_lSize - 1);

            if (m_lLongestSequence < lSequence)
                return nullptr;

            ++lSequence;
        }

        if (IsTaken(iIndex))
        {
            m_fLoad = m_fLoad * 0.9f + static_cast<float>(lSequence) * 0.1f;
            return &m_pArray[iIndex];
        }

        return nullptr;
    }

    template <typename K, typename V>
    V* ZHash<K, V>::Get(const K* pKey)
    {
        auto* pNode = Find(pKey);
        return pNode ? &pNode->m_Value : nullptr;
    }

    template <typename K, typename V>
    const _SHashNode<K, V>* ZHash<K, V>::Find(const K* pKey) const
    {
        unsigned int iSeed1 = 0;
        unsigned int iSeed2 = 0;
        const_cast<ZHash<K, V>*>(this)->IntValue(pKey, &iSeed1, &iSeed2);

        uint32_t iIndex = hf1(iSeed1);
        uint32_t iStep = hf2(iSeed2);
        uint32_t lSequence = 0;

        while (!IsTaken(iIndex) || !const_cast<ZHash<K, V>*>(this)->Equals(&m_pArray[iIndex].m_Key, pKey))
        {
            iIndex = (iIndex + iStep) & (m_lSize - 1);

            if (m_lLongestSequence < lSequence)
                return nullptr;

            ++lSequence;
        }

        return IsTaken(iIndex) ? &m_pArray[iIndex] : nullptr;
    }

    template <typename K, typename V>
    const V* ZHash<K, V>::Get(const K* pKey) const
    {
        auto* pNode = Find(pKey);
        return pNode ? &pNode->m_Value : nullptr;
    }

    template <typename K, typename V>
    uintptr_t ZHash<K, V>::First(K** ppKey, V** ppValue)
    {
        return Next(0, ppKey, ppValue);
    }

    template <typename K, typename V>
    uintptr_t ZHash<K, V>::First(const K** ppKey, const V** ppValue) const
    {
        return Next(0, ppKey, ppValue);
    }

    template <typename K, typename V>
    uintptr_t ZHash<K, V>::Next(uint32_t lIndex, K** ppKey, V** ppValue)
    {
        uint32_t iIndex = lIndex;
        auto* pNode = Next(&iIndex);

        if (!pNode)
            return 0;

        *ppKey = &pNode->m_Key;
        if (ppValue)
            *ppValue = &pNode->m_Value;

        return iIndex;
    }

    template <typename K, typename V>
    uintptr_t ZHash<K, V>::Next(uint32_t lIndex, const K** ppKey, const V** ppValue) const
    {
        uint32_t iIndex = lIndex;
        auto* pNode = Next(&iIndex);

        if (!pNode)
            return 0;

        *ppKey = &pNode->m_Key;
        if (ppValue)
            *ppValue = &pNode->m_Value;

        return iIndex;
    }

    template <typename K, typename V>
    _SHashNode<K, V>* ZHash<K, V>::Next(uint32_t* pIndex)
    {
        while (*pIndex < m_lSize)
        {
            if (IsTaken(*pIndex))
                return &m_pArray[(*pIndex)++];

            ++*pIndex;
        }

        return nullptr;
    }

    template <typename K, typename V>
    const _SHashNode<K, V>* ZHash<K, V>::Next(uint32_t* pIndex) const
    {
        while (*pIndex < m_lSize)
        {
            if (IsTaken(*pIndex))
                return &m_pArray[(*pIndex)++];

            ++*pIndex;
        }

        return nullptr;
    }

    template <typename K, typename V>
    void ZHash<K, V>::Remove(const K* pKey)
    {
        auto* pNode = Find(pKey);
        if (!pNode)
            return;

        pNode->m_Key = K();
        ClearTaken(static_cast<uint32_t>(pNode - m_pArray));

        --m_lCount;
        if (!m_lCount)
            m_lLongestSequence = 0;

        if (m_lCount < (m_lSize >> 2))
            Shrink();
    }

    template <typename V>
    ZPStrHash<V>::ZPStrHash(uint32_t lMinSize) : ZHash<const char*, V>(lMinSize)
    {
    }

    template <typename V>
    void ZPStrHash<V>::IntValue(const char* const* pKey, unsigned int* pSeed1, unsigned int* pSeed2)
    {
        Detail::StrToI64(pKey, pSeed1, pSeed2);
    }

    template <typename V>
    bool ZPStrHash<V>::Equals(const char* const* pKey1, const char* const* pKey2)
    {
        return std::strcmp(*pKey1, *pKey2) == 0;
    }
}
