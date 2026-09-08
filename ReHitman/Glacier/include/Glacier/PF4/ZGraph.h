#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/PF4/PF4.h>


namespace Glacier::PF4
{
    // Run-time description of one navigable sub-mesh. All indices are relative to
    // the per-graph arrays and translated to the global ZData arrays by adding the
    // m_iFirst* offsets.
    struct ZGraph
    {
        ZIndex  m_iNodes;           // +0x00 number of nodes
        ZIndex  m_iVertices;        // +0x02 number of ring vertices
        ZIndex  m_iComponents;      // +0x04 number of components
        ZIndex  m_iExits;           // +0x06 exit count (exit-distance rows)
        ZIndex  m_iEntrances;       // +0x08
        ZIndex  m_iFirstNode;       // +0x0A first node in ZData::m_pNodes
        ZIndex  m_iFirstVertex;     // +0x0C first vertex in ZData::m_pVertices
        ZIndex  m_iFirstComponent;  // +0x0E first component in ZData::m_pComponents
        ZUIndex m_iFirstExitDist;   // +0x10 first exit distance in ZData::m_pExitDists
        ZIndex  m_iSplitTree;       // +0x12 graph split tree
        ZIndex  m_iHeightTree;      // +0x14 height tree
        ZIndex  m_iEquations;       // +0x16 plane equations
        ZVector3 m_Min;             // +0x18 bounds
        ZVector3 m_Max;             // +0x24
    };
    RE_VERIFY_SIZE(ZGraph, 0x30); // Confirmed (PC pf4runtime)
}
