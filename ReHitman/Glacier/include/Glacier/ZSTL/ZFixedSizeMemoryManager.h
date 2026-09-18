#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniAssert.h>
#include <cstdint>


namespace Glacier
{
	/**
	 * @class ZFixedSizeMemoryManagerBase
	 * @brief A low-level, high-performance fixed-size block memory manager.
	 * 
	 * This base class manages a contiguous pool of memory divided into fixed-size blocks.
	 * It features dynamic validation of free blocks and incorporates an generation-shifting
	 * ID mechanism embedded in a reference handle system (@c rRef) to mitigate Use-After-Free (UAF) 
	 * vulnerabilities and dangling pointers.
	 * 
	 * @note This class deals with raw @c void* memory. For type-safe interactions, 
	 *       use the derived template wrapper @ref ZFixedSizeMemoryManager.
	 */
	struct ZFixedSizeMemoryManagerBase
	{
		// types
		struct ZFreeBlock
		{
			struct ZFreeBlock* m_Next;

			ZFreeBlock() : m_Next{nullptr} {}
			ZFreeBlock(ZFreeBlock* pNext) : m_Next(pNext) {}
		};

		struct Iterator
		{
			// members
			const ZFixedSizeMemoryManagerBase* m_MemoryManager;
			uint32_t m_Index;

			// methods
			Iterator()
				: m_MemoryManager(nullptr), m_Index(0)
			{
			}

			Iterator(const ZFixedSizeMemoryManagerBase* pMemMgmt, uint32_t iIndex)
				: m_MemoryManager(pMemMgmt), m_Index(iIndex)
			{
				MoveToNext();
			}

			Iterator& operator++()
			{
				ZASSERT(m_MemoryManager != nullptr);
				if (m_Index < m_MemoryManager->m_NumberOfBlocks)
				{
					++m_Index;
					MoveToNext();
				}

				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				ZASSERT(m_MemoryManager == other.m_MemoryManager);
				return m_Index != other.m_Index;
			}

			void MoveToNext()
			{
				ZASSERT(m_MemoryManager != nullptr);
				while (m_Index < m_MemoryManager->m_NumberOfBlocks && m_MemoryManager->m_ID[m_Index] < 0)
				{
					++m_Index;
				}
			}

			void* Get() const
			{
				ZASSERT(m_MemoryManager != nullptr);
            	return m_MemoryManager->Index2Address(static_cast<int>(m_Index));
			}
		};

		// methods
		ZFixedSizeMemoryManagerBase(int block_size, uint32_t number_of_blocks);
		~ZFixedSizeMemoryManagerBase();

		void ValidateFreeList();
		void* REF2Ptr(uint32_t rRef) const;
		uint32_t REF2Index(uint32_t rRef) const;
		uint16_t REF2ID(uint32_t rRef) const;
		uint32_t Ptr2REF(const void* pPtr) const;

		bool IsAllocated(void* pPtr) const;
		void* Index2Address(int iIndex) const;
		int16_t GetID(const void* pPtr) const;
		uint32_t GetCapacity() const;
		void* GetBufferPtr() const;
		void Free(void* pPtr);
		uint32_t Count() const;
		void* AllocDirect(uint32_t rRef);
		void* Alloc();
		uint32_t Address2Index(const void* pAddr) const;

		// members
		bool m_FreeListValid;
		RE_ADD_PADDING(3);
		const uint32_t m_BlockSize;
		const uint32_t m_NumberOfBlocks;
		char* m_Buffer;
		int16_t *m_ID;
		ZFreeBlock* m_FirstFreeBlock;
		ZFreeBlock* m_LastFreeBlock;
	};
	RE_VERIFY_SIZE(ZFixedSizeMemoryManagerBase, 0x1C);

	/**
	 * @class ZFixedSizeMemoryManager
	 * @tparam T The type of objects to be allocated within the fixed-size memory pool.
	 * @brief A type-safe facade over the fixed-size memory manager base.
	 * 
	 * Automatically calculates the block size required for type @c T and provides strongly-typed 
	 * wrappers for allocation, deallocation, reference-to-pointer lookups, and pool iteration.
	 */
	template <typename T>
	class ZFixedSizeMemoryManager : public ZFixedSizeMemoryManagerBase 
	{
	public:
		// types
		/**
		 * @class Iterator
		 * @brief A type-safe STL-like iterator for traversing active blocks within the pool.
		 * 
		 * Inherits from the base iterator to automatically skip unallocated or invalid slots,
		 * providing direct access to objects of type @c T* via the dereference operators.
		 */
		class Iterator : public ZFixedSizeMemoryManagerBase::Iterator
		{
		public:
			Iterator() = default;
			Iterator(const Iterator& other) : ZFixedSizeMemoryManagerBase::Iterator(other) {}
			
			Iterator(const ZFixedSizeMemoryManagerBase* pManager, uint32_t index)
				: ZFixedSizeMemoryManagerBase::Iterator(pManager, index) 
			{
			}

			T* operator*() const
			{
				return static_cast<T*>(Get());
			}

			T* operator->() const
			{
				return static_cast<T*>(Get());
			}
		};

		// methods
		ZFixedSizeMemoryManager(uint32_t numberOfBlocks)
			: ZFixedSizeMemoryManagerBase(sizeof(T), numberOfBlocks) 
		{
		}

		~ZFixedSizeMemoryManager() = default;

		T* Alloc()
		{
			return reinterpret_cast<T*>(ZFixedSizeMemoryManagerBase::Alloc());
		}

		T* AllocDirect(uint32_t rRef)
		{
			return reinterpret_cast<T*>(ZFixedSizeMemoryManagerBase::AllocDirect(rRef));
		}

		void Free(T* pPtr)
		{
			ZFixedSizeMemoryManagerBase::Free(pPtr);
		}

		T* REF2Ptr(uint32_t rRef) const
		{
			return reinterpret_cast<T*>(ZFixedSizeMemoryManagerBase::REF2Ptr(rRef));
		}

		T* Index2Address(uint32_t index) const
		{
			return reinterpret_cast<T*>(ZFixedSizeMemoryManagerBase::Index2Address(index));
		}

		T* GetBufferPtr() const
		{
			return reinterpret_cast<T*>(ZFixedSizeMemoryManagerBase::GetBufferPtr());
		}

		Iterator Begin() const
		{
			return Iterator(this, 0);
		}

		Iterator End() const
		{
			return Iterator(this, m_NumberOfBlocks);
		}
	};
}
