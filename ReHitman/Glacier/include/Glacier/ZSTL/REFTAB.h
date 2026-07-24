#pragma once

#include <cstdint>
#include <type_traits>
#include <Glacier/Glacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
    /**
     * @struct TabBlk
     * @brief Internal memory block header for REFTAB storage.
     *        Each block manages a contiguous array of raw data elements immediately 
     *        following this header in memory. Blocks are chained into a doubly-linked list.
     */
    struct TabBlk
    {
        TabBlk *_Prev; ///< Pointer to the previous memory block.
        TabBlk *_Next; ///< Pointer to the next memory block.
        int _Cou;      ///< Current active element counter inside this specific block.
        int _Pad;      ///< 0x10 Alignment padding
    };

    /**
     * @struct RefRun
     * @brief Opaque iterator/cursor state utilized for REFTAB traversal and mutations.
     *        Keeps track of the current block, position offset, and direction during forward or backward iterations.
     */
    struct RefRun
    {
        TabBlk *_RunPtr; ///< Pointer to the currently iterated memory block.
        int _RunCou;     ///< Current element offset (in bytes) within the block.
        int _RunDir;     ///< Iteration direction flag.

        /**
         * @brief Evaluates whether the iterator is pointing to a valid block.
         */
        explicit operator bool() const { return _RunPtr != nullptr; }
    };

    /**
     * @class REFTAB
     * @brief A fixed-size chunked allocator container (Unrolled Linked List) from Glacier Engine.
     *
     * @details
     * REFTAB is a performance-oriented hybrid container optimized for cache locality and rapid element recycling.
     * Instead of allocating memory for every single element (like std::list) or shifting the entire array 
     * on erase operations (like std::vector), REFTAB allocates large chunks of memory called **TabBlk**.
     * * ### Key Characteristics:
     * - **Chunked Allocations:** Contains an internal doubly-linked list of pages (@ref TabBlk). Each page holds a fixed 
     * amount of elements calculated via `BlkSize`.
     * - **Fast O(1) Erasure:** When an element is deleted via @ref RunDelRef, REFTAB avoids memory shifting. 
     * Instead, it copies the **very last element** of the entire container into the slot of the deleted element 
     * using a raw `memcpy`.
     * - **Binary Compatibility:** Strictly maintains a structure size of `0x1C` (28 bytes) to remain fully compatible
     * with the original game binary (*Hitman: Blood Money*).
     */
    class REFTAB
    {
    public: 
        // members
        TabBlk* TabFirstPtr; ///< Head of the memory block linked list.
        TabBlk* TabBlockPtr; ///< Tail of the memory block linked list (last allocated block).
        int m_lRefsPrBlk;    ///< Maximum capacity of elements per single block.
        int BlkSize;         ///< Total data capacity size per block in bytes (`m_lRefsPrBlk * EleSize`).
        int EleCount;        ///< Total number of active elements across all blocks.
        int EleSize;         ///< Total size of one element in bytes (`pUserData + 1`).

        // methods
        /**
         * @brief Constructs a new REFTAB container.
         * @param pPoolSize Maximum number of elements allowed inside one chunk/block.
         * @param pUserData Inner element size metric (internally incremented by 1 to form EleSize).
         */
        REFTAB(int pPoolSize, int pUserData);

        // factory
        static REFTAB* MakeReftab(int pPoolSize, int pUserData);
        static void DeleteReftab(REFTAB* pRefTab);

        // vtbl
        virtual ~REFTAB();
        virtual uint32_t* Add(uint32_t);
        virtual uint32_t* AddUnique(uint32_t);
        virtual void Clear();
        virtual void ClearThis();
        virtual int Count();
        virtual uint32_t Size();
        virtual uint32_t GetEleSize();
        virtual uint32_t PoolSize();
        virtual void DelRefPtr(uint32_t*);
        virtual bool Exists(uint32_t*);
        virtual bool Exists(uint32_t) const;
        virtual uint32_t* Find(uint32_t) const;
        virtual uint32_t GetRefNr(int) const;
        virtual uint32_t* GetRefPtrNr(int);
        virtual uint32_t GetIndex(uint32_t) const;
        virtual void Remove(uint32_t);
        virtual bool RemoveIfExists(uint32_t);
        virtual void RunDelRef(RefRun *);
        virtual void RunInitNxtRef(RefRun *) const;
        virtual void RunInitNxtRef(RefRun *);
        virtual void RunInitPrevRef(RefRun *) const;
        virtual void RunInitPrevRef(RefRun *);
        virtual uint32_t RunNxtRef(RefRun *) const;
        virtual uint32_t RunNxtRef(RefRun *);
        virtual const uint32_t* RunNxtRefPtr(RefRun *) const;
        virtual uint32_t* RunNxtRefPtr(RefRun *);
        virtual uint32_t RunPrevRef(RefRun *) const;
        virtual uint32_t RunPrevRef(RefRun *);
        virtual const uint32_t* RunPrevRefPtr(RefRun *) const;
        virtual uint32_t* RunPrevRefPtr(RefRun *);
        virtual uint32_t operator[](int) const;
        virtual const uint32_t* RunToRefPtr(RefRun *) const;
        virtual void DeleteBlock(TabBlk *);
        virtual TabBlk* NewBlock(void);

        // STL Iterators
        template <typename T = uint32_t>
        class Iterator 
        {
        private:
            REFTAB* m_pContainer;
            RefRun  m_Run;
            uint32_t* m_pCurrentPtr;

            void Advance() 
            {
                m_pCurrentPtr = m_pContainer->RunNxtRefPtr(&m_Run);
            }

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = T;
            using difference_type   = std::ptrdiff_t;
            using pointer           = T*;
            using reference         = const T&;

            Iterator(REFTAB* container, bool isEnd) : m_pContainer(container) 
            {
                if (isEnd || !container || container->EleCount == 0) 
                {
                    m_Run._RunPtr = nullptr;
                    m_Run._RunCou = 0;
                    m_Run._RunDir = 0;
                    m_pCurrentPtr = nullptr;
                } 
                else 
                {
                    m_pContainer->RunInitNxtRef(&m_Run);
                    m_pCurrentPtr = m_pContainer->RunNxtRefPtr(&m_Run);
                }
            }

            value_type operator*() const 
            { 
                if constexpr (std::is_pointer_v<value_type>)
                {
                    return reinterpret_cast<value_type>(*m_pCurrentPtr); 
                }
                else
                {
                    return static_cast<value_type>(*m_pCurrentPtr);
                }
            }

            value_type operator->() const 
            { 
                if constexpr (std::is_pointer_v<value_type>)
                {
                    return reinterpret_cast<value_type>(*m_pCurrentPtr); 
                }
                else
                {
                    return static_cast<value_type>(*m_pCurrentPtr);
                }
            }

            Iterator& operator++() 
            {
                Advance();
                return *this;
            }

            Iterator operator++(int) 
            {
                Iterator tmp = *this;
                Advance();
                return tmp;
            }

            bool operator==(const Iterator& other) const 
            {
                if (!m_pCurrentPtr && !other.m_pCurrentPtr) 
                    return true;
                
                return m_pCurrentPtr == other.m_pCurrentPtr;
            }

            bool operator!=(const Iterator& other) const 
            {
                return !(*this == other);
            }
        };

        template <typename T>
        class TypedView
        {
        public:
            explicit TypedView(REFTAB* container) : m_pContainer(container) {}

            Iterator<T> begin() { return Iterator<T>(m_pContainer, false); }
            Iterator<T> end() { return Iterator<T>(m_pContainer, true); }

        private:
            REFTAB* m_pContainer;
        };

        template <typename T>
        TypedView<T> As()
        {
            return TypedView<T>(this);
        }

        template <typename T>
        TypedView<T> As() const
        {
            return TypedView<T>(const_cast<REFTAB*>(this));
        }
        
        Iterator<uint32_t> begin() { return Iterator<uint32_t>(this, false); }
        Iterator<uint32_t> end()   { return Iterator<uint32_t>(this, true); }

    protected:
        void MakeDirty();
    };
    RE_VERIFY_SIZE(REFTAB, 0x1C);
}
