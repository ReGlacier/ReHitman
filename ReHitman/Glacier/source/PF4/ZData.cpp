#include <Glacier/PF4/ZData.h>
#include <Glacier/PF4/ZDynamicObstacle.h>
#include <Glacier/PF4/ZPath.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <algorithm>
#include <cmath>
#include <cstring>


namespace Glacier::PF4
{
    namespace
    {
        // Component height at (x, z): walks the component height tree and
        // evaluates the resulting plane equation (PC 004D9900).
        float ComponentHeightAt(const ZData& rData, const ZComponent& rComponent, float x, float z)
        {
            const int iFirstHeightTree = rComponent.m_FirstHeightTree;
            if (iFirstHeightTree < 0)
            {
                return 0.0f;
            }

            int iOffset = 0;
            for (;;)
            {
                const int iTree = iFirstHeightTree + iOffset;
                const ZSplitTree& heightTree = rData.m_pHeightTrees[iTree];
                const bool bSide = (x * heightTree.m_fA + z * heightTree.m_fB >= heightTree.m_fC);
                const int iChild = heightTree.m_iChild[bSide ? 1 : 0];
                if (iChild < 0)
                {
                    const int iPlaneEquation = -1 - iChild;
                    if (iPlaneEquation >= 0 && rData.m_pPlaneEquations)
                    {
                        const ZPlaneEquation& plane = rData.m_pPlaneEquations[iPlaneEquation];
                        return z * plane.m_fB + x * plane.m_fA + plane.m_fC;
                    }
                    return 0.0f;
                }
                iOffset = iChild;
            }
        }

        // Component containing (x, z) inside a graph: walks the graph split tree (PC 004D99C0).
        int GraphGetComponent(const ZData& rData, const ZGraph& rGraph, float x, float z)
        {
            if (rGraph.m_iSplitTree == -1)
            {
                return -1;
            }

            int iTree = rGraph.m_iSplitTree;
            for (;;)
            {
                const ZSplitTree& splitTree = rData.m_pSplitTrees[iTree];
                const bool bSide = (x * splitTree.m_fA + z * splitTree.m_fB >= splitTree.m_fC);
                const int iChild = splitTree.m_iChild[bSide ? 1 : 0];
                if (iChild < 0)
                {
                    return -2 - iChild;
                }
                iTree = iChild;
            }
        }

        bool BBoxContainsPoint(const ZVector3& vMin, const ZVector3& vMax, const float* pvPos, float fMargin)
        {
            return fMargin + pvPos[0] >= vMin.x && pvPos[0] - fMargin <= vMax.x
                && fMargin + pvPos[1] >= vMin.y && pvPos[1] - fMargin <= vMax.y
                && fMargin + pvPos[2] >= vMin.z && pvPos[2] - fMargin <= vMax.z;
        }

        // Empty/inverted box used when accumulating a region from points (PC 004DBC00).
        void BoxReset(float (&fBox)[6])
        {
            fBox[0] = 3.4028235e38f;
            fBox[1] = 3.4028235e38f;
            fBox[2] = 3.4028235e38f;
            fBox[3] = -3.4028235e38f;
            fBox[4] = -3.4028235e38f;
            fBox[5] = -3.4028235e38f;
        }

        // Grows the box to contain a point expanded by (1, 100, 1) margins (PC 004DBC60).
        void BoxAddPoint(float (&fBox)[6], const float* pvPos)
        {
            fBox[0] = std::min(fBox[0], pvPos[0] - 1.0f);
            fBox[1] = std::min(fBox[1], pvPos[1] - 100.0f);
            fBox[2] = std::min(fBox[2], pvPos[2] - 1.0f);
            fBox[3] = std::max(fBox[3], pvPos[0] + 1.0f);
            fBox[4] = std::max(fBox[4], pvPos[1] + 100.0f);
            fBox[5] = std::max(fBox[5], pvPos[2] + 1.0f);
        }

        const ZNode& LinkToNode(const ZData& rData, const ZLink& rLink)
        {
            const ZGraph& graph = rData.m_pGraphs[rLink.m_Graph];
            return rData.m_pNodes[rLink.m_iNode + graph.m_iFirstNode];
        }

        // ZDataRef encode/decode helpers used by the path building code.
        inline uint32_t MakeHandle(uint32_t lId, uint32_t lType)
        {
            return (lId & 0x3FFF) | ((lType & 3) << 14);
        }

        inline uint32_t HandleOf(const ZDataRef& rRef)
        {
            return (rRef.m_Id & 0x3FFF) | ((static_cast<uint32_t>(rRef.m_Type) & 3) << 14);
        }

        inline ZDataRef MakeRef(uint32_t lHandle, float fX, float fZ)
        {
            ZDataRef rRef{};
            rRef.m_Id = lHandle & 0x3FFF;
            rRef.m_Type = (lHandle >> 14) & 3;
            rRef.m_Pos.x = fX;
            rRef.m_Pos.y = fZ;
            return rRef;
        }

        inline float Dist2DSq(float fAx, float fAz, float fBx, float fBz)
        {
            const float fDx = fAx - fBx;
            const float fDz = fAz - fBz;
            return fDx * fDx + fDz * fDz;
        }

        // Closest component to the given point. Returns global component index or -1 (PC 004DD2E0).
        int GraphGetClosestComponent(
            const ZData& rData, const ZGraph& rGraph, const float* pvPos, float& fDistOut, float* pvClosest)
        {
            const float fX = pvPos[0];
            const float fY = pvPos[1];
            const float fZ = pvPos[2];
            pvClosest[0] = fX;
            pvClosest[1] = fY;
            pvClosest[2] = fZ;

            const int iComponent = GraphGetComponent(rData, rGraph, fX, fZ);
            if (iComponent != -1)
            {
                const float fHeight = ComponentHeightAt(rData, rData.m_pComponents[iComponent], fX, fZ);
                fDistOut = std::sqrt((fY - fHeight) * (fY - fHeight));
                return iComponent;
            }

            fDistOut = 3.4028235e38f;
            int iResult = -1;
            float fBestX = fX;
            float fBestY = fY;
            float fBestZ = fZ;
            for (int iGraphComponent = 0; iGraphComponent < rGraph.m_iComponents; ++iGraphComponent)
            {
                const ZComponent& rComponent = rData.m_pComponents[iGraphComponent + rGraph.m_iFirstComponent];
                const int iCorners = rComponent.m_Corners;
                for (int iCorner = 0; iCorner < iCorners; ++iCorner)
                {
                    const ZCorner& corner = rData.m_pCorners[rComponent.m_FirstCorner + iCorner];
                    if (corner.m_iComponent >= 0)
                    {
                        continue;
                    }

                    const int iNextCorner = (iCorner + 1 >= iCorners) ? 0 : iCorner + 1;
                    const ZCorner& nextCorner = rData.m_pCorners[rComponent.m_FirstCorner + iNextCorner];

                    const ZVertex& vertexA = rData.m_pVertices[corner.m_iVertex];
                    const ZVertex& vertexB = rData.m_pVertices[nextCorner.m_iVertex];
                    const float fAx = vertexA.m_kPos.x;
                    const float fAz = vertexA.m_kPos.y;
                    const float fAh = vertexA.m_fHeight;
                    const float fBx = vertexB.m_kPos.x;
                    const float fBz = vertexB.m_kPos.y;
                    const float fBh = vertexB.m_fHeight;

                    const float fEx = fBx - fAx;
                    const float fEz = fBz - fAz;
                    const float fDx = fX - fAx;
                    const float fDz = fZ - fAz;
                    const float fT = fDz * fEz + fDx * fEx;
                    const float fSegLen2 = fEz * fEz + fEx * fEx;

                    float fClosestX;
                    float fClosestZ;
                    float fClosestH;
                    float fDist;
                    if (fT >= 0.0f)
                    {
                        if (fT <= fSegLen2)
                        {
                            const float fS = fT / fSegLen2;
                            fClosestX = fAx + fEx * fS;
                            fClosestZ = fAz + fEz * fS;
                            fClosestH = (fBh - fAh) * fS + fAh;
                            fDist = std::sqrt(
                                (fZ - fClosestZ) * (fZ - fClosestZ)
                                + (fY - fClosestH) * (fY - fClosestH)
                                + (fX - fClosestX) * (fX - fClosestX));
                        }
                        else
                        {
                            fClosestX = fBx;
                            fClosestZ = fBz;
                            fClosestH = fBh;
                            fDist = std::sqrt(
                                (fZ - fBz) * (fZ - fBz)
                                + (fY - fBh) * (fY - fBh)
                                + (fX - fBx) * (fX - fBx));
                        }
                    }
                    else
                    {
                        fClosestX = fAx;
                        fClosestZ = fAz;
                        fClosestH = fAh;
                        fDist = std::sqrt(
                            (fZ - fAz) * (fZ - fAz)
                            + (fY - fAh) * (fY - fAh)
                            + (fX - fAx) * (fX - fAx));
                    }

                    if (fDist < fDistOut)
                    {
                        fDistOut = fDist;
                        iResult = iGraphComponent + rGraph.m_iFirstComponent;
                        fBestX = fClosestX;
                        fBestY = fClosestH;
                        fBestZ = fClosestZ;
                    }
                }
            }

            pvClosest[0] = fBestX;
            pvClosest[1] = fBestY;
            pvClosest[2] = fBestZ;
            return iResult;
        }
    }

    // ZData

    // Debug command that turns on the path line visualisation (PC 0x9725A4).
    ZDebugInt g_lDisplayPathLinesCommand("pf4ShowPath", "Draw paths as they are found. Hold SHIFT to clear all lines.", 0, 0, 1, 1, "PathFinder/");

