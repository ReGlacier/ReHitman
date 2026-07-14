#pragma once

#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/ZSTL/ZQStaticMemBlock.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
	struct SBaseGeomListHeader
	{
		// Number of stored ranges per list type. The low 3 bits of every geom id store this type.
		uint8_t m_cTypeLens[8];
		// Number of used payload slots. Odd value means slot 0 is a link to the next list node.
		uint8_t m_cUsedLen;
		// Total payload slot capacity. Slots store uintptr_t values for pointer-size portability.
		uint8_t m_cPoolLen;
		uint16_t m_sDummy;
	};
	RE_VERIFY_SIZE(SBaseGeomListHeader, 0xC);

	/**
	 * Walks pairs stored in a chained base-geom list.
	 *
	 * Each list node starts with SBaseGeomListHeader and is followed by payload slots.
	 * If m_cUsedLen is odd, payload slot 0 is a link to the next node and the remaining
	 * slots are geom ranges: [first packed geom, last packed geom]. Packed geom ids keep
	 * the geom pointer with QLISTGEOMTYPE stored in the low 3 bits.
	 */
	struct SGeomPairRecursion
	{
		// methods
		/**
		 * @brief Initializes iteration over the first node that contains at least one geom pair.
		 *
		 * @param pList Pointer-sized id of the root list node, or 0 for an empty list.
		 *
		 * @details Empty forwarding nodes are skipped. If the active node has an odd
		 * m_cUsedLen, payload slot 0 is treated as a link to the next node and iteration
		 * starts at slot 1.
		 */
		void InitPair(uintptr_t pList);
		/**
		 * @brief Advances iteration to the next linked node when the current node is exhausted.
		 *
		 * @details The caller is responsible for advancing m_cCur while reading pairs.
		 * This method only follows the node chain after m_cCur reaches m_cCurEnd.
		 */
		void NextPair();

		// members
		SBaseGeomListHeader *DpInsertList;
		uint8_t m_cCur;
		uint8_t m_cCurEnd;
		uint16_t m_sCursorPad;
		ZBaseGeom *pSingle;
		uintptr_t m_pSizePad;
	};
	RE_VERIFY_SIZE(SGeomPairRecursion, 0x10);

    /**
     * Stores compact ranges of consecutive ZBaseGeom instances grouped by list type.
     *
     * The original engine stores list nodes inside the ZQStaticMemBlock buffer and refers
     * to them by pointer-sized ids. A node can point to an older node, forming a small
     * chain when the current node has no room for another range. AddBaseGeoms merges
     * adjacent ranges with the same parent and type; RemoveBaseGeoms shrinks, removes,
     * or splits existing ranges.
     */
	struct ZBaseGeomLists : public ZQStaticMemBlock
	{
		// methods
		/**
		 * @brief Creates a base-geom list allocator backed by a static memory block.
		 *
		 * @param lSize Size of the backing buffer in bytes.
		 * @param pFreeList Optional initial free-list data. The original debug path does
		 * not support this for the current reverse-engineered implementation.
		 */
		ZBaseGeomLists(uint32_t lSize, uint32_t* pFreeList);
		/**
		 * @brief Checks whether @p pGeom is covered by any range in the list chain.
		 *
		 * @param pList Pointer-sized id of the root list node, or 0 for an empty list.
		 * @param pGeom Geometry pointer to search for.
		 * @param lType List type stored in the low 3 bits of packed geom ids.
		 * @return True when @p pGeom lies between a stored first/last geom pair.
		 *
		 * @note This mirrors the original behavior and currently does not filter by
		 * @p lType after finding a containing range.
		 */
		bool Exists(uintptr_t pList, ZBaseGeom* pGeom, uint32_t lType);
		/**
		 * @brief Adds a consecutive range of base geoms to a list chain.
		 *
		 * @param pList Pointer-sized id of the current root list node, or 0 to create one.
		 * @param pFirstGeom First geom in the consecutive range.
		 * @param pLastGeom Last geom in the consecutive range.
		 * @param lType List type stored in the low 3 bits of packed geom ids.
		 * @param lMinPoolSize Requested minimum node payload capacity. Rounded up to an
		 * 8-slot boundary and required to fit in one byte.
		 * @return Pointer-sized id of the root list node. This may differ from @p pList
		 * when a new head node is allocated.
		 *
		 * @details Adjacent ranges with the same parent and type are merged instead of
		 * inserting a new pair. If the selected node has no room for two more payload
		 * slots, a new node is allocated from the inherited ZQStaticMemBlock allocator.
		 */
		uintptr_t AddBaseGeoms(uintptr_t pList, ZBaseGeom* pFirstGeom, ZBaseGeom* pLastGeom, uint32_t lType, uint32_t lMinPoolSize);
		/**
		 * @brief Removes a consecutive range of base geoms from a list chain.
		 *
		 * @param pList Pointer-sized id of the current root list node, or 0 for an empty list.
		 * @param pFirstGeom First geom in the range to remove.
		 * @param pLastGeom Last geom in the range to remove.
		 * @param lType List type stored in the low 3 bits of packed geom ids.
		 * @return Pointer-sized id of the root list node after removal. Returns 0 when
		 * the last range is removed and the chain becomes empty.
		 *
		 * @details Depending on where the removed range lies, this can shrink the start
		 * of a stored range, shrink its end, remove a whole pair, or split one stored
		 * range into two ranges. Empty nodes are returned to the inherited offset
		 * allocator.
		 */
		uintptr_t RemoveBaseGeoms(uintptr_t pList, ZBaseGeom* pFirstGeom, ZBaseGeom* pLastGeom, uint32_t lType);
	};
	RE_VERIFY_SIZE(ZBaseGeomLists, 0x18);
}
