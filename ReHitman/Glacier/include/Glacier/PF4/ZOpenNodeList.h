#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/PF4.h>
#include <Glacier/PF4/ZOpenNode.h>


namespace Glacier::PF4
{
    struct ZNodeData;

    // Binary heap of the component/navmesh A* search plus the per-node open/closed
    // bookkeeping. Every navmesh node has a ZNodeData::iOpenNode slot giving its
    // position inside m_aList, or -1 when it is neither open nor closed. Nodes
    // extracted by ExtractMin are kept in the closed tail of m_aList so that the
    // reverse path walk can read back their final key/cost/parent.
    struct ZOpenNodeList
    {
        // methods (PC pf4runtime: ExtractMin 0x4DA770, DecreaseKey 0x4DA6B0,
        // Swap 0x4DA600, SetNodeData 0x4DA580)
        float GetKey(int iNode) const;
        float GetCost(int iNode) const;
        int GetLinkIndex(int iNode) const;
        bool GetVisited(int iNode) const;
        void SetVisited(int iNode, bool bVisited);
        void SetNodeData(ZNodeData* pNodeData, int iNode);
        void ResetUsedNodeData();
        int ExtractMin();
        void DecreaseKey(
            int iParent, int iNode, int iLinkIndex, int iGate0, int iGate1, float fKey, float fCost);
        void Swap(int iOpenNodeA, int iOpenNodeB);

        // members
        ZNodeData* m_pNodeData; // +0x00 per-node open-list positions
        int m_iNodeCount;       // +0x04 number of navmesh nodes
        int m_iOpenNodeCount;   // +0x08 open heap entries
        int m_iClosedNodeCount; // +0x0C closed entries kept at the tail
        ZOpenNode m_aList[1000];
    };
    RE_VERIFY_SIZE(ZOpenNodeList, 0x5DD0); // Confirmed
}