    ZData::ZData()
    {
        // Base arrays
        m_pGraphs = nullptr;
        m_pNodes = nullptr;
        m_pVertices = nullptr;
        m_pLinks = nullptr;
        m_pComponents = nullptr;
        m_pSplitTrees = nullptr;
        m_pHeightTrees = nullptr;
        m_pPlaneEquations = nullptr;
        m_pExitDists = nullptr;
        m_pCorners = nullptr;
        m_pSubNodes = nullptr;
        m_pStaticObstacleIds = nullptr;
        m_pStaticObstacles = nullptr;

        m_iSplitTreeCount = 0;
        m_iHeightTreeCount = 0;
        m_iPlaneEquationCount = 0;
        m_iComponentCount = 0;
        m_iGraphCount = 0;
        m_iNodeCount = 0;
        m_iVertexCount = 0;
        m_iLinkCount = 0;
        m_iExitDistCount = 0;
        m_iCornerCount = 0;
        m_iSubNodeCount = 0;
        m_iStaticObstacleIdCount = 0;
        m_iStaticObstacleCount = 0;

        m_pDynamicObstacles = nullptr;
        m_ObstacleCount = 0;
        m_iMetaId = 0;
        m_pComponentVisited = nullptr;
        m_iComponentVisitedSize = 0;
        m_pNodeData = nullptr;

        m_kOpenList.m_pNodeData = nullptr;
        m_kOpenList.m_iNodeCount = 0;
        m_kOpenList.m_iOpenNodeCount = 0;
        m_kOpenList.m_iClosedNodeCount = 0;

        const int aiBlockSize[3] = { 10, 30, 200 };
        const int aiMaxBlocks[3] = { 20, 20, 10 };
        for (int i = 0; i < 3; ++i)
        {
            m_Allocator[i].m_BlockSize = aiBlockSize[i];
            m_Allocator[i].m_MaxBlocks = aiMaxBlocks[i];
        }

        m_BlockStack = static_cast<int16_t*>(ZUniMemory::Allocate(100));
        m_DataRefs = static_cast<ZDataRef*>(ZUniMemory::Allocate(33600));

        int iStackOffset = 0;
        int iDataRefsOffset = 0;
        for (int i = 0; i < 3; ++i)
        {
            m_Allocator[i].Init(&m_BlockStack[iStackOffset], &m_DataRefs[iDataRefsOffset]);
            iStackOffset += m_Allocator[i].m_MaxBlocks;
            iDataRefsOffset += m_Allocator[i].m_MaxBlocks * m_Allocator[i].m_BlockSize;
        }
    }

    ZData::~ZData()
    {
        // Frees all run-time buffers allocated by ZData()/LoadDataBlock (PC 004DB500 Data::Dtor)
        if (m_pDynamicObstacles)
        {
            ZUniMemory::Free(m_pDynamicObstacles);
        }
        if (m_pComponentVisited)
        {
            ZUniMemory::Free(m_pComponentVisited);
        }
        if (m_pNodeData)
        {
            ZUniMemory::Free(m_pNodeData);
        }
        if (m_BlockStack)
        {
            ZUniMemory::Free(m_BlockStack);
        }
        if (m_DataRefs)
        {
            ZUniMemory::Free(m_DataRefs);
        }
    }

    void ZData::LoadDataBlock(void* pData)
    {
        auto* pBlock = static_cast<SPF4DataBlock*>(pData);

        m_iSplitTreeCount = pBlock->m_iSplitTreeCount;
        m_iHeightTreeCount = pBlock->m_iHeightTreeCount;
        m_iPlaneEquationCount = pBlock->m_iPlaneEquationCount;
        m_iComponentCount = pBlock->m_iComponentCount;
        m_iGraphCount = pBlock->m_iGraphCount;
        m_iNodeCount = pBlock->m_iNodeCount;
        m_iVertexCount = pBlock->m_iVertexCount;
        m_iLinkCount = pBlock->m_iLinkCount;
        m_iExitDistCount = pBlock->m_iExitDistCount;
        m_iCornerCount = pBlock->m_iCornerCount;
        m_iSubNodeCount = pBlock->m_iSubNodeCount;
        m_iStaticObstacleIdCount = pBlock->m_iStaticObstacleIdCount;
        m_iStaticObstacleCount = pBlock->m_iStaticObstacleCount;

        // Everything past the header is a tight packed sequence of arrays.
        char* pCursor = reinterpret_cast<char*>(pBlock + 1);

        m_pSplitTrees = (m_iSplitTreeCount != 0) ? reinterpret_cast<ZSplitTree*>(pCursor) : nullptr;
        pCursor += 0x10 * m_iSplitTreeCount;

        m_pHeightTrees = (m_iHeightTreeCount != 0) ? reinterpret_cast<ZSplitTree*>(pCursor) : nullptr;
        pCursor += 0x10 * m_iHeightTreeCount;

        m_pPlaneEquations = (m_iPlaneEquationCount != 0) ? reinterpret_cast<ZPlaneEquation*>(pCursor) : nullptr;
        pCursor += 0xC * m_iPlaneEquationCount;

        m_pComponents = (m_iComponentCount != 0) ? reinterpret_cast<ZComponent*>(pCursor) : nullptr;
        pCursor += 0x14 * m_iComponentCount;

        m_pGraphs = (m_iGraphCount != 0) ? reinterpret_cast<ZGraph*>(pCursor) : nullptr;
        pCursor += 0x30 * m_iGraphCount;

        m_pNodes = (m_iNodeCount != 0) ? reinterpret_cast<ZNode*>(pCursor) : nullptr;
        pCursor += 0x14 * m_iNodeCount;

        m_pVertices = (m_iVertexCount != 0) ? reinterpret_cast<ZVertex*>(pCursor) : nullptr;
        pCursor += 0x14 * m_iVertexCount;

        m_pLinks = (m_iLinkCount != 0) ? reinterpret_cast<ZLink*>(pCursor) : nullptr;
        pCursor += 0x10 * m_iLinkCount;

        m_pExitDists = (m_iExitDistCount != 0) ? reinterpret_cast<ZIndex*>(pCursor) : nullptr;
        pCursor += 0x2 * m_iExitDistCount;

        m_pCorners = (m_iCornerCount != 0) ? reinterpret_cast<ZCorner*>(pCursor) : nullptr;
        pCursor += 0x4 * m_iCornerCount;

        m_pSubNodes = (m_iSubNodeCount != 0) ? reinterpret_cast<ZSubNode*>(pCursor) : nullptr;
        pCursor += 0x8 * m_iSubNodeCount;

        m_pStaticObstacleIds = (m_iStaticObstacleIdCount != 0) ? reinterpret_cast<ZIndex*>(pCursor) : nullptr;
        pCursor += 0x2 * m_iStaticObstacleIdCount;

        m_pStaticObstacles = (m_iStaticObstacleCount != 0) ? reinterpret_cast<ZStaticObstacle*>(pCursor) : nullptr;
        pCursor += 0x4 * m_iStaticObstacleCount;

        // Runtime node data for the A* open list
        if (m_iNodeCount)
        {
            m_pNodeData = static_cast<ZNodeData*>(ZUniMemory::Allocate(0x2 * m_iNodeCount));

            m_kOpenList.m_pNodeData = m_pNodeData;
            m_kOpenList.m_iNodeCount = m_iNodeCount;
            std::memset(m_pNodeData, 0, 0x2 * m_iNodeCount);
            for (int i = 0; i < m_kOpenList.m_iNodeCount; ++i)
            {
                m_pNodeData[i].iOpenNode = 0xFFFF;
            }
            std::memset(m_kOpenList.m_aList, 0, sizeof(m_kOpenList.m_aList));
            for (int i = 0; i < 1000; ++i)
            {
                m_kOpenList.m_aList[i].iNode = 0xFFFF;
                m_kOpenList.m_aList[i].iParent = 0xFFFF;
            }
            m_kOpenList.m_iOpenNodeCount = 0;
            m_kOpenList.m_iClosedNodeCount = 0;
        }

        // Per-component visited flags used during component flood fill
        if (m_iComponentCount)
        {
            m_ObstacleCount = 0;
            m_pDynamicObstacles = static_cast<ZLink**>(ZUniMemory::Allocate(0x4 * m_iComponentCount));
            std::memset(m_pDynamicObstacles, 0, 0x4 * m_iComponentCount);

            m_iComponentVisitedSize = (m_iComponentCount + 31) / 32;
            m_pComponentVisited = static_cast<uint32_t*>(ZUniMemory::Allocate(0x4 * m_iComponentVisitedSize));
        }
    }

    void ZData::GetIndex(void*& pBuffer, ZIndex& index) const
    {
        index = *static_cast<ZIndex*>(pBuffer);
        pBuffer = static_cast<char*>(pBuffer) + 2;
    }

    void ZData::GetIndex(void*& pBuffer, ZUIndex& index) const
    {
        index = *static_cast<ZUIndex*>(pBuffer);
        pBuffer = static_cast<char*>(pBuffer) + 2;
    }

    void* ZData::GetArray(void*& pBuffer, int lLength) const
    {
        void* pArray = pBuffer;
        pBuffer = static_cast<char*>(pBuffer) + lLength;
        return pArray;
    }

    int ZData::SplitTreeCount() const
    {
        return m_iSplitTreeCount;
    }

    int ZData::HeightTreeCount() const
    {
        return m_iHeightTreeCount;
    }

    int ZData::PlaneEquationCount() const
    {
        return m_iPlaneEquationCount;
    }

    int ZData::ComponentCount() const
    {
        return m_iComponentCount;
    }

    int ZData::NodeCount() const
    {
        return m_iNodeCount;
    }

    int ZData::VertexCount() const
    {
        return m_iVertexCount;
    }

    int ZData::LinkCount() const
    {
        return m_iLinkCount;
    }

    float* ZData::MapNodeIdx(ZDataRef rRef, float* pPos, float* pNormal, EPathWayActions& outAction, unsigned int& outData)
    {
        switch (rRef.m_Type)
        {
        case 0: // node
            outAction = PWA_DEFAULT;
            {
                const ZNode& node = m_pNodes[rRef.m_Id];
                pPos[0] = node.m_kPos.x;
                pPos[1] = node.m_fHeight;
                pPos[2] = node.m_kPos.y;
            }
            outData = 0;
            return pPos;

        case 1: // vertex
            outAction = PWA_DEFAULT;
            {
                const ZVertex& vertex = m_pVertices[rRef.m_Id];
                pPos[0] = vertex.m_kPos.x;
                pPos[1] = vertex.m_fHeight;
                pPos[2] = vertex.m_kPos.y;
                pNormal[0] = vertex.m_kNormal.x;
                pNormal[1] = 0.0f;
                pNormal[2] = vertex.m_kNormal.y;
            }
            outData = 0;
            return pNormal;

        case 2: // link
            {
                const ZLink& link = m_pLinks[rRef.m_Id];
                outAction = static_cast<EPathWayActions>(link.m_iAction);
                outData = link.m_rDoorController;
                const ZGraph& graph = m_pGraphs[link.m_Graph];
                const ZNode& node = m_pNodes[link.m_iNode + graph.m_iFirstNode];
                pPos[0] = node.m_kPos.x;
                pPos[1] = node.m_fHeight;
                pPos[2] = node.m_kPos.y;
            }
            return pPos;

        default:
            outAction = PWA_DEFAULT;
            outData = 0;
            return pPos;
        }
    }

