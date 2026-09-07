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
    class ZBlockAlocator
    {
    public:
        // methods
        ZBlockAlocator();
        ~ZBlockAlocator();
        void Init(int16_t* pStack, ZDataRef* pDataRef);
        ZDataRef* Alloc();
        void Free(ZDataRef* pRef);
        bool BelongsTo(ZDataRef* pDataRef) const;

        // members
        ZDataRef* m_Data;
        short* m_Stack;
        int m_BlockSize;
        int m_MaxBlocks;
        int m_Count;
    };
    RE_VERIFY_SIZE(ZBlockAlocator, 0x14); // Confirmed

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



    struct ZVertex
    {
        ZVector2 m_kPos;      // +0x0 world x/z
        float m_fHeight;      // +0x8
        ZVector2 m_kNormal;   // +0xC
    };
    RE_VERIFY_SIZE(ZVertex, 0x14); // Confirmed (PC pf4runtime)

    struct ZNode
    {
        ZVector2 m_kPos;      // +0x0 world x/z
        float m_fHeight;      // +0x8
        ZIndex m_iFirstLink;  // +0xC
        ZIndex m_iLastLink;   // +0xE
        ZIndex m_iComponent;  // +0x10
        ZIndex m_iGraph;      // +0x12
    };
    RE_VERIFY_SIZE(ZNode, 0x14); // Confirmed (PC pf4runtime)

    struct ZNodeData
    {
        ZIndex iOpenNode;
    };
    RE_VERIFY_SIZE(ZNodeData, 0x2);

    struct ZSplitTree
    {
        ZIndex m_iChild[2];   // +0x0 child index (or encoded component when negative)
        float m_fA;           // +0x4 plane: A * x + B * z
        float m_fB;           // +0x8
        float m_fC;           // +0xC
    };
    RE_VERIFY_SIZE(ZSplitTree, 0x10); // Confirmed (PC pf4runtime)

    struct ZCorner
    {
        ZIndex m_iComponent;  // +0x0
        ZIndex m_iVertex;     // +0x2
    };
    RE_VERIFY_SIZE(ZCorner, 0x4); // Confirmed (PC pf4runtime)

    struct ZSubNode
    {
        ZIndex m_Node;            // +0x0
        ZIndex m_Divider;         // +0x2
        ZIndex m_SourceComponent; // +0x4
        ZIndex m_Component;       // +0x6
    };
    RE_VERIFY_SIZE(ZSubNode, 0x8); // Confirmed (PC pf4runtime)

    struct SPF4DataBlock
    {
        int16_t m_iSplitTreeCount;
        int16_t m_iHeightTreeCount;
        int16_t m_iPlaneEquationCount;
        int16_t m_iComponentCount;
        int16_t m_iGraphCount;
        int16_t m_iNodeCount;
        int16_t m_iVertexCount;
        int16_t m_iLinkCount;
        int16_t m_iExitDistCount;
        int16_t m_iCornerCount;
        int16_t m_iSubNodeCount;
        int16_t m_iStaticObstacleIdCount;
        int16_t m_iStaticObstacleCount;
        int16_t PADDING;
    };
    RE_VERIFY_SIZE(SPF4DataBlock, 0x1C);

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

    struct ZPathLink
    {
        ZVector3 m_Pos;
        int m_Action;
    };

    struct ZPlaneEquation
    {
        float m_fA;
        float m_fB;
        float m_fC;
    };
    RE_VERIFY_SIZE(ZPlaneEquation, 0xC); // Confirmed

    struct ZGraph
    {
        ZIndex  m_iNodes;
        ZIndex  m_iVertices;
        ZIndex  m_iComponents;
        ZIndex  m_iExits;
        ZIndex  m_iEntrances;
        ZIndex  m_iFirstNode;
        ZIndex  m_iFirstVertex;
        ZIndex  m_iFirstComponent;
        ZUIndex m_iFirstExitDist;
        ZIndex  m_iSplitTree;
        ZIndex  m_iHeightTree;
        ZIndex  m_iEquations;
        ZVector3 m_Min;
        ZVector3 m_Max;
    };
    RE_VERIFY_SIZE(ZGraph, 0x30); // Confirmed (PC pf4runtime)

    class ZData : public ZInterface
    {
    public:
        ZData();
        ~ZData() override;

        // vtbl
        float* MapNodeIdx(ZDataRef rRef, float*, float*, EPathWayActions&, unsigned int&) override;
        int GetMetaId() override;
        void AddNode(ZMetaNode* pNode, const ZLocation& kLocation) override;
        ZMetaNode* RemoveNode(ZMetaNode* pNode) override;
        void MoveNodeConstrained(ZMetaNode* pNode, const ZVector3& vPos) override;
        bool TeleportNode(ZMetaNode* pNode, const ZVector3& vPos) override;
        int FindNodes(const ZLocation& kSource, ZResult* pList, int iMaxEntities, float fMaxDistance, int type) override;
        void AddObstacle(ZDynamicObstacle* pObstacle2, const ZLocation& kLocation) override;
        void RemoveObstacle(ZDynamicObstacle* pObstacle2) override;
        void MoveObstacle(ZDynamicObstacle* pObstacle2, const ZLocation& kPos) override;
        void AddObstacle(ZDynamicObstacle* pObstacle2, const ZVector3& vPos) override;
        void MoveObstacle(ZDynamicObstacle* pObstacle2, const ZVector3& vPos) override;
        void PushOutOfObstacles(ZMetaNode* pNode, int iObstacleTypeMask, ZVector3& vPos) override;
        bool HasObstacles() override;
        bool FindPath(ZPathRequest* pRequest) override;
        void FreePath(ZPath* pPath) override;
        bool AllocateBufferPath(const ZPath& sPath, ZPath* pResult) override;
        float FindPathLength(ZPathRequest* pRequest) override;
        bool PositionInside(const ZVector3& vPos) override;
        bool MapLocation(const ZVector3& vPos, ZLocation& kLocation) override;
        bool MapValidLocation(const ZVector3& vPos, ZLocation& kLocation) override;
        bool MapInside(const ZLocation& kSource, ZLocation& kNewTarget, bool bUseObstacle) override;
        float ComponentHeight(int iComponent, float x, float z) override;
        bool MapOntoComponent(const ZLocation& kSource, ZLocation& kTarget) override;
        int FindComponents(const ZLocation& kSource, int* pList, int iMax, float fMaxDistance) override;
        int CloseExit(float*) override;
        int CloseLinks(const ZVector3& vMin, const ZVector3& vMax, uint32_t lNewKeyMask) override;
        int OpenLinks(const ZVector3& vMin, const ZVector3& vMax, uint32_t lNewKeyMask) override;
        int GraphCount() override;
        void FindCornersInGraph(int, int&, float*) override;
        float FindWallIntersection(
            const ZLocation& kLoc,
            const ZVector3& vEndPoint,
            float fMaxDistance,
            const ZVector3&,
            ZVector3&,
            bool& bHitWall,
            ZLocation& endLocation,
            bool bReportDoorsAsWalls,
            bool bIgnoreObstacles) override;
        int FindWalls(const ZLocation& kSource, float fMaxDistance, float* pWall) override;
        int FindObstacles(const ZLocation& kLocation, ZDynamicObstacle** obstacles2, int max) override;
        void RemapDoorRefs(ZREF* pRefs, uint32_t lRefsNr) override;

        // methods
        bool FindPath(const ZLocation&, const ZLocation&, ZPath&, ZPathLink*, int&, bool, unsigned int);
        void AssignGraph(ZLocation& kLocation);
        bool GetGraphAndComponent(const float* pvPosition, ZIndex& rGraph, ZIndex& rComponent);
        bool GetClosestGraphAndComponent(
            const float* pvPosition, float* pvClosest, ZIndex& rGraph, ZIndex& rComponent, bool bSkipFallback);
        int FindComponentPathAStar(const ZLocation&, const ZLocation&, ZPath&, ZPathLink*, unsigned int);
        int TraceExit(int iGraph, int iFromNode, int iToNode, int* pOut, int* pOutEnd);
        int StraightenGates(const float*, const float*, int*, int, ZPathLink*, ZPath&);
        void LoadDataBlock(void* pData);

        void GetIndex(void*& pBuffer, ZIndex& index) const;
        void GetIndex(void*& pBuffer, ZUIndex& index) const;
        void* GetArray(void*& pBuffer, int lLength) const;
        int SplitTreeCount() const;
        int HeightTreeCount() const;
        int PlaneEquationCount() const;
        int ComponentCount() const;
        int NodeCount() const;
        int VertexCount() const;
        int LinkCount() const;

        // members
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
    RE_VERIFY_SIZE(ZData, 0x5E80); // PC alloc verified
}
