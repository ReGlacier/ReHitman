#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZScheduledEvent;
    class ZEventBase;

    /**
     * @class ZScheduledEventList
     * @brief An intrusive, circular singly-linked list optimized for low-overhead game event scheduling.
     *
     * @details This class serves as a specialized priority queue managing `ZScheduledEvent` objects,
     *          typically sorted by their execution timestamps (`TIMETYPE`). Instead of relying on 
     *          standard collections or heap-based binary heaps, it uses an intrusive design where 
     *          the node links (`m_pNxt`) reside directly within the events themselves.
     * 
     *          ### Memory Architecture & Layout
     *          The list is implemented as a **Circular Singly-Linked List**. The class stores only a 
     *          single pointer, `m_pLast`, which points directly to the **tail** (last element) of the queue. 
     *          Due to the circular topology:
     *          - `m_pLast` represents the furthest event in the future (the maximum time).
     *          - `m_pLast->m_pNxt` directly yields the **head** (first element) of the queue, 
     *            representing the most immediate event to execute.
     *          This allows both head retrieval and tail insertion to operate in stable $O(1)$ time.
     * 
     *          ### Key Responsibilities & Behaviors
     *          - **Time-Ordered Dispatching (`InsertSorted`):** Maintains events sorted chronologically 
     *            using fixed-point time deltas. While insertion is theoretically $O(N)$, the flat iteration 
     *            combined with ultra-fast fixed-point scalar comparisons outperforms complex tree 
     *            structures for average game frame sizes.
     *          - **Lazy Evaluation & Cleanup (`First`):** Implements an aggressive deferred-cleanup 
     *            pattern. Instead of instantly unlinking modified/canceled events (which would require a 
     *            costly $O(N)$ linear search to find the predecessor), events are simply flagged via 
     *            `m_bChanged`. The queue filters and unlinks these dead nodes lazily during the next 
     *            dispatch loop inside `First()`.
     *          - **O(1) Removals (`RemoveEntry`):** Performs node isolation in strict $O(1)$ time by 
     *            taking a pointer to the *predecessor* node rather than the target node itself, 
     *            avoiding link traversal overhead.
     *          - **Queue Rotation (`MoveForward`):** Allows cyclic streaming/rotation of scheduled tasks 
     *            without moving any blocks in memory by simply advancing the `m_pLast` tail pointer.
     */
    struct ZScheduledEventList
    {
        // methods
        ZScheduledEventList();

        /**
         * @brief Retrieves the first event in the list, performing a lazy cleanup of modified events.
         *
         * @details The list is implemented as a circular single-linked list where `m_pLast` points to 
         *          the tail, making `m_pLast->m_pNxt` the actual head (first element) of the list.
         * 
         *          Instead of instantly removing events from the queue when their parameters change 
         *          (which is an expensive O(N) operation), the engine uses a **Lazy Cleanup** pattern. 
         *          If the event at the head of the list has its `m_bChanged` flag set, this function 
         *          will unlnk it, hand it back to the global scheduler via `AttachEvent()` for 
         *          re-sorting, and move to the next event.
         * 
         *          This process repeats until a valid, unmodified event is found or the list becomes empty.
         *
         * @return ZScheduledEvent* Pointer to the first valid event ready for execution, 
         *                          or `nullptr` if the list is empty or all elements were evacuated.
         */
        ZScheduledEvent* First();

        ZScheduledEvent* Last() const;
        int Exists(ZScheduledEvent* pEvent) const;
        void Insert(ZScheduledEvent* pEvent);
        void InsertSorted(ZScheduledEvent* pEvent, TIMETYPE timeDelta);
        void Remove();
        void RemoveEntry(ZScheduledEvent* pPrevEvent);
        void MoveForward();

        // members
        ZScheduledEvent* m_pLast{nullptr};
    };
    RE_VERIFY_SIZE(ZScheduledEventList, 0x4);
}