    int ZData::GetMetaId()
    {
        return m_iMetaId;
    }

    void ZData::AddNode(ZMetaNode* pNode, const ZLocation& kLocation)
    {
        if (pNode->m_Location.Component() != -1 || kLocation.Component() == -1)
        {
            return;
        }

        ZLocation mappedLocation;
        MapOntoComponent(kLocation, mappedLocation);

        pNode->m_Location = mappedLocation;

        const int component = mappedLocation.Component();
        ZMetaNode* pMetaNodes = m_pComponents[component].m_MetaNodes;
        pNode->m_Next = pMetaNodes;
        pNode->m_Prev = nullptr;
        if (pMetaNodes)
        {
            pMetaNodes->m_Prev = pNode;
        }
        m_pComponents[component].m_MetaNodes = pNode;
    }

    ZMetaNode* ZData::RemoveNode(ZMetaNode* pNode)
    {
        const int component = pNode->m_Location.Component();
        if (component != -1)
        {
            if (pNode->m_Next)
            {
                pNode->m_Next->m_Prev = pNode->m_Prev;
            }
            if (pNode->m_Prev)
            {
                pNode->m_Prev->m_Next = pNode->m_Next;
            }
            if (m_pComponents[component].m_MetaNodes == pNode)
            {
                m_pComponents[component].m_MetaNodes = pNode->m_Next;
            }

            pNode->m_Next = nullptr;
            pNode->m_Prev = nullptr;
            pNode->m_Location.m_Component = -1;
            pNode->m_Location.m_Inside = 0;
            pNode->m_Location.m_Graph = -1;
        }

        return pNode;
    }

    void ZData::MoveNodeConstrained(ZMetaNode*, const ZVector3&)
    {
        // TODO: Finish me (PC 004DF240)
    }

    bool ZData::TeleportNode(ZMetaNode* pNode, const ZVector3& vPos)
    {
        // Moves a meta node to a new world position (PC 004DC990). When the node is
        // already placed close by the node is nudged without touching the lists;
        // otherwise it is re-registered in the component containing vPos.
        if (pNode->m_Location.Component() != -1)
        {
            const ZVector3& vNodePos = pNode->m_Location.m_vPos;
            const float fDx = vNodePos.x - vPos.x;
            const float fDz = vNodePos.z - vPos.z;
            if (fDx * fDx + fDz * fDz < 10000.0f && std::fabs(vNodePos.y - vPos.y) < 100.0f)
            {
                MoveNodeConstrained(pNode, vPos);
                return true;
            }

            ZLocation location;
            if (!MapValidLocation(vPos, location))
            {
                return false;
            }
            RemoveNode(pNode);
            MapOntoComponent(location, location);
            AddNode(pNode, location);
            return location.m_Inside != 0;
        }

        ZLocation location;
        MapLocation(vPos, location);
        if (location.Graph() == -1)
        {
            return false;
        }
        MapOntoComponent(location, location);
        AddNode(pNode, location);
        return location.m_Inside != 0;
    }

    int ZData::FindNodes(const ZLocation& kSource, ZResult* pList, int iMaxEntities, float fMaxDistance, int iType)
    {
        // Flood fills components reachable from the query location through corner
        // gates that lie within fMaxDistance, collecting every meta node of the
        // visited components (PC 004DF690). Reachability is a 3D point-to-gate
        // distance test on the gate's two corner vertices.
        if (kSource.Component() == -1)
        {
            return 0;
        }

        std::memset(m_pComponentVisited, 0, 4u * m_iComponentVisitedSize);

        const float fMaxDistance2 = fMaxDistance * fMaxDistance;
        const ZVector3& rQuery = kSource.m_vPos;

        int aStack[300];
        int iStackSize = 0;
        aStack[iStackSize++] = kSource.Component();

        int iCount = 0;
        while (iStackSize > 0)
        {
            const int iComponent = aStack[--iStackSize];
            m_pComponentVisited[iComponent >> 5] |= 1u << (iComponent & 0x1F);

            const ZComponent& rComponent = m_pComponents[iComponent];

            ZMetaNode* pNode = rComponent.m_MetaNodes;
            while (pNode)
            {
                if (iType == -1 || pNode->m_Type == iType)
                {
                    const ZVector3& rNodePos = pNode->m_Location.m_vPos;
                    const float fDist = std::sqrt(
                        Dist2DSq(rQuery.x, rQuery.z, rNodePos.x, rNodePos.z));
                    pList[iCount].fDistance = fDist;
                    if (fDist < fMaxDistance)
                    {
                        pList[iCount].pNode = pNode;
                        ++iCount;
                        if (iCount >= iMaxEntities)
                        {
                            return iCount;
                        }
                    }
                }
                pNode = pNode->m_Next;
            }

            const int iCornerCount = rComponent.m_Corners;
            if (iCornerCount <= 0)
            {
                continue;
            }

            const int iFirstCorner = rComponent.m_FirstCorner;
            for (int iCorner = 0; iCorner < iCornerCount; ++iCorner)
            {
                const ZCorner& rCornerA = m_pCorners[iFirstCorner + iCorner];
                int iNeighbor = rCornerA.m_iComponent;
                if (iNeighbor == -1)
                {
                    continue;
                }
                if (iNeighbor < 0)
                {
                    iNeighbor = -2 - iNeighbor;
                }
                if (iNeighbor < 0 || (m_pComponentVisited[iNeighbor >> 5] & (1u << (iNeighbor & 0x1F))) != 0)
                {
                    continue;
                }

                const int iNextCorner = (iCorner + 1 == iCornerCount) ? 0 : iCorner + 1;
                const ZCorner& rCornerB = m_pCorners[iFirstCorner + iNextCorner];

                const ZVertex& rVertexA = m_pVertices[rCornerA.m_iVertex];
                const ZVertex& rVertexB = m_pVertices[rCornerB.m_iVertex];
                const float fAx = rVertexA.m_kPos.x;
                const float fAh = rVertexA.m_fHeight;
                const float fAz = rVertexA.m_kPos.y;
                const float fBx = rVertexB.m_kPos.x;
                const float fBh = rVertexB.m_fHeight;
                const float fBz = rVertexB.m_kPos.y;

                const float fEx = fBx - fAx;
                const float fEh = fBh - fAh;
                const float fEz = fBz - fAz;
                const float fSegLen2 = fEx * fEx + fEh * fEh + fEz * fEz;

                // Squared 3D distance from the query position to the gate segment
                // (clamped so the segment end points are included).
                float fCx = fAx;
                float fCy = fAh;
                float fCz = fAz;
                if (fSegLen2 > 0.0f)
                {
                    const float fT =
                        ((rQuery.x - fAx) * fEx + (rQuery.y - fAh) * fEh + (rQuery.z - fAz) * fEz)
                        / fSegLen2;
                    if (fT > 0.0f)
                    {
                        if (fT < 1.0f)
                        {
                            fCx = fAx + fEx * fT;
                            fCy = fAh + fEh * fT;
                            fCz = fAz + fEz * fT;
                        }
                        else
                        {
                            fCx = fBx;
                            fCy = fBh;
                            fCz = fBz;
                        }
                    }
                }
                const float fPx = fCx - rQuery.x;
                const float fPy = fCy - rQuery.y;
                const float fPz = fCz - rQuery.z;
                const float fDist2 = fPx * fPx + fPy * fPy + fPz * fPz;

                if (fDist2 <= fMaxDistance2 && iStackSize < 300)
                {
                    aStack[iStackSize++] = iNeighbor;
                }
            }
        }

        return iCount;
    }

    void ZData::AddObstacle(ZDynamicObstacle* pObstacle, const ZLocation& kLocation)
    {
        // Registers the obstacle with every component found near its location.
        // Each registration is a dedicated link node stored in the obstacle's
        // m_Linking array, inserted into the per-component list heads kept in
        // m_pDynamicObstacles (PC 004DD990).
        if (pObstacle->m_Location.Component() != -1 || kLocation.Component() == -1 || pObstacle->m_iLinks != 0)
        {
            return;
        }

        auto** pHeads = reinterpret_cast<ZDynamicObstacle_Link**>(m_pDynamicObstacles);
        if (!pHeads)
        {
            return;
        }

        ZLocation mapped;
        if (!MapOntoComponent(kLocation, mapped))
        {
            return;
        }

        pObstacle->m_Location = mapped;

        int vComponents[16];
        const int iCount = FindComponents(mapped, vComponents, 16, pObstacle->m_Radius);
        for (int i = 0; i < iCount && pObstacle->m_iLinks < 16; ++i)
        {
            const int iComponent = vComponents[i];
            if (iComponent < 0 || iComponent >= m_iComponentCount)
            {
                continue;
            }

            const int iLink = pObstacle->m_iLinks;
            reinterpret_cast<int32_t*>(pObstacle->m_Component)[iLink] = iComponent;

            ZDynamicObstacle_Link* pNode = &pObstacle->m_Linking[iLink];
            pNode->m_This = pObstacle;
            pNode->m_Next = pHeads[iComponent];
            pNode->m_Prev = nullptr;
            if (pNode->m_Next)
            {
                pNode->m_Next->m_Prev = pNode;
            }
            pHeads[iComponent] = pNode;
            ++pObstacle->m_iLinks;
        }

        ++m_ObstacleCount;
    }

    void ZData::RemoveObstacle(ZDynamicObstacle* pObstacle)
    {
        // Unlinks every registration link node and clears the obstacle's graph
        // location (PC 004DB8A0).
        if (!pObstacle || pObstacle->m_Location.Component() == -1)
        {
            return;
        }

        auto** pHeads = reinterpret_cast<ZDynamicObstacle_Link**>(m_pDynamicObstacles);
        if (pHeads)
        {
            auto* pComponents = reinterpret_cast<int32_t*>(pObstacle->m_Component);
            ZDynamicObstacle_Link* pNode = pObstacle->m_Linking;
            for (int i = 0; i < pObstacle->m_iLinks; ++i)
            {
                const int iComponent = pComponents[i];
                if (pNode->m_Next)
                {
                    pNode->m_Next->m_Prev = pNode->m_Prev;
                }
                if (pNode->m_Prev)
                {
                    pNode->m_Prev->m_Next = pNode->m_Next;
                }
                if (iComponent >= 0 && iComponent < m_iComponentCount && pHeads[iComponent] == pNode)
                {
                    pHeads[iComponent] = pNode->m_Next;
                }
                ++pNode;
            }
        }

        pObstacle->m_iLinks = 0;
        pObstacle->m_Location.m_Component = -1;
        pObstacle->m_Location.m_Graph = -1;
        pObstacle->m_Location.m_Inside = 0;
        --m_ObstacleCount;
    }

