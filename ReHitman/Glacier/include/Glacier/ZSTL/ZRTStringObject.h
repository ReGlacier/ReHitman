#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZList.h>
#include <Glacier/ZSTL/ZUInt24.h>
#include <cstdint>
#include <new>


namespace Glacier
{
	// fwds
	/** Interned string allocation stored inside ZStringMemoryManager. */
	struct ZRTStringObject;

	/**
	 * Fixed-size memory manager used by ZRTString for interned string objects.
	 *
	 * The manager stores allocated string objects and free blocks in one linear
	 * buffer. Allocated strings are indexed through a 4096-bucket hash table.
	 */
	struct ZStringMemoryManager
	{
		// fwds
		/** Free block header stored in the managed string buffer. */
		struct ZFreeHeader;

		/** Free block footer used to find the previous free block in memory. */
		struct ZFreeFooter;

		/** Common block header stored before both allocated and free blocks. */
		struct ZHeader
		{
			/** Packed free/allocated size and allocated-object reference counter. */
			struct ZRefAndSize
			{
				// methods
				/** Constructs an end-marker header value. */
				ZRefAndSize();

				/** Constructs a free or allocated header with the given block size. */
				ZRefAndSize(bool, uint32_t);

				/** Sets the 15-bit reference counter for an allocated block. */
				void SetRefCounter(uint32_t);

				/** Marks the block as free and stores its 31-bit size. */
				void SetFreeSize(uint32_t);

				/** Marks the block as allocated and stores its 16-bit size. */
				void SetAllocatedSize(uint32_t);

				/** Changes the size of an existing free block. */
				void Resize(uint32_t);

				/** Returns true when the high free-bit is set. */
				bool IsFree() const;

				/** Returns true when this header is the buffer terminator marker. */
				bool IsEnd() const;

				/** Returns either the free size or allocated size depending on state. */
				uint32_t GetSize() const;

				/** Returns the reference counter of an allocated block. */
				uint32_t GetRefCounter() const;

				// members
				/** Raw packed value: free flag, ref counter, and size fields. */
				uint32_t m_Data;
			};

			// methods
			/** Constructs an end-marker block header. */
			ZHeader();

			/** Constructs a free or allocated block header. */
			ZHeader(bool, uint32_t);

			/** Returns true when this header describes a free block. */
			bool IsFreeHeader() const;

			/** Returns true when this header is the buffer terminator. */
			bool IsEnd() const;

			/** Returns the size stored in this header. */
			uint32_t GetSize() const;

			/** Packed block state used by the allocator. */
			ZRefAndSize m_RefOrSize;
		};

		/** Footer stored at the end of each free block for backward coalescing. */
		struct ZFreeFooter
		{
			// methods
			/** Constructs a footer for a free block of the given size. */
			ZFreeFooter(uint32_t iSize) : m_NonZero(1), m_Size(iSize)
			{}

			/** Returns true when this memory location contains a free footer. */
			bool IsFreeFooter() const { return m_NonZero != 0; }

			/** Returns the owning free block size. */
			uint32_t GetSize() const { return static_cast<uint32_t>(m_Size); }
			
			/** Computes the owning free header address from this footer. */
			ZFreeHeader* GetHeader() const
			{
				const uint8_t* pFooterAddr = reinterpret_cast<const uint8_t*>(this);
        		const uint8_t* pHeaderAddr = pFooterAddr - GetSize() + sizeof(ZFreeFooter);
        		return const_cast<ZFreeHeader*>(reinterpret_cast<const ZFreeHeader*>(pHeaderAddr));		
			}

			// members
			/** 24-bit size of the owning free block. */
			const ZUInt24 m_Size;

			/** Non-zero marker distinguishing a footer from allocated string data. */
			const char m_NonZero;
		};
		RE_VERIFY_SIZE(ZFreeFooter, 4);

		/** Header for a free block, also linked into the free-list. */
		struct ZFreeHeader : public ZHeader, public ZListNode<ZFreeHeader, 0>
		{
			// vtbl - none
			// methods
			/** Unlinks this free block from the free-list. */
			~ZFreeHeader() = default;
			
			/** Constructs a free block and writes its footer. */
			ZFreeHeader(uint32_t lSize) 
				: ZHeader(true, lSize)
			{
				// placement new for 'footer'
				new (GetFooter()) ZFreeFooter(lSize);
			}
			
			/** Shrinks this free block and returns the tail space for allocation. */
			ZHeader* Resize(uint32_t lSize)
			{
				const auto iCurrentSize = GetSize();

				if (iCurrentSize >= lSize)
				{
					const uint32_t lNewSize = iCurrentSize - lSize;

					if (lNewSize < sizeof(ZFreeHeader) + sizeof(ZFreeFooter))
						return nullptr;

					ZFreeFooter* pOldFooter = GetFooter();
					if (pOldFooter)
					{
						pOldFooter->~ZFreeFooter();
					}

					m_RefOrSize.Resize(lNewSize);

					ZFreeFooter* pNewFooter = GetFooter(lNewSize);
					new (pNewFooter) ZFreeFooter(lNewSize);

					return reinterpret_cast<ZHeader*>(reinterpret_cast<uint8_t*>(this) + lNewSize);
				}

				return nullptr;
			}

			/** Returns true when this block can satisfy an allocation request. */
			bool IsLargeEnough(uint32_t lSize) const
			{
				return GetSize() >= lSize;
			}

