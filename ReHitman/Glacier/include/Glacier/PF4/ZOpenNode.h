#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/PF4.h>


namespace Glacier::PF4
{
    // A single entry of the A* open/closed heap (PC pf4runtime). While a node is
    // open the entry lives in m_aList[0..iOpenNodeCount); when it is popped by
    // ExtractMin the entry is copied to the closed area at the tail of the same
    // array so the final costs remain reachable for the path reconstruction.
    struct ZOpenNode
    {
        // members
        bool bVisited;
        RE_ADD_PADDING(3);
        float fKey;        // +0x04 sort key of the heap
        ZIndex iNode;      // +0x08 node the entry belongs to
        ZIndex iParent;    // +0x0A parent node id (-1 for seed entries)
        ZIndex iLinkIndex; // +0x0C component/gate tag of the entry
        ZIndex iGate[2];   // +0x0E type / sub-node tag pair of the entry
        float fCost;       // +0x14 accumulated path cost (g)
    };
    RE_VERIFY_SIZE(ZOpenNode, 0x18); // Confirmed
}
