#pragma once

#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Com/CSharedCom.h>

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>

#include <cstdint>


namespace Glacier 
{
	/**
	 * @struct CCom
	 * @brief A fixed-capacity (16 KB) COM key-value store backed by a REFTAB32 index
	 *        and a ZOffsetAlloc memory manager.
	 *
	 * @details
	 * CCom provides a compact, self-contained key-value dictionary for game
	 * configuration and runtime variables. It inherits the full @ref CSharedCom
	 * interface (SetVal / GetVal / Get / operator[] / etc.) and implements
	 * the required pure-virtual storage back-end.
	 *
	 * ### Memory Layout
	 *
	 * A static 16384-byte buffer (`m_Memory`) serves as the arena. Each entry
	 * is laid out contiguously as:
	 *
	 * ```
	 * +--------+--------------+---------------+------------+
	 * | Header | ComValueInfo | Name (NUL-t.) | Data       |
	 * |  4 B   |    12 B      |  lNameLen B   | lDataLen B |
	 * +--------+--------------+---------------+------------+
	 * ```
	 *
	 * The 4-byte header stores the *user* (total aligned) size of the record,
	 * required by @ref Free to return the correct range to @ref ZOffsetAlloc.
	 * A `REFTAB32` (inline 32-element container) tracks all live `ComValueInfo`
	 * pointers for O(n) iteration and lookup.
	 *
	 * ### Allocation Strategy
	 *
	 * - @ref Alloc obtains space via `best-fit` from @ref ZOffsetAlloc.
	 * - @ref Free returns the space and coalesces adjacent free blocks.
	 * - On construction the full 16 KB is registered as a single free range.
	 *
	 * @note Total struct size is verified at **0x4148** (16712 bytes) for
	 *       binary compatibility with the original Glacier engine.
	 *
	 * @code
	 * CCom com;
	 * com.SetVal("iHealth", 100);
	 * com.SetVal("fSpeed", 1.5f);
	 * com.SetVal("sName", "Agent47");
	 *
	 * int health = com["iHealth"];
	 * float speed = com.Get("fSpeed");
	 *
	 * com.PrintStatus();
	 * com.Clear();
	 * @endcode
	 */
	struct CCom : public CSharedCom
	{
		// --- vtbl (virtual table overrides) ----------------------------------

		/**
		 * @brief Destructor. Resets the offset allocator and clears all
		 *        entries before invoking the base-class destructor.
		 */
		~CCom() override;

		/**
		 * @brief Resets the offset allocator to the full 16 KB range,
		 *        clears the REFTAB32 index, and resets the memory pointer.
		 *
		 * @post All previously stored values are invalidated.
		 * @post Subsequent allocations start at the beginning of `m_Memory`.
		 */
		void Clear() override;

		/**
		 * @brief Removes the value identified by @p pValName.
		 *
		 * @param pValName  Null-terminated name of the value to remove.
		 * @param lValNameLen Length of the name including NUL terminator;
		 *        if 0, `strlen(pValName) + 1` is used automatically.
		 * @return `true` if a matching entry was found and removed, `false` otherwise.
		 *
		 * @details Scans the REFTAB32 index for a matching name, calls @ref Free
		 *          to return the memory to the offset allocator, then removes the
		 *          pointer from the index.
		 *
		 * @code
		 * CCom com;
		 * com.SetVal("iScore", 42);
		 * bool removed = com.RemoveVal("iScore", 0);
		 * // removed == true
		 * @endcode
		 */
		bool RemoveVal(const char* pValName, int lValNameLen) override;

		/**
		 * @brief Checks whether a value with the given name exists.
		 *
		 * @param pValName  Null-terminated name to search for.
		 * @param lValNameLen Length including NUL; computed automatically when 0.
		 * @return `true` if the name is found, `false` otherwise.
		 */
		bool Exists(const char* pValName, int lValNameLen) override;

		/**
		 * @brief Low-level insertion of a typed value record.
		 *
		 * @param pValName    Raw name bytes (not necessarily NUL-terminated).
		 * @param lValNameLen Exact byte length of the name (including NUL if present).
		 * @param CCTCType    Pointer to the @ref CCOMTypeCast descriptor.
		 * @param lDataLen    Byte length of the data payload.
		 * @param pData       Raw data bytes; may be `nullptr` when `lDataLen` is 0.
		 *
		 * @details Any existing entry with the same name is removed first
		 *          (via @ref RemoveVal). The new record is allocated, filled
		 *          with header + ComValueInfo + name + data, and registered in
		 *          the REFTAB32 index.
		 *
		 * @pre @p pValName must not be null.
		 * @pre `lValNameLen` must be > 0.
		 */
		void AddValOfType(const char* pValName, int lValNameLen, CCOMTypeCast* CCTCType, int lDataLen, const char* pData) override;