    void ZData::MoveObstacle(ZDynamicObstacle* pObstacle, const ZLocation& kLocation)
    {
        // Unregisters the obstacle from its old components and registers it at
        // the new graph location (PC 004DDA90).
        RemoveObstacle(pObstacle);
        AddObstacle(pObstacle, kLocation);
    }

    void ZData::AddObstacle(ZDynamicObstacle* pObstacle, const ZVector3& vPos)
    {
        // Maps the world position onto the navmesh and registers the obstacle
        // there (PC 004DDAE0).
        ZLocation location;
        if (MapLocation(vPos, location))
        {
            AddObstacle(pObstacle, location);
        }
    }

    void ZData::MoveObstacle(ZDynamicObstacle* pObstacle, const ZVector3& vPos)
    {
        // Maps the world position onto the navmesh and moves the obstacle there
        // (PC 004DA140).
        ZLocation location;
        if (MapLocation(vPos, location))
        {
            MoveObstacle(pObstacle, location);
        }
    }

    void ZData::PushOutOfObstacles(ZMetaNode* pNode, int iObstacleTypeMask, ZVector3& vPos)
    {
        // Pushes the given position out of dynamic obstacles of the matching
        // type that contain it (PC 004DCA70).
        const int iComponent = pNode ? pNode->m_Location.Component() : -1;
        if (iComponent < 0 || iComponent >= m_iComponentCount)
        {
            return;
        }

        auto** pHeads = reinterpret_cast<ZDynamicObstacle_Link**>(m_pDynamicObstacles);
        if (!pHeads)
        {
            return;
        }

        float fX = vPos.x;
        float fZ = vPos.z;
        for (ZDynamicObstacle_Link* pNodeLink = pHeads[iComponent]; pNodeLink; pNodeLink = pNodeLink->m_Next)
        {
            ZDynamicObstacle* pObstacle = pNodeLink->m_This;
            if (!pObstacle || (iObstacleTypeMask & static_cast<int>(pObstacle->m_Type)) == 0)
            {
                continue;
            }

            const float fPos[2] = { fX, fZ };
            if (!pObstacle->IsInside(fPos))
            {
                continue;
            }

            // Direction from the obstacle centre to the pushed position.
            const float fCentreX = pObstacle->m_Location.m_vPos.x;
            const float fCentreZ = pObstacle->m_Location.m_vPos.z;
            float fDirX = fX - fCentreX;
            float fDirZ = fZ - fCentreZ;
            const float fDirLen = std::sqrt(fDirX * fDirX + fDirZ * fDirZ);
            if (fDirLen <= 0.0f)
            {
                fDirX = 1.0f;
                fDirZ = 0.0f;
            }
            else
            {
                fDirX /= fDirLen;
                fDirZ /= fDirLen;
            }

            // Cast from a point radius beyond the centre back towards the centre
            // and intersect with the hull to find the push-out distance.
            const float fStartX = fCentreX + fDirX * pObstacle->m_Radius;
            const float fStartZ = fCentreZ + fDirZ * pObstacle->m_Radius;
            const float fStart[2] = { fStartX, fStartZ };
            const float fCentre[2] = { fCentreX, fCentreZ };
            float fTangent[2][2] = {};
            int iRight = -1;
            int iLeft = -1;
            const float fHit = pObstacle->Intersect(fStart, fCentre, pObstacle->m_Radius, pObstacle->m_Radius,
                fTangent, iRight, iLeft);
            if (fHit > 0.0f && fHit < pObstacle->m_Radius)
            {
                const float fPush = fHit * 0.99f;
                fX = fStartX - fDirX * fPush;
                fZ = fStartZ - fDirZ * fPush;
            }
        }

        vPos.x = fX;
        vPos.z = fZ;
    }

    bool ZData::HasObstacles()
    {
        return m_ObstacleCount != 0;
    }

    bool ZData::FindPath(ZPathRequest* pPath)
    {
        // Finds a path from the request source to its destination, honouring the
        // stopping distance and keeping clear of the destination reservation when
        // other nodes are parked there (PC 004E0860).
        ZPath* pRequestPath = pPath->m_Path;

        FreePath(pRequestPath);
        pRequestPath->Clear();

        ZLocation destination;
        destination = *pPath->m_Dest;

        ZDataRef aTempBuffer[200];
        ZPath tempPath(aTempBuffer, 200);
        ZPathLink aPathLinks[200];
        int iPathLinkCount = 0;

        if (pPath->m_StoppingDistance != 0.0f)
        {
            if (!FindPath(
                    *pPath->m_Source, destination, tempPath, aPathLinks, iPathLinkCount,
                    pPath->m_GoOutside, pPath->m_ActorKeymask))
            {
                tempPath.m_pathIdx = nullptr;
                return false;
            }

            const float fCutDistance = (pPath->m_StoppingDistance <= 0.0f)
                ? tempPath.m_Cost + pPath->m_StoppingDistance
                : pPath->m_StoppingDistance;
            tempPath.CutAtDistance(fCutDistance);

            ZVector3 vPathEnd;
            tempPath.GetEndPosition(vPathEnd);
            MapLocation(vPathEnd, destination);
        }

        if (!pPath->m_IgnoreReservation && pPath->m_Reservation)
        {
            ZResult aNearbyNodes[20];
            const int iReservationType = pPath->m_Reservation->m_Type;
            const int iNearCount = FindNodes(destination, aNearbyNodes, 10, 70.0f, iReservationType);

            // Only relocate when the reservation is not the single occupant of the
            // destination area.
            if (iNearCount > 0 && (iNearCount != 1 || aNearbyNodes[0].pNode != pPath->m_Reservation))
            {
                const int iNodeCount = FindNodes(destination, aNearbyNodes, 20, 250.0f, iReservationType);

                bool bMoved = false;
                for (int iDirection = 0; iDirection < 6; ++iDirection)
                {
                    const float fAngle = static_cast<float>(iDirection) * 1.0471976f;
                    const float fCos = std::cos(fAngle);
                    const float fSin = std::sin(fAngle);

                    // End point of the probe ray cast; as in the original, the
                    // horizontal offset lands in the second component of the
                    // wall-cast endpoint vector.
                    ZVector3 vRayEnd;
                    vRayEnd.x = destination.m_vPos.x + fCos * 400.0f;
                    vRayEnd.y = destination.m_vPos.z + fSin * 400.0f;

                    ZVector3 vRayNormal;
                    ZVector3 vRayOut;
                    ZLocation wallEndLocation;
                    bool bHitWall = false;
                    const float fWallDistance = FindWallIntersection(
                        destination, vRayEnd, 400.0f, vRayNormal, vRayOut,
                        bHitWall, wallEndLocation, false, false);

                    for (int iStep = 1; iStep < 5; ++iStep)
                    {
                        const float fProbeDistance = static_cast<float>(iStep) * 50.0f;
                        if (bHitWall && fProbeDistance > fWallDistance)
                        {
                            break;
                        }

                        ZVector3 vProbe;
                        vProbe.x = destination.m_vPos.x + fCos * fProbeDistance;
                        vProbe.z = destination.m_vPos.z + fSin * fProbeDistance;

                        int iNode = 0;
                        for (; iNode < iNodeCount; ++iNode)
                        {
                            const ZMetaNode* pNode = aNearbyNodes[iNode].pNode;
                            if (pNode != pPath->m_Reservation
                                && Dist2DSq(pNode->m_Location.m_vPos.x, pNode->m_Location.m_vPos.z,
                                    vProbe.x, vProbe.z)
                                    < 4900.0f)
                            {
                                break;
                            }
                        }

                        if (iNode == iNodeCount)
                        {
                            vProbe.y = destination.m_vPos.y;
                            MapLocation(vProbe, destination);
                            bMoved = true;
                            break;
                        }
                    }

                    if (bMoved)
                    {
                        break;
                    }
                }

                if (!bMoved)
                {
                    pRequestPath->Clear();
                    tempPath.m_pathIdx = nullptr;
                    return false;
                }
            }
        }

        // The destination may have been shifted while looking for a free spot:
        // drop the previously computed path so it is rebuilt for the new location.
        if (tempPath.m_Size > 0)
        {
            ZVector3 vPathEnd;
            tempPath.GetEndPosition(vPathEnd);
            if (vPathEnd.x != destination.m_vPos.x
                || vPathEnd.y != destination.m_vPos.y
                || vPathEnd.z != destination.m_vPos.z)
            {
                tempPath.Clear();
            }
        }

        if (tempPath.m_Size <= 0)
        {
            if (!FindPath(
                    *pPath->m_Source, destination, tempPath, aPathLinks, iPathLinkCount,
                    pPath->m_GoOutside, pPath->m_ActorKeymask))
            {
                tempPath.m_pathIdx = nullptr;
                return false;
            }
        }

        if (AllocateBufferPath(tempPath, pRequestPath))
        {
            if (pPath->m_Reservation)
            {
                RemoveNode(pPath->m_Reservation);
                AddNode(pPath->m_Reservation, destination);
            }
        }

        tempPath.m_pathIdx = nullptr;
        return true;
    }

    void ZData::FreePath(ZPath* pPath)
    {
        // Returns a path buffer previously handed out by AllocateBufferPath to the
        // block allocator that owns it (PC 004DBA80). Buffers that belong to none
        // of the allocators (e.g. a caller-provided array) are left untouched.
        if (!pPath->m_pathIdx)
        {
            return;
        }

        ZBlockAlocator* pOwner = nullptr;
        for (int i = 0; i < 3; ++i)
        {
            if (m_Allocator[i].BelongsTo(pPath->m_pathIdx))
            {
                pOwner = &m_Allocator[i];
                break;
            }
        }
        if (!pOwner)
        {
            return;
        }

        pOwner->Free(pPath->m_pathIdx);
        pPath->m_pathIdx = nullptr;
        pPath->Clear();
    }

