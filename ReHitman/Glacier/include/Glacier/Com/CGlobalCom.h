#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Com/CSharedCom.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @struct CGlobalCom
     * @brief A variable-size, defragmenting COM key-value store backed by a
     *        caller-supplied memory buffer.
     *
     * @details
     * `CGlobalCom` implements the @ref CSharedCom interface for a system-wide
     * key-value store (the "global COM"). Unlike @ref CCom which uses a fixed
     * 16 KB inline buffer and an offset allocator, `CGlobalCom` employs a
     * **linear append** strategy with **defragmentation**:
     *
     * ### Memory Layout
     *
     * Each entry is laid out sequentially in the buffer:
     *
     * ```
     * +-------------------+--------------+---------------+------------+
     * | CGlobalComMemInfo | ComValueInfo | Name (NUL-t.) | Data       |
     * |      8 B          |    12 B      |  lNameLen B   | lDataLen B |
     * +-------------------+--------------+---------------+------------+
     * ^
     * |
     * pMemInfo
     * ```
     *
     * `CGlobalComMemInfo` holds the total entry size and a `used` flag.
     * Removing a value simply marks it `used = false`; no memory is reclaimed
     * until @ref Defrag is called (automatically triggered when the buffer
     * fills up).
     *
     * ### Append Strategy
     *
     * - @ref Append writes new entries at `memoryWritePtr` and advances it.
     * - If a value with the same name already exists and its entry size is
     *   identical, the data is overwritten in-place (no new allocation).
     * - If sizes differ, the old entry is marked unused and a new entry is
     *   appended.
     * - When the buffer is full, @ref Defrag compacts live entries and resets
     *   `memoryWritePtr`. If still insufficient, an assertion fires.
     *
     * ### Global Instances
     *
     * Two global singletons exist (resolved at runtime):
     * - `g_pGlobalCOM` — pointer to the global COM (address `0x00820830`).
     * - `g_GlobalCom` — the global COM instance itself (address `0x008B4B90`).
     *
     * @note Total struct size is verified at **0x10** (16 bytes) for binary
     *       compatibility with the original Glacier engine.
     *
     * @code
     * char buffer[4096];
     * CGlobalCom com(buffer, sizeof(buffer));
     *
     * com.SetVal("iLevel", 5);
     * com.SetVal("sMission", "Flatline");
     *
     * char keyOut[256];
     * if (com.EnumKeys(keyOut, sizeof(keyOut), 0))
     *     printf("First key: %s\n", keyOut);
     *
     * com.RemoveVal("iLevel", 0);
     * com.Defrag();
     * @endcode
     */
    struct CGlobalCom : public CSharedCom
	{
		// --- types -----------------------------------------------------------

		/**
		 * @struct CGlobalComMemInfo
		 * @brief Per-entry metadata header stored before each @ref ComValueInfo.
		 */
		struct CGlobalComMemInfo
		{
			/**
			 * @brief Total size of this entry in bytes, including the
			 *        CGlobalComMemInfo header itself.
			 */
			uint32_t size;

			/**
			 * @brief Whether this entry is currently active.
			 *        Set to `false` by @ref RemoveVal; reclaimed by @ref Defrag.
			 */
			bool used;
		};
		RE_VERIFY_SIZE(CGlobalComMemInfo, 0x8);

		// --- vtbl (virtual table overrides) ----------------------------------

		/**
		 * @brief Destructor (defaulted — no dynamic resources owned).
		 */
		~CGlobalCom() override;

		/**
		 * @brief Resets the write pointer to the beginning of the buffer,
		 *        logically clearing all entries.
		 *
		 * @post @ref memoryWritePtr is 0; all stored values are lost.
		 *
		 * @code
		 * CGlobalCom com(buf, 1024);
		 * com.SetVal("iTest", 42);
		 * com.Clear();
		 * bool exists = com.Exists("iTest", 0); // false
		 * @endcode
		 */
		void Clear() override;

		/**
		 * @brief Marks the entry matching @p pValName as unused.
		 *
		 * @param pValName    Name of the value to remove.
		 * @param lValNameLen Name length including NUL; computed automatically when 0.
		 * @return `true` if the entry was found and marked unused, `false` otherwise.
		 *
		 * @details The entry's memory is not reclaimed immediately — call
		 *          @ref Defrag to compact the buffer.
		 */
		bool RemoveVal(const char* pValName, int lValNameLen) override;

		/**
		 * @brief Checks whether a value with the given name exists and is
		 *        marked as used.
		 *
		 * @param pValName    Name to search for.
		 * @param lValNameLen Name length including NUL; computed automatically when 0.
		 * @return `true` if a used entry is found, `false` otherwise.
		 */
		bool Exists(const char* pValName, int lValNameLen) override;

		/**
		 * @brief Low-level insertion of a typed value record.
		 *
		 * @param pValName    Raw name bytes.
		 * @param lValNameLen Exact byte length of the name.
		 * @param CCTCType    Pointer to the @ref CCOMTypeCast descriptor.
		 * @param lDataLen    Byte length of the data payload.
		 * @param pData       Raw data bytes.
		 *
		 * @details Builds a temporary contiguous record on the stack (via `alloca`),
		 *          then delegates to @ref Append for insertion into the buffer.
		 */
		void AddValOfType(const char* pValName, int lValNameLen, CCOMTypeCast* CCTCType, int lDataLen, const char* pData);

		/**
		 * @brief Looks up the @ref ComValueInfo header for a named value.
		 *
		 * @param pValName    Name to search for.
		 * @param lValNameLen Name length including NUL; computed automatically when 0.
		 * @return Pointer to the @ref ComValueInfo (past the @ref CGlobalComMemInfo
		 *         header), or `nullptr` if not found.
		 */
		ComValueInfo* GetpVal(const char* pValName, int lValNameLen) override;

		/**
		 * @brief Enumerates the key at the given index.
		 *
		 * @param[out] output Buffer to receive the NUL-terminated key string.
		 * @param       maxLen Capacity of @p output in bytes.
		 * @param       index  Zero-based index of the key to retrieve.
		 * @return `true` if a key was written, `false` if @p index is out of range
		 *         or the buffer is too small.
		 *
		 * @details Skips entries marked as unused. Useful for iterating all
		 *          live keys without needing to know their names in advance.
		 *
		 * @code
		 * char name[256];
		 * for (unsigned i = 0; com.EnumKeys(name, sizeof(name), i); ++i)
		 *     printf("Key %u: %s\n", i, name);
		 * @endcode
		 */
		virtual bool EnumKeys(char* output, int maxLen, unsigned int index);

		// --- methods ---------------------------------------------------------

		/**
		 * @brief Constructs a CGlobalCom backed by the given buffer.
		 *
		 * @param pBuffer    Pointer to a caller-owned memory block.
		 * @param lBufferSize Total size of the buffer in bytes.
		 *
		 * @code
		 * uint8_t mem[8192];
		 * CGlobalCom com(mem, sizeof(mem));
		 * @endcode
		 */
		CGlobalCom(void* pBuffer, int lBufferSize);

		/**
		 * @brief Default constructor. Initializes with null buffer and zero size.
		 *
		 * @details Useful for the global instance which receives its buffer
		 *          later at engine initialization time.
		 */
		CGlobalCom();

		/**
		 * @brief Appends a pre-built entry to the buffer.
		 *
		 * @param pNewVal Pointer to a fully formed @ref ComValueInfo + name + data
		 *                in contiguous memory.
		 *
		 * @details If an existing entry with the same name has identical size,
		 *          the data is overwritten in-place. Otherwise a new entry is
		 *          appended at @ref memoryWritePtr. If the buffer is full,
		 *          @ref Defrag is called once; if still full, an assertion fires.
		 */
		void Append(ComValueInfo* pNewVal);

		/**
		 * @brief Compacts the buffer by removing entries marked as unused.
		 *
		 * @details Iterates all entries, `memmove`s live ones to the front,
		 *          then sets @ref memoryWritePtr to the new logical end.
		 *
		 * @post All entries marked `used = false` are permanently removed.
		 * @post @ref memoryWritePtr reflects the compacted end of live data.
		 *
		 * @code
		 * com.RemoveVal("iOldKey", 0);
		 * com.RemoveVal("fOldVal", 0);
		 * com.Defrag(); // reclaims space from both
		 * @endcode
		 */
		void Defrag();

		/**
		 * @brief Finds the @ref CGlobalComMemInfo header for a named value.
		 *
		 * @param pValName    Name to search for.
		 * @param lValNameLen Name length including NUL; computed automatically when 0.
		 * @return Pointer to the @ref CGlobalComMemInfo header, or `nullptr` if not found.
		 *
		 * @details Uses case-insensitive comparison (`_memicmp`). Skips unused entries.
		 */
		CGlobalComMemInfo* Find(const char* pValName, int lValNameLen);

		// --- members ---------------------------------------------------------

		/**
		 * @brief Total capacity of the memory buffer in bytes.
		 */
		int maxSize;

		/**
		 * @brief Pointer to the caller-owned memory buffer.
		 */
		void* memory;

		/**
		 * @brief Current write position within the buffer.
		 *        All entries reside in `[0, memoryWritePtr)`.
		 */
		int memoryWritePtr;
	};
	RE_VERIFY_SIZE(CGlobalCom, 0x10);

	STATIC_GLOBAL_CLASS_INSTANCE(CGlobalCom*, g_pGlobalCOM);
	STATIC_GLOBAL_CLASS_INSTANCE(CGlobalCom, g_GlobalCom);
}
