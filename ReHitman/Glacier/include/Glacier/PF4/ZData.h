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
        void Init(int16_t* pData, ZDataRef* pDRef);
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

    class ZData : public ZInterface
    {
    public:
        // vtbl
        ~ZData() override;
        int MapNodeIdx(ZDataRef rRef, float*, float*, EPathWayActions&, unsigned int&) override;
        int GetMetaId() override;
        void AddNode(ZMetaNode* pNode, const ZLocation& kLocation) override;
        void RemoveNode(ZMetaNode* pNode) override;
        void MoveNodeConstrained(ZMetaNode* pNode, const ZVector3& vPos) override;
        void TeleportNode(ZMetaNode* pNode, const ZVector3& vPos) override;
        int FindNodes(const ZLocation& kSource, ZResult* pList, int iMaxEntities, float fMaxDistance, int type) override;
        void AddObstacle(ZDynamicObstacle* pObstacle2, const ZLocation& kLocation) override;
        void RemoveObstacle(ZDynamicObstacle* pObstacle2) override;
        void MoveObstacle(ZDynamicObstacle* pObstacle2, const ZLocation& kPos) override;
        void AddObstacle(ZDynamicObstacle* pObstacle2, const ZVector3& vPos) override;
        void MoveObstacle(ZDynamicObstacle* pObstacle2, const ZVector3& vPos) override;
        void PushOutOfObstacles(ZMetaNode* pNode, int iObstacleTypeMask, const ZVector3& vPos) override;
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
        void AssignGraph(ZLocation&);
        bool GetGraphAndComponent(const float*, ZIndex&, ZIndex&);
        bool GetClosestGraphAndComponent(const float*, float*, ZIndex&, ZIndex&, bool);
        int FindComponentPathAStar(const ZLocation&, const ZLocation&, ZPath&, ZPathLink*, unsigned int);
        int TraceExit(int, int, int, int*);
        int StraightenGates(const float*, const float*, int*, int, ZPathLink*, ZPath&);

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
