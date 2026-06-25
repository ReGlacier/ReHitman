#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/ZInterface.h>
#include <Glacier/PF4/PF4.h>
#include <cstdint>


namespace Glacier::PF4
{
    struct ZOpenNode
    {
        bool bVisited;
        RE_ADD_PADDING(3);
        float fKey;
        ZIndex iNode;
        ZIndex iParent;
        ZIndex iLinkIndex;
        ZIndex[2] iGate;
        float fCost;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZOpenNode, 24);

    struct ZOpenNodeList 
    {
        ZNodeData* m_pNodeData;
        int m_iNodeCount;
        int m_iOpenNodeCount;
        int m_iClosedNodeCount;
        ZOpenNode[1000] m_aList;
    };
    RE_VERIFY_SIZE(ZOpenNodeList, 0x5DD0);

    struct ZBlockAlocator
    {
        PF4::ZDataRef* m_Data;
        short* m_Stack;
        int m_BlockSize;
        int m_MaxBlocks;
        int m_Count;
    };
    RE_VERIFY_SIZE(ZBlockAlocator, 0x14);

    struct ZData : public ZInterface
    {
        ZGraph*           m_pGraphs;
        ZNode*            m_pNodes;
        ZVertex*          m_pVertices;
        ZLink*            m_pLinks;
        ZComponent*       m_pComponents;
        ZSplitTree*       m_pSplitTrees;
        ZSplitTree*       m_pHeightTrees;
        ZPlaneEquation*   m_pPlaneEquations;
        ZIndex*           m_pExitDists;
        ZCorner*          m_pCorners;
        ZSubNode*         m_pSubNodes;
        ZIndex*           m_pStaticObstacleIds;
        ZStaticObstacle*  m_pStaticObstacles;
        ZIndex            m_iSplitTreeCount;
        ZIndex            m_iHeightTreeCount;
        ZIndex            m_iPlaneEquationCount;
        ZIndex            m_iComponentCount;
        ZIndex            m_iGraphCount;
        ZIndex            m_iNodeCount;
        ZIndex            m_iVertexCount;
        ZIndex            m_iLinkCount;
        ZUIndex           m_iExitDistCount;
        ZIndex            m_iCornerCount;
        ZIndex            m_iSubNodeCount;
        ZIndex            m_iStaticObstacleIdCount;
        ZIndex            m_iStaticObstacleCount;
        ZLink**           m_pDynamicObstacles;
        int               m_ObstacleCount;
        int               m_iMetaId;
        unsigned int*     m_pComponentVisited;
        int               m_iComponentVisitedSize;
        ZNodeData*        m_pNodeData;
        ZOpenNodeList     m_kOpenList;
        int16*            m_BlockStack;
        ZDataRef*         m_DataRefs;
        ZBlockAlocator[3] m_Allocator;
    };
}