    bool ZData::AllocateBufferPath(const ZPath& sPath, ZPath* pResult)
    {
        // Allocates a path buffer from the first block allocator that fits the path
        // size, copies the source path into it and repoints the result's path
        // buffer (PC 004DA160 / PS2 0x1F78AC).
        const int iSize = sPath.m_Size;

        ZDataRef* pBuffer = nullptr;
        for (int i = 0; i < 3; ++i)
        {
            if (m_Allocator[i].m_BlockSize >= iSize)
            {
                pBuffer = m_Allocator[i].Alloc();
                if (pBuffer)
                {
                    break;
                }
            }
        }

        if (!pBuffer)
        {
            return false;
        }

        *pResult = sPath;
        std::memcpy(pBuffer, sPath.m_pathIdx, 12 * iSize);
        pResult->m_pathIdx = pBuffer;

        return true;

    }

    float ZData::FindPathLength(ZPathRequest* pRequest)
    {
        // Runs a temporary path request towards the same destination through a
        // throw-away path object, keeps only the total cost (PC 004DB9D0 /
        // PS2 0x1F6A08).
        ZPath tempPath{};
        ZPath* pSavedPath = pRequest->m_Path;
        pRequest->m_Path = &tempPath;
        FindPath(pRequest);
        pRequest->m_Path = pSavedPath;

        float fCost = 0.0f;
        if (tempPath.m_Size > 0)
        {
            fCost = tempPath.m_Cost;
        }

        if (tempPath.m_pathIdx)
        {
            FreePath(&tempPath);
        }

        return fCost;

    }

    bool ZData::PositionInside(const ZVector3& vPos)
    {
        // True when the point lies inside one of the navmesh graphs' components
        // (PC 004DB9B0).
        ZIndex iGraph = -1;
        ZIndex iComponent = -1;
        return GetGraphAndComponent(&vPos.x, iGraph, iComponent);
    }

    bool ZData::MapLocation(const ZVector3& vPos, ZLocation& kLocation)
    {
        // Finds the closest graph/component for the position and stores the
        // raw position together with the found graph/component (PC 004DF1E0).
        float vClosest[3];
        ZIndex iGraph = -1;
        ZIndex iComponent = -1;
        const bool bFound = GetClosestGraphAndComponent(&vPos.x, vClosest, iGraph, iComponent, false);
        kLocation.Set(vPos, iGraph, iComponent, bFound);
        return bFound;
    }

    bool ZData::MapValidLocation(const ZVector3& vPos, ZLocation& kLocation)
    {
        // Finds the closest graph/component forthe position without the fall-back
        // scan and stores the raw position together with the found graph/component
        // (PC 004DF170 / PS2 0x1FC0D8).
        float vClosest[3];
        ZIndex iGraph = -1;
        ZIndex iComponent = -1;

        if (!GetClosestGraphAndComponent(&vPos.x, vClosest, iGraph, iComponent, true))
        {

            return false;

        }


        kLocation.Set(vPos, iGraph, iComponent, true);
        return true;
    }

    float ZData::ComponentHeight(int iComponent, float x, float z)
    {
        // The public height query resolves the component and walks its height
        // tree (PC 004DA8C0 wrapper, PC 004D9900 worker).
        if (iComponent < 0 || iComponent >= m_iComponentCount || !m_pComponents)
        {
            return 0.0f;
        }

        return ComponentHeightAt(*this, m_pComponents[iComponent], x, z);
    }

    bool ZData::MapOntoComponent(const ZLocation& kSource, ZLocation& kTarget)
    {
        // Snaps the source position onto the walkable region of the component it
        // belongs to and adjusts the target height (PC 004DA8E0).
        if (!m_pComponents)
        {
            return false;
        }

        const bool bMapped = MapInside(kSource, kTarget, true);
        const int iComponent = kTarget.Component();
        if (iComponent >= 0 && iComponent < m_iComponentCount)
        {
            kTarget.m_vPos.y = ComponentHeightAt(*this, m_pComponents[iComponent], kSource.m_vPos.x, kSource.m_vPos.z);
        }

        return bMapped;
    }

    bool ZData::MapInside(const ZLocation& kSource, ZLocation& kTarget, bool /*bUseObstacle*/)
    {
        // Ensures a graph location's position lies inside its component polygon.
        // A position that is not inside is pulled onto the nearest wall edge and
        // the target height is adjusted to the component floor (PC 004DCF20).
        if (kSource.Graph() == -1)
        {
            return false;
        }

        ZComponent* pComponent = (m_pComponents && kSource.Component() >= 0 && kSource.Component() < m_iComponentCount)
            ? &m_pComponents[kSource.Component()]
            : nullptr;
        if (!pComponent)
        {
            ZASSERT(false);
            return false;
        }

        if (kSource.Inside() != 0)
        {
            // Already mapped: keep the source position untouched.
            kTarget = kSource;
            kTarget.m_Inside = 1;
            return true;
        }

        float fX = kSource.m_vPos.x;
        float fY = kSource.m_vPos.y;
        float fZ = kSource.m_vPos.z;

        const int iCorners = pComponent->m_Corners;
        const int iFirstCorner = pComponent->m_FirstCorner;
        bool bInsidePolygon = true;
        float fBestDist = 3.4028235e38f;
        float fBestX = fX;
        float fBestZ = fZ;

        if (iCorners > 0)
        {
            for (int i = 0; i < iCorners; ++i)
            {
                const int iNext = (i + 1 == iCorners) ? 0 : i + 1;
                const ZCorner& rCornerA = m_pCorners[iFirstCorner + i];
                const ZCorner& rCornerB = m_pCorners[iFirstCorner + iNext];
                const ZVertex& rVertexA = m_pVertices[rCornerA.m_iVertex];
                const ZVertex& rVertexB = m_pVertices[rCornerB.m_iVertex];

                const float fAx = rVertexA.m_kPos.x;
                const float fAz = rVertexA.m_kPos.y;
                const float fBx = rVertexB.m_kPos.x;
                const float fBz = rVertexB.m_kPos.y;

                const float fEx = fBx - fAx;
                const float fEz = fBz - fAz;
                const float fDx = fX - fAx;
                const float fDz = fZ - fAz;

                if (fDx * fEz - fDz * fEx < 0.0f)
                {
                    bInsidePolygon = false;
                }

                // Only solid (wall) corner edges are used to pull the point back
                // into the component; openings to other components are ignored.
                if (rCornerA.m_iComponent != -1)
                {
                    continue;
                }

                const float fDot = fDz * fEz + fDx * fEx;
                const float fLen2 = fEz * fEz + fEx * fEx;

                float fQx;
                float fQz;
                float fDist;
                if (fDot >= 0.0f)
                {
                    if (fDot <= fLen2)
                    {
                        const float fT = fDot / fLen2;
                        fQx = fAx + fEx * fT;
                        fQz = fAz + fEz * fT;
                        const float fHeight = ComponentHeightAt(*this, *pComponent, fX, fZ);
                        fDist = std::sqrt(
                            (fZ - fQz) * (fZ - fQz) + (fY - fHeight) * (fY - fHeight) + (fX - fQx) * (fX - fQx));
                    }
                    else
                    {
                        fQx = fBx;
                        fQz = fBz;
                        fDist = std::sqrt(
                            (fZ - fQz) * (fZ - fQz)
                            + (fY - rVertexB.m_fHeight) * (fY - rVertexB.m_fHeight)
                            + (fX - fQx) * (fX - fQx));
                    }
                }
                else
                {
                    fQx = fAx;
                    fQz = fAz;
                    fDist = std::sqrt(
                        (fZ - fQz) * (fZ - fQz)
                        + (fY - rVertexA.m_fHeight) * (fY - rVertexA.m_fHeight)
                        + (fX - fQx) * (fX - fQx));
                }

                if (fDist < fBestDist)
                {
                    fBestDist = fDist;
                    fBestX = fQx;
                    fBestZ = fQz;
                }
            }
        }

        if (!bInsidePolygon)
        {
            fX = fBestX;
            fZ = fBestZ;
        }

        kTarget = kSource;
        kTarget.m_vPos.x = fX;
        kTarget.m_vPos.z = fZ;
        kTarget.m_vPos.y = ComponentHeightAt(*this, *pComponent, fX, fZ);
        kTarget.m_Inside = 1;
        return true;
    }

    int ZData::FindComponents(const ZLocation& kLocation, int* pList, int iMax, float fMaxDistance)
    {
        // Flood fills adjacent components whose shared corner edges lie within
        // fMaxDistance of the query location (PC 004DCC20). The caller receives at
        // most iMax component ids.
        const int iStartComponent = kLocation.Component();
        if (iStartComponent == -1 || !m_pComponents || !m_pComponentVisited || iMax <= 0)
        {
            return 0;
        }

        std::memset(m_pComponentVisited, 0, 4u * m_iComponentVisitedSize);

        const float fRadius2 = fMaxDistance * fMaxDistance;
        int iCount = 0;

        int aStack[100];
        int iStackTop = 0;
        aStack[iStackTop++] = iStartComponent;

        while (iStackTop > 0 && iCount < iMax)
        {
            const int iComponent = aStack[--iStackTop];
            pList[iCount++] = iComponent;

            m_pComponentVisited[iComponent >> 5] |= 1u << (iComponent & 0x1F);

            const ZComponent& rComponent = m_pComponents[iComponent];
            const int iCorners = rComponent.m_Corners;
            const int iFirstCorner = rComponent.m_FirstCorner;
            for (int i = 0; i < iCorners; ++i)
            {
                const int iNext = (i + 1 == iCorners) ? 0 : i + 1;
                const ZCorner& rCornerA = m_pCorners[iFirstCorner + i];
                int iNeighbor = rCornerA.m_iComponent;
                if (iNeighbor == -1)
                {
                    continue;
                }
                if (iNeighbor < 0)
                {
                    iNeighbor = -2 - iNeighbor;
                }
                if (iNeighbor < 0 || (m_pComponentVisited[iNeighbor >> 5] & (1u << (iNeighbor & 0x1F))) != 0)
                {
                    continue;
                }

                const ZVertex& rVertexA = m_pVertices[rCornerA.m_iVertex];
                const ZVertex& rVertexB = m_pVertices[m_pCorners[iFirstCorner + iNext].m_iVertex];

                const float fAx = rVertexA.m_kPos.x;
                const float fAz = rVertexA.m_kPos.y;
                const float fBx = rVertexB.m_kPos.x;
                const float fBz = rVertexB.m_kPos.y;
                const float fPx = kLocation.m_vPos.x;
                const float fPz = kLocation.m_vPos.z;

                const float fDax = fAx - fPx;
                const float fDaz = fAz - fPz;
                const float fDbx = fBx - fPx;
                const float fDbz = fBz - fPz;
                if (fDax * fDax + fDaz * fDaz <= fRadius2 || fDbx * fDbx + fDbz * fDbz <= fRadius2)
                {
                    if (iStackTop < 100)
                    {
                        aStack[iStackTop++] = iNeighbor;
                    }
                    continue;
                }

                const float fEx = fBx - fAx;
                const float fEz = fBz - fAz;
                const float fLen2 = fEx * fEx + fEz * fEz;
                if (fLen2 <= 0.0f)
                {
                    continue;
                }

                const float fT = ((fPx - fAx) * fEx + (fPz - fAz) * fEz) / fLen2;
                if (fT <= 0.0f || fT >= 1.0f)
                {
                    continue;
                }

                const float fQx = fAx + fEx * fT;
                const float fQz = fAz + fEz * fT;
                const float fDqx = fQx - fPx;
                const float fDqz = fQz - fPz;
                if (fDqx * fDqx + fDqz * fDqz <= fRadius2 && iStackTop < 100)
                {
                    aStack[iStackTop++] = iNeighbor;
                }
            }
        }

        return iCount;
    }