			/** Returns the footer for the current free block size. */
			ZFreeFooter* GetFooter() const
			{
				return GetFooter(GetSize());
			}

			/** Returns the footer for an explicit free block size. */
			ZFreeFooter* GetFooter(uint32_t lSelfSize) const
			{
				return const_cast<ZFreeFooter*>(reinterpret_cast<const ZFreeFooter*>(reinterpret_cast<const uint8_t*>(this) + lSelfSize - sizeof(ZFreeFooter)));
			}

			/** Extends this free block by absorbing adjacent released space. */
			void Extend(uint32_t lSize)
			{
				ZFreeFooter* pOldFooter = GetFooter();
				if (pOldFooter)
				{
					pOldFooter->~ZFreeFooter();
				}

				const uint32_t lNewSize = GetSize() + lSize;
				m_RefOrSize.Resize(lNewSize);

				ZFreeFooter* pNewFooter = GetFooter(lNewSize);
				new (pNewFooter) ZFreeFooter(lNewSize);
			}
			// members - none
		};

		/** End-marker header placed after the last managed block. */
		struct ZBufferTerminator : public ZHeader
		{
			/** Constructs the default end-marker header. */
			ZBufferTerminator() = default;
		};

		// methods
		/** Creates a manager with a fixed string buffer size. */
		ZStringMemoryManager(uint32_t lSize = 0x10000);

		/** Releases the backing buffer after unlinking intrusive lists. */
		~ZStringMemoryManager();

		/** Returns the process-wide RT string manager singleton. */
		static ZStringMemoryManager* Instance();

		/** Reinitializes the singleton manager in-place. */
		void NukeIt();

		/** Returns an interned object for the string, allocating it if needed. */
		ZRTStringObject* GetStringObject(const char* pStr);

		/** Returns the previous adjacent block if it is free. */
		ZFreeHeader* GetPrevIfFree(const char* pObject) const;

		/** Computes the original 12-bit hash bucket index for a string. */
		uint32_t GetHashCode(const char* pStr) const;

		/** Releases an allocated string object back into the free-list. */
		void Free(ZRTStringObject* pObject);

		/** Finds the first free block large enough for the requested size. */
		ZFreeHeader* FindFreeBlock(uint32_t lSize);

		/** Verifies basic buffer invariants and adjacent free-block coalescing. */
		bool Check();

		/** Allocates a raw string object for the given C string. */
		ZRTStringObject* Alloc(const char* pStr);

		// members
		/** Raw backing memory containing all headers, strings, and footers. */
		char* m_Buffer;

		/** Intrusive list of currently free blocks. */
		ZList<ZStringMemoryManager::ZFreeHeader,false,0> m_FreeList;

		/** Hash buckets containing interned string objects. */
		ZList<ZRTStringObject,true,0> m_HashTable[4096];

		/** First address that may contain a managed string object. */
		void* m_StringObjectStart;

		/** One-past-last address for managed string objects. */
		void* m_StringObjectEnd;     
	};

	/** Reference-counted interned string object stored inside the manager buffer. */
	struct ZRTStringObject : public ZStringMemoryManager::ZHeader, public ZListNode<ZRTStringObject, 0>
	{
		/** Constructs an allocated string object header of the given block size. */
		ZRTStringObject(uint32_t lSize);

		/** Reports leaked references and unlinks this object from hash buckets. */
		~ZRTStringObject();

		/** Placement allocation used when constructing inside manager storage. */
		static void* operator new(std::size_t, void* pMemory) { return pMemory; }

		/** Returns this object to the singleton string manager. */
		static void operator delete(void* pMemory);

		/** Matching placement delete for failed placement construction. */
		static void operator delete(void*, void*) {}

		/** Increments and returns the reference counter. */
		uint32_t IncRef();

		/** Decrements the reference counter and returns true when it reaches zero. */
		bool DecRef();

		/** Returns the null-terminated character data following the object header. */
		const char* c_str() const;

		/** Converts this object to its null-terminated character data. */
		operator const char*() const;
	};
	RE_VERIFY_SIZE(ZRTStringObject, 0xC);

	/** Small reference-counted handle to an interned runtime string object. */
	struct ZRTString
	{
		/** Constructs a null runtime string. */
		ZRTString();

		/** Constructs and interns a runtime string from a C string. */
		ZRTString(const char* pCString);

		/** Copy-constructs and shares the interned string object. */
		ZRTString(const ZRTString& other);

		/** Releases this handle's reference to the interned object. */
		~ZRTString();

		/** Attaches to an interned object and increments its reference count. */
		void Setup(ZRTStringObject* pStringObject);

		/** Releases the current interned object, freeing it on last reference. */
		void Cleanup();

		/** Returns the referenced C string, or null when empty. */
		const char* c_str() const;

		/** Compares this runtime string with a C string using strcmp semantics. */
		int Compare(const char* pCString) const;

		/** Assigns this handle to an interned copy of the given C string. */
		ZRTString& operator=(const char* pCString);

		/** Assigns this handle to share another runtime string object. */
		ZRTString& operator=(const ZRTString& other);

		/** Returns true when this string compares equal to the C string. */
		bool operator==(const char* pCString) const;

		/** Returns true when this string sorts before the C string. */
		bool operator<(const char* pCString) const;

		/** Returns the first character and asserts when the string is null. */
		char operator*() const;

		/** Converts this handle to its referenced C string, or null. */
		operator const char*() const;

		/** Referenced interned string object, or null for an empty handle. */
		ZRTStringObject* m_StringObject;
	};
}