		/**
		 * @brief Looks up the @ref ComValueInfo header for a named value.
		 *
		 * @param pValName    Name to search for.
		 * @param lValNameLen Name length including NUL; computed automatically when 0.
		 * @return Pointer to the @ref ComValueInfo header, or `nullptr` if not found.
		 */
		ComValueInfo* GetpVal(const char* pValName, int lValNameLen) override;

		/**
		 * @brief Deep-copies all entries from @p pSource into this CCom.
		 *
		 * @param pSource Source CCom to copy from; if null, the call is a no-op.
		 *
		 * @details Each entry in the source is allocated in this CCom's arena
		 *          using @ref Alloc. The ComValueInfo header, name, and data are
		 *          `memcpy`d verbatim.
		 *
		 * @note This performs a **merge** — existing entries in `this` are
		 *       preserved unless overwritten by the caller before calling this.
		 *
		 * @code
		 * CCom src, dst;
		 * src.SetVal("iAmmo", 30);
		 * dst.CopyAll(&src);
		 * int ammo = dst["iAmmo"]; // 30
		 * @endcode
		 */
		virtual void CopyAll(CCom* pSource);

		// --- methods ---------------------------------------------------------

		/**
		 * @brief Default constructor.
		 *
		 * Initializes the offset allocator over `m_Memory` (0 .. 0x4000) with
		 * 16 @ref ZOffsetAlloc::ZLink slots stored inline in `m_OffsetAllocLinks`.
		 * Sets `m_pMemory` to point to `m_Memory`.
		 */
		CCom();

		/**
		 * @brief Returns an allocated block to the free list.
		 *
		 * @param pData Pointer previously returned by @ref Alloc.
		 *
		 * @details Reads the 4-byte size header immediately before @p pData,
		 *          computes the offset from `m_pMemory`, and calls
		 *          @ref ZOffsetAlloc::Free.
		 *
		 * @pre @p pData must be a valid pointer returned by @ref Alloc
		 *      and must not have been freed already.
		 */
		void Free(void* pData);

		/**
		 * @brief Prints all stored key-value pairs to stdout.
		 *
		 * @details Iterates the REFTAB32 index and uses @ref CComRead to
		 *          fetch typed values. The first character of each key is
		 *          used as a type hint (`b` → bool, `f` → float, `i` → int32,
		 *          `s` → string, otherwise raw byte size).
		 */
		void PrintStatus();

		/**
		 * @brief Allocates @p lSize bytes (with 4-byte alignment) from the
		 *        internal memory arena.
		 *
		 * @param lSize Requested payload size.
		 * @return Pointer to the user-accessible region (past the 4-byte
		 *         size header), or `nullptr` / invalid on failure.
		 *
		 * @details The total allocation is rounded up to the next 4-byte
		 *          boundary (`(lSize + 3) & ~3`). A 4-byte header is
		 *          prepended to hold the aligned size, which @ref Free
		 *          later consumes.
		 */
		void* Alloc(uint32_t lSize);

		// --- members ---------------------------------------------------------

		/**
		 * @brief Inline 16 KB memory arena for all key-value storage.
		 */
		char m_Memory[0x4000];

		/**
		 * @brief Points to the active memory base (`m_Memory` initially;
		 *        may be redirected by subclasses).
		 */
		void* m_pMemory;

		/**
		 * @brief Fixed-size REFTAB index of up to 32 entries, each storing a
		 *        `ComValueInfo*` (cast to `uint32_t`).
		 */
		REFTAB32 m_ComDat;

		/**
		 * @brief Inline link array for @ref ZOffsetAlloc (capacity = 16).
		 */
		ZOffsetAlloc::ZLink m_OffsetAllocLinks[0x10];

		/**
		 * @brief Offset-based allocator managing the 16 KB arena.
		 */
		ZOffsetAlloc m_OffsetAlloc;
	};
	RE_VERIFY_SIZE(CCom, 0x4148);
}