    int ZData::CloseExit(float* pvPos)
    {
        // Closes exit links whose endpoint node is within 250 units of pvPos (PC 004DBB20).
        int iClosed = 0;
        for (int i = 0; i < m_iLinkCount; ++i)
        {
            ZLink& link = m_pLinks[i];
            if (link.m_Type != 1)
            {
                continue;
            }

            const ZNode& node = LinkToNode(*this, link);
            const float fDx = node.m_kPos.x - pvPos[0];
            const float fDy = node.m_fHeight - pvPos[1];
            const float fDz = node.m_kPos.y - pvPos[2];
            if (fDx * fDx + fDy * fDy + fDz * fDz < 62500.0f)
            {
                link.m_iKeyMask = 0;
                ++iClosed;
            }
        }

        return iClosed;
    }

    int ZData::CloseLinks(const ZVector3& vMin, const ZVector3& vMax, uint32_t lKeyMask)
    {
        // Clears the given key mask on all links ending inside the region (PC 004E0390).
        float fBox[6];
        BoxReset(fBox);
        const float aMin[3] = { vMin.x, vMin.y, vMin.z };
        const float aMax[3] = { vMax.x, vMax.y, vMax.z };
        BoxAddPoint(fBox, aMin);
        BoxAddPoint(fBox, aMax);

        int iChanged = 0;
        for (int i = 0; i < m_iLinkCount; ++i)
        {
            const ZNode& node = LinkToNode(*this, m_pLinks[i]);
            if (node.m_kPos.x < fBox[0] || node.m_kPos.x > fBox[3]
                || node.m_fHeight < fBox[1] || node.m_fHeight > fBox[4]
                || node.m_kPos.y < fBox[2] || node.m_kPos.y > fBox[5])
            {
                continue;
            }

            m_pLinks[i].m_iKeyMask = static_cast<uint16_t>(m_pLinks[i].m_iKeyMask & ~lKeyMask);
            ++iChanged;
        }

        return iChanged;
    }

    int ZData::OpenLinks(const ZVector3& vMin, const ZVector3& vMax, uint32_t lKeyMask)
    {
        // Sets the given key mask on all links ending inside the region (PC 004E04C0).
        float fBox[6];
        BoxReset(fBox);
        const float aMin[3] = { vMin.x, vMin.y, vMin.z };
        const float aMax[3] = { vMax.x, vMax.y, vMax.z };
        BoxAddPoint(fBox, aMin);
        BoxAddPoint(fBox, aMax);

        int iChanged = 0;
        for (int i = 0; i < m_iLinkCount; ++i)
        {
            const ZNode& node = LinkToNode(*this, m_pLinks[i]);
            if (node.m_kPos.x < fBox[0] || node.m_kPos.x > fBox[3]
                || node.m_fHeight < fBox[1] || node.m_fHeight > fBox[4]
                || node.m_kPos.y < fBox[2] || node.m_kPos.y > fBox[5])
            {
                continue;
            }

            m_pLinks[i].m_iKeyMask = static_cast<uint16_t>(m_pLinks[i].m_iKeyMask | lKeyMask);
            ++iChanged;
        }

        return iChanged;
    }

    int ZData::GraphCount()
    {
        return m_iGraphCount;
    }

void ZData::FindCornersInGraph(int iGraph, int& iMaxCorners, float* pvOut)
    {
        // Walks the graph's vertex ring and reports every sharp gate corner inside
        // a 60-unit radius. Each reported corner is three world-space triples:
        // the corner position and the two gate direction vectors along the flanking
        // edges (PC 004DE0F0 / PS2 0x1F5644).
        if (iGraph == -1)
        {
            iMaxCorners = 0;
            return;
        }

        const int iVertexCount = m_pGraphs[iGraph].m_iVertices - m_pGraphs[iGraph].m_iExits;
        if (iVertexCount <= 0 || !m_pVertices)
        {
            iMaxCorners = 0;
            return;
        }

        const ZVertex* pRing = m_pVertices + m_pGraphs[iGraph].m_iFirstVertex;

        auto FnVertex = [](const ZVertex& v, float* pOut)
        {
            pOut[0] = v.m_kPos.x;
            pOut[1] = v.m_fHeight;
            pOut[2] = v.m_kPos.y;
        };

        float a3[3];
        float a2a[3];
        FnVertex(pRing[iVertexCount - 1], a3);
        FnVertex(pRing[0], a2a);

        float pResult[3];
        vsub(pResult, a2a, a3);
        pResult[1] = 0.0f;
        float fPrevLen = vnorm(pResult);

        int iCornerCount = 0;
        for (int i = 0; i < iVertexCount; ++i)
        {
            if (iCornerCount >= iMaxCorners)
            {
                break;
            }

            const ZVertex& rNext = pRing[(i + 1) % iVertexCount];
            float v38[3];
            FnVertex(rNext, v38);

            float vector[3];
            vsub(vector, v38, a2a);
            vector[1] = 0.0f;
            const float fNextLen = vnorm(vector);

            if (fPrevLen > 60.0f || fNextLen > 60.0f)
            {
                const float fDot = vdot(vector, pResult);

                if (fDot < 0.15f && fDot > -0.15f)
                {
                    float vec2[3];
                    vcross(vec2, pResult, vector);
                    vsetlen(vec2, 100.0f);

                    if (vec2[1] < 0.0f)
                    {
                        float v40[3];
                        float v41[3];

                        vcross(v40, pResult, vec2);
                        vnorm(v40);
                        vcross(v41, vector, vec2);
                        vnorm(v41);

                        pvOut[0] = a2a[0];
                        pvOut[1] = a2a[1];
                        pvOut[2] = a2a[2];
                        pvOut[3] = v40[0];
                        pvOut[4] = v40[1];
                        pvOut[5] = v40[2];
                        pvOut[6] = v41[0];
                        pvOut[7] = v41[1];
                        pvOut[8] = v41[2];
                        pvOut += 9;
                        ++iCornerCount;
                    }
                }
            }

            std::memcpy(a3, a2a, sizeof(a3));
            std::memcpy(a2a, v38, sizeof(a2a));
            std::memcpy(pResult, vector, sizeof(pResult));
            fPrevLen = fNextLen;
        }

        iMaxCorners = iCornerCount;
    }

    float ZData::FindWallIntersection(
        const ZLocation&,
        const ZVector3&,
        float,
        const ZVector3&,
        ZVector3&,
        bool&,
        ZLocation&,
        bool,
        bool)
    {
        // TODO: Finish me (PC 004DFAA0)
        return 0.0f;
    }

    int ZData::FindWalls(const ZLocation&, float, float*)
    {
        // TODO: Finish me (PC 004DDB30)
        return 0;
    }

    int ZData::FindObstacles(const ZLocation& kLocation, ZDynamicObstacle** pvFoundObstacles, int iMax)
    {
        // Returns up to iMax dynamic obstacles registered in the location's
        // component that contain the location point (PC 004DB930).
        const int iComponent = kLocation.Component();
        if (iComponent == -1 || iMax <= 0)
        {
            return 0;
        }

        auto** pHeads = reinterpret_cast<ZDynamicObstacle_Link**>(m_pDynamicObstacles);
        if (!pHeads)
        {
            return 0;
        }

        int iCount = 0;
        const float fPos[2] = { kLocation.m_vPos.x, kLocation.m_vPos.z };
        for (ZDynamicObstacle_Link* pNodeLink = pHeads[iComponent]; pNodeLink && iCount < iMax;
             pNodeLink = pNodeLink->m_Next)
        {
            ZDynamicObstacle* pObstacle = pNodeLink->m_This;
            if (pObstacle && pObstacle->IsInside(fPos))
            {
                pvFoundObstacles[iCount++] = pObstacle;
            }
        }

        return iCount;
    }

    void ZData::RemapDoorRefs(ZREF* pRefs, uint32_t lRefsNr)
    {
        // Replaces each link's 1-based door controller id with its new value (PC 004DA480).
        for (int i = 0; i < m_iLinkCount; ++i)
        {
            ZLink& link = m_pLinks[i];
            if (link.m_rDoorController < lRefsNr)
            {
                link.m_rDoorController = pRefs[link.m_rDoorController - 1];
            }
        }
    }

