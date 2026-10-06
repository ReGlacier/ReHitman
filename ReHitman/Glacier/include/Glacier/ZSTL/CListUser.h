#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/CMemPool.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZBaseGeom;

    /**
     * @brief Dynamic overflow block for CListUser adjacency lists.
     *
     * The original layout stores three 32-bit node pointer values followed by a
     * pointer to the next overflow block. The fields intentionally keep the
     * recovered 32-bit pointer-shaped layout.
     */
    struct SDynBlock
    {
        // members
        /** @brief First dynamic list node pointer value. */
        uint32_t* pNode1;
        /** @brief Second dynamic list node pointer value. */
        uint32_t* pNode2;
        /** @brief Third dynamic list node pointer value. */
        uint32_t* pNode3;
        /** @brief Next dynamic overflow block pointer value. */
        uint32_t* pNextBlock;
    };
    RE_VERIFY_SIZE(SDynBlock, 0x10);

    /**
     * @brief Stores and updates packed adjacency lists between ZBaseGeom nodes.
     *
     * Static list data lives in the generated buffer. Runtime additions spill into
     * CMemPool-backed SDynBlock chains. Node references are packed into 32-bit
     * values whose low three bits are list attributes and whose high bits hold an
     * aligned node pointer, offset, or converted reference depending on load state.
     */
    struct CListUser : public CMemPool
    {
        // vtbl
        /** @brief Replaces the caller node's adjacency set with the collected collision set. */
        virtual void AnalyzeCatch(uint32_t uLengthCol, void* pCallerNode);
        /** @brief Checks whether a node is present in either the static or dynamic part of a list. */
        virtual bool IsNodeInList(uint32_t uListID, void* pNode);
        /** @brief Expands a packed list into an internal contiguous node pointer buffer. */
        virtual uint32_t* UnfoldList(uint32_t* pListLength, uint32_t uListID);
        /** @brief Returns the temporary collision buffer and its maximum capacity. */
        virtual uint32_t* GetCatchBuffer(uint32_t* pMaxInput);
        /** @brief Disconnects a caller node from every member in its current list. */
        virtual void DisconnectFromAllMembers(void* pCallerNode);
        /** @brief Marks every member in the caller node's list as needing list updates. */
        virtual void NotifyAllMembers(void* pCallerNode);
        /** @brief Removes the bidirectional relationship between two geometry nodes. */
        virtual bool DisconnectNodeFromNode(ZBaseGeom* pNode1, ZBaseGeom* pNode2);
        /** @brief Converts packed static and dynamic list offsets into runtime references/pointers. */
        virtual void ConvertOffsetsToRefs(const uint32_t* pRemapTable);
        /** @brief Returns the total generated buffer size in 32-bit words. */
        virtual uint32_t GetTotalBufferSize() const;
        /** @brief Returns the beginning of the generated static/runtime/dynamic buffer. */
        virtual uint32_t* GetFullBuffer() const;
        /** @brief Allocates a runtime list entry for a node and returns its list id. */
        virtual uint32_t AddRuntimeMember(void* pNode);
        /** @brief Frees a node's runtime list entry and clears its runtime list id. */
        virtual void RemoveRuntimeMember(void* pNode);

        // methods
        /** @brief Builds a list user over an optional generated buffer. */
        CListUser(void* pGenBuf);
        /** @brief Returns whether a member index points into the runtime buffer region. */
        bool IsRuntimeMember(uint32_t) const;
        /** @brief Converts one packed member entry from offset/ref form into pointer form. */
        void FixMember(uint32_t*, uint32_t, const uint32_t*);
        /** @brief Converts all packed entries in a dynamic overflow block chain. */
        void ConvertInDynamicBuf(uint32_t* uDynAddr, const uint32_t* pRemapTable);
        /** @brief Removes a static list hole by shifting subsequent entries left. */
        void ClearHoleInStaticList(uint32_t*);
        /** @brief Finds, and optionally removes, a node in the static part of a list. */
        uint32_t TrackDownInStaticList(uint32_t uListID, void* pNode, bool bPermitDelete);
        /** @brief Finds, and optionally removes, a node in dynamic overflow blocks. */
        uint32_t TrackDownInDynamicList(uint32_t uListID, void* pNode, bool bPermitDelete);
        /** @brief Appends a batch of node pointer values to a list's dynamic blocks. */
        uint32_t ConnectBatchToList(uint32_t uListID, uint32_t* pNodes, int iLength);
        /** @brief Performs the compact static-list delete path for single entries or ranges. */
        uint32_t QuickStaticDelete(uint32_t uListID, uint32_t* pListPos, void* pNode, bool bIsFromTo);
        /** @brief Removes a node from the static part of a list. */
        uint32_t DisconnectFromStaticList(uint32_t uListID, void* pNodes);
        /** @brief Removes a node from the dynamic part of a list. */
        uint32_t DisconnectFromDynamicList(uint32_t uListID, void* pNodes);
        /** @brief Expands static and dynamic list entries and returns packed static/dynamic counts. */
        uint32_t PriUnfoldList(uint32_t* pGetMembers, uint32_t uListID);
        /** @brief Adds a single node to a list. */
        bool ConnectToList(uint32_t uListID, void* pNodes);
        /** @brief Removes a single node from a list. */
        bool DisconnectFromList(uint32_t uListID, void* pNodes);
        /** @brief Checks the static part of a list for a node. */
        bool IsNodeInStaticList(uint32_t uListID, void* pNodes);
        /** @brief Checks the dynamic part of a list for a node. */
        bool IsNodeInDynamicList(uint32_t uListID, void* pNodes);

        // members
        /** @brief Size of the generated static-list region in 32-bit words. */
        uint32_t m_uSizeOfStaticBuf;
        /** @brief Base pointer to the generated buffer. */
        uint32_t* m_pStaticBuf;
        /** @brief Size of the generated dynamic block region in 32-bit words. */
        uint32_t m_uSizeOfDynamicBuf;
        /** @brief Base pointer to dynamic overflow blocks backed by this CMemPool. */
        SDynBlock* m_pDynamicBuf;
        /** @brief Size of the runtime-entry region in 32-bit words. */
        uint32_t m_uSizeOfRuntimeBuf;
        /** @brief Pool used for two-word runtime list entries. */
        CMemPool* m_pGetDynEntryPool;
        /** @brief Maximum number of node pointers supported by temporary buffers. */
        uint32_t m_uMaxLength;
        /** @brief Temporary buffer containing collided node pointer values. */
        uint32_t* m_pCollidedWith;
        /** @brief Temporary buffer used by PriUnfoldList. */
        uint32_t* m_pInternalListUnfold;
        /** @brief Temporary merge/output buffer used by AnalyzeCatch. */
        uint32_t* m_pInternalOutBuf;
    };
    RE_VERIFY_SIZE(CListUser, 0x44);
}
