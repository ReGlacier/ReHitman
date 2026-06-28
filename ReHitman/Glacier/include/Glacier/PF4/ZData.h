#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>

#include <Glacier/PF4/EPathWayActions.h>
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
        ZIndex iGate[2];
        float fCost;
    };
    RE_VERIFY_SIZE(ZOpenNode, 0x18); // Confirmed

    struct ZOpenNodeList 
    {
        ZNodeData* m_pNodeData;
        int m_iNodeCount;
        int m_iOpenNodeCount;
        int m_iClosedNodeCount;
        ZOpenNode m_aList[1000];
    };
    RE_VERIFY_SIZE(ZOpenNodeList, 0x5DD0); // Confirmed

    struct ZBlockAlocator
    {
        ZDataRef* m_Data;
        short* m_Stack;
        int m_BlockSize;
        int m_MaxBlocks;
        int m_Count;
    };
    RE_VERIFY_SIZE(ZBlockAlocator, 0x14); // Confirmed

    struct ZVertex
    {
        EPathWayActions m_eAction;
        ZVector3 Position;
        float distFromStart;
        int id;
    };
    RE_VERIFY_SIZE(ZVertex, 0x18);

    struct ZNode
    {
        ZVector3 m_Pos;
        uint32_t m_FirstChild;
        uint32_t m_LastChild;
        uint32_t m_sExitNodeID;
        uint32_t m_sExitGraphID;
        int m_ExitId;
        uint32_t m_myGraph;
        uint32_t coord;
    };
    RE_VERIFY_SIZE(ZNode, 0x28);

    struct ZLink 
    {
        int16_t m_fCost;
        ZIndex m_iNode;
        ZIndex m_Graph;
        uint8_t m_Type;
        uint8_t m_iAction;
        uint16_t m_iKeyMask;
        ZREF m_rDoorController;
    };
    RE_VERIFY_SIZE(ZLink, 0x10);

    struct ZComponent 
    {
        uint8_t m_Corners;
        uint8_t m_SubNodes;
        uint8_t m_ObstacleIds;
        uint8_t PADDING;
        ZMetaNode* m_MetaNodes;
        ZIndex        m_Graph;
        ZIndex        m_FirstCorner;
        ZIndex        m_FirstSubNode;
        ZIndex        m_FirstObstacleId;
        ZIndex        m_FirstHeightTree;
    };
    RE_VERIFY_SIZE(ZComponent, 0x14);

    struct ZPlaneEquation 
    {
        float m_fA;
        float m_fB;
        float m_fC;
    };
    RE_VERIFY_SIZE(ZPlaneEquation, 0xC); // Confirmed

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
        int32_t           m_ObstacleCount;
        int32_t           m_iMetaId;
        uint32_t*         m_pComponentVisited;
        int32_t           m_iComponentVisitedSize;
        ZNodeData*        m_pNodeData;
        ZOpenNodeList     m_kOpenList;
        int16_t*          m_BlockStack;
        ZDataRef*         m_DataRefs;
        ZBlockAlocator    m_Allocator[3];
    };
}