    bool ZData::FindPath(
        const ZLocation& kFrom, const ZLocation& kTo, ZPath& rPath, ZPathLink* pPathLink,
        int& iPathLinkCount, bool bGoOutside, unsigned int lActorKeymask)
    {
        // Builds the raw way-point list for a move between two graph locations.
        // The caller receives the number of recorded ZPathLink entries through
        // iPathLinkCount (PC 004E05E0).
        iPathLinkCount = 0;

        if (kFrom.Graph() == -1 || kTo.Graph() == -1)
        {
            return false;
        }

        rPath.Clear();
        rPath.m_PathFinder = this;

        const bool bFromInside = kFrom.Inside() != 0;
        const bool bToInside = kTo.Inside() != 0;

        ZLocation fromLocation;
        ZLocation toLocation;
        MapInside(kFrom, fromLocation, false);
        MapInside(kTo, toLocation, true);

        if (fromLocation.Component() == -1 || toLocation.Component() == -1)
        {
            return false;
        }

        AssignGraph(fromLocation);
        AssignGraph(toLocation);

        if (fromLocation.Graph() >= m_iGraphCount)
        {
            AssignGraph(fromLocation);
        }
        if (fromLocation.Graph() == -1 || toLocation.Graph() == -1)
        {
            return false;
        }

        if (!bFromInside)
        {
            // The source lies off the navmesh: append it as a plain approach point
            // that precedes the first navigated way point.
            const ZDataRef sourceRef = rPath.AddVertex(&kFrom.m_vPos.x);
            ZASSERT(sourceRef.m_Type == 3 && sourceRef.m_Id < 4);

            ZPathLink& rLink = pPathLink[iPathLinkCount];
            rLink.m_Action = PWA_DEFAULT;
            rLink.m_Pos = kFrom.m_vPos;
            ++iPathLinkCount;
        }

        const int iAstarLinks = FindComponentPathAStar(
            fromLocation, toLocation, rPath, &pPathLink[iPathLinkCount], lActorKeymask);
        iPathLinkCount += iAstarLinks;
        if (iAstarLinks < 2)
        {
            iPathLinkCount = 0;
            return false;
        }

        if (bGoOutside && !bToInside)
        {
            // The destination lies off the navmesh: append it as a trailing point
            // after the last navigated way point.
            const ZDataRef destRef = rPath.AddVertex(&kTo.m_vPos.x);
            ZASSERT(destRef.m_Type == 3 && destRef.m_Id < 4);

            ZPathLink& rLink = pPathLink[iPathLinkCount];
            rLink.m_Action = PWA_DEFAULT;
            rLink.m_Pos = kTo.m_vPos;
            ++iPathLinkCount;
        }

        // PC 004E0810: g_lDisplayPathLinesCommand toggles the debug visualisation
        // of the path lines; the original additionally reset a throw-away identity
        // matrix inside the enabled branch.
        static bool s_bShowPaths = false;
        if (g_lDisplayPathLinesCommand)
        {
            s_bShowPaths = true;
        }
        else if (s_bShowPaths)
        {
            s_bShowPaths = false;
        }

        return true;
    }

    void ZData::AssignGraph(ZLocation& kLocation)
    {
        // The location must already have a valid component; here we find the graph
        // this component belongs to and store it back into the location (PC 004DA930).
        const int iComponent = kLocation.Component();
        const ZIndex iLastGraph = m_iGraphCount - 1;
        if (iComponent < 0 || iComponent >= m_iComponentCount)
        {
            ZASSERT(false);
        }

        if (m_iGraphCount <= 1)
        {
            kLocation.m_Graph = iLastGraph;
            return;
        }

        int iGraph = 1;
        while (iComponent >= m_pGraphs[iGraph].m_iFirstComponent)
        {
            ++iGraph;
            if (iGraph >= m_iGraphCount)
            {
                kLocation.m_Graph = iLastGraph;
                return;
            }
        }

        kLocation.m_Graph = iGraph - 1;
    }

    bool ZData::GetGraphAndComponent(const float* pvPosition, ZIndex& rGraph, ZIndex& rComponent)
    {
        rGraph = -1;
        rComponent = -1;
        if (m_iGraphCount <= 0)
        {
            return false;
        }

        for (int iGraph = 0; iGraph < m_iGraphCount; ++iGraph)
        {
            const ZGraph& graph = m_pGraphs[iGraph];
            if (pvPosition[0] < graph.m_Min.x || pvPosition[0] > graph.m_Max.x
                || pvPosition[1] < graph.m_Min.y || pvPosition[1] > graph.m_Max.y
                || pvPosition[2] < graph.m_Min.z || pvPosition[2] > graph.m_Max.z)
            {
                continue;
            }

            const int iComponent = GraphGetComponent(*this, graph, pvPosition[0], pvPosition[2]);
            if (iComponent != -1)
            {
                rComponent = static_cast<ZIndex>(iComponent);
                rGraph = static_cast<ZIndex>(iGraph);
                return rGraph != -1;
            }
        }

        return false;
    }

    bool ZData::GetClosestGraphAndComponent(
        const float* pvPosition, float* pvClosest, ZIndex& rGraph, ZIndex& rComponent, bool bSkipFallback)
    {
        const float fX = pvPosition[0];
        const float fY = pvPosition[1];
        const float fZ = pvPosition[2];
        rGraph = -1;
        rComponent = -1;
        pvClosest[0] = fX;
        pvClosest[1] = fY;
        pvClosest[2] = fZ;

        int iBestGraph = -1;
        int iBestComponent = -1;
        float fBestX = fX;
        float fBestY = fY;
        float fBestZ = fZ;
        float fBestMetric = 100000.0f;

        // Quick pass: point strictly inside a graph's x/z bounds; compare the height
        // difference to the component's floor height.
        for (int iGraph = 0; iGraph < m_iGraphCount; ++iGraph)
        {
            const ZGraph& graph = m_pGraphs[iGraph];
            if (fX <= graph.m_Min.x || fX >= graph.m_Max.x || fZ <= graph.m_Min.z || fZ >= graph.m_Max.z)
            {
                continue;
            }

            const int iComponent = GraphGetComponent(*this, graph, fX, fZ);
            if (iComponent == -1)
            {
                continue;
            }

            const float fHeight = ComponentHeightAt(*this, m_pComponents[iComponent], fX, fZ);
            const float fHeightDiff = std::fabs(fY - fHeight);
            if (fHeightDiff < fBestMetric)
            {
                fBestMetric = fHeightDiff;
                iBestGraph = iGraph;
                iBestComponent = iComponent;
                fBestX = fX;
                fBestY = fHeight;
                fBestZ = fZ;
            }
        }

        if (fBestMetric < 200.0f)
        {
            rGraph = static_cast<ZIndex>(iBestGraph);
            rComponent = static_cast<ZIndex>(iBestComponent);
            return true;
        }

        if (!bSkipFallback)
        {
            for (int iGraph = 0; iGraph < m_iGraphCount; ++iGraph)
            {
                const ZGraph& graph = m_pGraphs[iGraph];
                if (!BBoxContainsPoint(graph.m_Min, graph.m_Max, pvPosition, 400.0f))
                {
                    continue;
                }

                float fDist;
                float vClosest[3];
                const int iComponent = GraphGetClosestComponent(*this, graph, pvPosition, fDist, vClosest);
                if (iComponent != -1 && (iBestGraph == -1 || fDist < fBestMetric))
                {
                    iBestGraph = iGraph;
                    iBestComponent = iComponent;
                    fBestMetric = fDist;
                    fBestX = vClosest[0];
                    fBestY = vClosest[1];
                    fBestZ = vClosest[2];
                }
            }

            if (iBestGraph != -1 && fBestMetric < 500.0f)
            {
                pvClosest[0] = fBestX;
                pvClosest[1] = fBestY;
                pvClosest[2] = fBestZ;
                rGraph = static_cast<ZIndex>(iBestGraph);
                rComponent = static_cast<ZIndex>(iBestComponent);
            }
        }

        return false;
    }

    int ZData::FindComponentPathAStar(const ZLocation&, const ZLocation&, ZPath&, ZPathLink*, unsigned int)
    {
        // TODO: Finish me (PC 004DE3B0)
        return 0;
    }

    int ZData::TraceExit(int iGraph, int iStartNode, int iEndNode, int* pOut, int* pOutEnd)
    {
        // Walks exit distances from iEndNode back to iStartNode, emitting 3-int
        // records into pOut (PC 004DA2A0).
        const ZGraph& graph = m_pGraphs[iGraph];
        const int iTargetNode = iStartNode + graph.m_iFirstNode;
        int iCurrentNode = iEndNode;
        int iCount = 0;
        if (iEndNode == iTargetNode)
        {
            return 0;
        }

        while (pOut < pOutEnd)
        {
            const int iExitDist = m_pExitDists[
                2 * iStartNode
                + 2 * graph.m_iExits * (iCurrentNode - graph.m_iFirstNode)
                + graph.m_iFirstExitDist
                + 1];

            if (iExitDist > -2)
            {
                if (iExitDist < 0)
                {
                    return iCount;
                }

                const ZSubNode& subNode = m_pSubNodes[iExitDist];
                iCurrentNode = subNode.m_Node;
                if (iCurrentNode == iTargetNode)
                {
                    return iCount;
                }

                if (subNode.m_Component == -1)
                {
                    pOut[1] = -4;
                    pOut[2] = subNode.m_Node;
                }
                else
                {
                    const ZComponent& component = m_pComponents[subNode.m_SourceComponent];
                    const int iDivider = subNode.m_Divider;
                    const int iNextDivider = (iDivider + 1 >= component.m_Corners) ? 0 : iDivider + 1;
                    pOut[1] = m_pCorners[component.m_FirstCorner + iDivider].m_iVertex;
                    pOut[2] = m_pCorners[component.m_FirstCorner + iNextDivider].m_iVertex;
                }
            }
            else
            {
                const int iExit = -2 - iExitDist;
                iCurrentNode = graph.m_iFirstNode + m_pLinks[iExit].m_iNode;
                pOut[1] = -3;
                pOut[2] = iExit;
            }

            pOut += 3;
            ++iCount;
            if (iCurrentNode == iTargetNode)
            {
                return iCount;
            }
        }

        return -1;
    }

    int ZData::StraightenGates(const float* pvStart, const float* pvEnd, int* pGates, int iGateCount, ZPathLink* pLinksOut, ZPath& rPath)
    {
        // Smooths a gate corridor into a straight path (PC 004DBD40).
        // Gate records are produced by TraceExit: [-3, exit, ...] / [-4, subnode, ...] /
        // otherwise [vertexA, vertexB]. Each 3-int gate is 12 bytes.
        //
        // NOTE: DRAFT translation - the gate record semantics and the ZDataRef
        // encoding used here must be validated against TraceExit and its only caller
        // FindComponentPathAStar once those are fully reversed.

        ZVector3 vStart(pvStart[0], pvStart[1], pvStart[2]);
        ZVector3 vEnd(pvEnd[0], pvEnd[1], pvEnd[2]);

        int iOutCount = 0;
        if (iGateCount <= 0)
        {
            pLinksOut[0].m_Pos = vEnd;
            pLinksOut[0].m_Action = 0;
            iOutCount = 1;
        }

        // "prev" end point/candidate references (mirrors v76/v78 and their refs).
        // We track a candidate as: handle (hA/hB), point + its own pos copy.
        ZDataRef refStart = rPath.AddVertex(pvStart);
        const uint32_t hStart = HandleOf(refStart);

        // Current best "start" reference of the segment being extended.
        uint32_t hSeg = hStart;         // v82/v87 (== combined)
        float fSegX = refStart.m_Pos.x; // v83/v88 (x)
        float fSegZ = refStart.m_Pos.y; // v84/X    (z)
        ZVector3 vSeg = vStart;         // v76

        // Alternate/right candidate reference.
        uint32_t hAlt = hStart;         // v87
        float fAltX = fSegX;            // v88
        float fAltZ = fSegZ;            // X
        ZVector3 vAlt = vStart;         // v78

        // reference to the currently processed gate start point
        float fCurX = vStart.x; // v79
        float fCurZ = vStart.z; // Z

        // gate cursor in bytes
        int iGateByte = 0;

        int iGateIndex = -1; // v81 (loop counter = processed gate index)

        const int iLoopEnd = iGateCount + 1;
        while (++iGateIndex < iLoopEnd)
        {
            ZPathLink* pOut = &pLinksOut[iOutCount];
            ZVector3 vA = vEnd;  // v94 (start of the segment for this gate)
            ZVector3 vB = vEnd;  // v93 (end of the segment for this gate)
            uint32_t hA = 0x7FFE; // v96
            uint32_t hB = 0x7FFE; // v99
            float fAx = vEnd.x;
            float fAz = vEnd.z;
            float fBx = vEnd.x;
            float fBz = vEnd.z;

            bool bSkip = false;
            bool bTouching = false;

            if (iGateIndex != iGateCount)
            {
                const int iType = pGates[iGateByte / 4 + 1]; // gate[1]
                const int iArg = pGates[iGateByte / 4 + 2];  // gate[2]

                if (iType == -2 || iType == -3)
                {
                    // Exit link: the "gate" is a doorway the segment must pass through.
                    const ZLink& link = m_pLinks[iArg];
                    const ZNode& node = LinkToNode(*this, link);
                    vA = ZVector3(node.m_kPos.x, node.m_fHeight, node.m_kPos.y);
                    vB = vA;

                    // Output current segment point and pass through the door.
                    pOut->m_Pos = vSeg;
                    pOut->m_Action = 0;
                    rPath.AddRef(MakeRef(hSeg, fSegX, fSegZ));
                    ++iOutCount;

                    pOut = &pLinksOut[iOutCount];
                    pOut->m_Pos = vA;
                    pOut->m_Action = link.m_iAction;
                    rPath.AddRef(MakeRef(MakeHandle(iArg, 2u), vA.x, vA.z));
                    ++iOutCount;

                    // The segment continues from the door point.
                    hSeg = MakeHandle(static_cast<uint32_t>(iArg), 2u);
                    fSegX = vA.x;
                    fSegZ = vA.z;
                    vSeg = vA;
                    vAlt = vA;
                    fCurX = vA.x;
                    fCurZ = vA.z;
                    bSkip = true;
                }
                else if (iType == -4)
                {
                    // Sub-node crossing: only worth resolving when the next gate is an exit.
                    if (iGateIndex + 1 >= iGateCount || pGates[iGateByte / 4 + 4] >= 0)
                    {
                        bSkip = true;
                    }
                    else
                    {
                        // Sub-node point (uses this gate's vertex B).
                        const ZNode& nodeA = m_pNodes[iArg];
                        vB = ZVector3(nodeA.m_kPos.x, nodeA.m_fHeight, nodeA.m_kPos.y);
                        vA = vB;
                        const uint32_t hTmp = iArg & 0x3FFF;
                        hB = hTmp;
                        hA = hTmp;
                        fBx = vB.x;
                        fBz = vB.z;
                        fAx = vA.x;
                        fAz = vA.z;
                    }
                }
                else
                {
                    // Vertex gate (normal case, ids >= 0): a passage between two corner
                    // vertices; shrink it by a small safety margin.
                    const ZVertex& vertFrom = m_pVertices[iType];
                    const ZVertex& vertTo = m_pVertices[iArg];
                    const float fFromH = vertFrom.m_fHeight;
                    const float fToH = vertTo.m_fHeight;
                    float fFromX = vertFrom.m_kPos.x;
                    float fFromZ = vertFrom.m_kPos.y;
                    float fToX = vertTo.m_kPos.x;
                    float fToZ = vertTo.m_kPos.y;

                    float fDX = fToX - fFromX;
                    float fDZ = fToZ - fFromZ;
                    float fLen = std::sqrt(fDX * fDX + fDZ * fDZ);
                    float fInvLen = 1.0f / fLen;
                    float fEdge = fLen - 5.0f;
                    if (fEdge > 0.0f)
                    {
                        if (fEdge > 100.0f)
                        {
                            fEdge = 100.0f;
                        }
                        fDZ *= fInvLen;
                        const float fStep = fDX * fInvLen * fEdge;
                        fDX = fStep;
                        const float fStepZ = fEdge * fDZ;
                        fDZ = fStepZ;
                        fFromX += 0.40000001f * fDX;
                        fFromZ += 0.40000001f * fDZ;
                        fToX -= 0.89999998f * fDX;
                        fToZ -= 0.89999998f * fDZ;
                    }

                    // vA = from point, vB = to point
                    vA = ZVector3(fFromX, fFromH, fFromZ);
                    vB = ZVector3(fToX, fToH, fToZ);
                    fAx = vA.x;
                    fAz = vA.z;
                    fBx = vB.x;
                    fBz = vB.z;
                    hA = MakeHandle(static_cast<uint32_t>(iType), 1u);
                    hB = MakeHandle(static_cast<uint32_t>(iArg), 1u);
                }
            }
            else
            {
                // Last gate: close at the end point.
                vA = vEnd;
                vB = vEnd;
                hA = 0x7FFE;
                hB = 0x7FFE;
                fAx = fBx = vEnd.x;
                fAz = fBz = vEnd.z;
            }

            if (!bSkip)
            {
                const float fEps = 0.0000099999997f;
                const float f2DTol = 0.0099999998f;
                const bool bAtSeg = (vSeg.x == fCurX && vSeg.z == fCurZ);
                const bool bAtAlt = (vAlt.x == fCurX && vAlt.z == fCurZ);

                const float fCrossA = (vB.x - fCurX) * (vSeg.z - fCurZ) - (vB.z - fCurZ) * (vSeg.x - fCurX);
                const float fCrossB = (vA.x - fCurX) * (vAlt.z - fCurZ) - (vA.z - fCurZ) * (vAlt.x - fCurX);

                if (bAtSeg || fCrossA >= -fEps)
                {
                    // Segment not forced to turn yet: possibly advance alt/seg.
                    const float fCrossAB = (vA.x - fCurX) * (vSeg.z - fCurZ) - (vA.z - fCurZ) * (vSeg.x - fCurX);
                    if (bAtAlt || (fCrossB < -fEps) || fCrossB <= fEps)
                    {
                        const bool bAltAhead = bAtSeg
                            || (((vA.x - fCurX) * (vAlt.z - fCurZ) - (vA.z - fCurZ) * (vAlt.x - fCurX)) >= -fEps
                                && ((vA.x - fCurX) * (vAlt.z - fCurZ) - (vA.z - fCurZ) * (vAlt.x - fCurX)) > fEps);
                        if (bAltAhead)
                        {
                            vAlt = vA;
                            hAlt = hA;
                            fAltX = fAx;
                            fAltZ = fAz;
                        }

                        const bool bSegAhead = bAtAlt
                            || ((vB.x - fCurX) * (vAlt.z - fCurZ) - (vB.z - fCurZ) * (vAlt.x - fCurX)) < -fEps;
                        if (bSegAhead)
                        {
                            vSeg = vB;
                            hSeg = hB;
                            fSegX = fBx;
                            fSegZ = fBz;
                        }

                        const bool bSegTouches = Dist2DSq(vSeg.x, vSeg.z, vA.x, vA.z) < f2DTol;
                        if (bSegTouches)
                        {
                            bTouching = true;
                        }
                        if (Dist2DSq(vAlt.x, vAlt.z, vB.x, vB.z) < f2DTol)
                        {
                            // collapse the gate into a single output point
                            pOut = &pLinksOut[iOutCount];
                            pOut->m_Pos = vSeg;
                            pOut->m_Action = 0;
                            rPath.AddRef(MakeRef(hSeg, fSegX, fSegZ));
                            ++iOutCount;
                            fCurX = vSeg.x;
                            fCurZ = vSeg.z;
                            iGateByte += 12;
                            continue;
                        }
                        bSkip = true;
                    }
                }

                if (!bSkip && !bTouching)
                {
                    // The segment is no longer free: commit the "start" candidate.
                    if (bAtSeg || fCrossA < -fEps)
                    {
                        // add alt
                        pOut = &pLinksOut[iOutCount];
                        pOut->m_Pos = vAlt;
                        pOut->m_Action = 0;
                        rPath.AddRef(MakeRef(hAlt, fAltX, fAltZ));
                        ++iOutCount;

                        vSeg = vAlt;
                        hSeg = hAlt;
                        fSegX = fAltX;
                        fSegZ = fAltZ;
                        fCurX = vAlt.x;
                        fCurZ = vAlt.z;
                    }
                    else
                    {
                        // add seg
                        pOut = &pLinksOut[iOutCount];
                        pOut->m_Pos = vSeg;
                        pOut->m_Action = 0;
                        rPath.AddRef(MakeRef(hSeg, fSegX, fSegZ));
                        ++iOutCount;

                        fCurX = vSeg.x;
                        fCurZ = vSeg.z;
                        vAlt = vSeg;
                        hAlt = hSeg;
                        fAltX = fSegX;
                        fAltZ = fSegZ;
                    }
                }
            }

            iGateByte += 12;
        }

        // Append the destination as a custom vertex.
        const ZDataRef refEnd = rPath.AddVertex(pvEnd);
        ZPathLink* pFinal = &pLinksOut[iOutCount];
        pFinal->m_Pos = vEnd;
        pFinal->m_Action = 0;

        return iOutCount + 1;
    }

    ZInterface* CreatePathFinder(void* data)
    {
        auto* pData = znew_placement(static_cast<ZData*>(ZUniMemory::Allocate(sizeof(ZData))));
        if (pData)
        {
            pData->LoadDataBlock(data);
        }

        return pData;
    }
}
