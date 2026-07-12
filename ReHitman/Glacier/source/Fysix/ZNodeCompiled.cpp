#include <Glacier/Fysix/ZCommonAlgorithms.h>
#include <Glacier/Fysix/ZOctreeCompiled.h>
#include <Glacier/ZSTL/CObjectInfo.h>
#include <Glacier/ZSTL/CQuadtree.h> // SRecurseInfoCompiled
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    struct SlocSubNode
    {
        float fT;
        void* pNode;
        unsigned int uBits;
    };

    namespace
    {
        void AddToHeap(SlocSubNode* pQueue, int iQueueLength, const SlocSubNode& subNode)
        {
            int i = iQueueLength;
            while (i > 0)
            {
                int iParent = (i - 1) >> 1;
                if (pQueue[iParent].fT <= subNode.fT)
                    break;

                pQueue[i] = pQueue[iParent];
                i = iParent;
            }
            pQueue[i] = subNode;
        }

        void Heapify(SlocSubNode* pQueue, int iQueueLength, int iNode)
        {
            while (true)
            {
                int iLeft = 2 * iNode + 1;
                int iRight = 2 * iNode + 2;
                int iSmallest = iNode;

                if (iLeft < iQueueLength && pQueue[iLeft].fT < pQueue[iSmallest].fT)
                    iSmallest = iLeft;
                if (iRight < iQueueLength && pQueue[iRight].fT < pQueue[iSmallest].fT)
                    iSmallest = iRight;

                if (iSmallest == iNode)
                    break;

                std::swap(pQueue[iNode], pQueue[iSmallest]);
                iNode = iSmallest;
            }
        }

        SlocSubNode* ExtractMin(SlocSubNode* pOutResult, SlocSubNode* pQueue, int iQueueLength)
        {
            pOutResult->fT = pQueue[0].fT;
            pOutResult->pNode = pQueue[0].pNode;
            pOutResult->uBits = pQueue[0].uBits;

            const SlocSubNode& lastNode = pQueue[iQueueLength - 1];

            pQueue[0].fT = lastNode.fT;
            pQueue[0].pNode = lastNode.pNode;
            pQueue[0].uBits = lastNode.uBits;

            Heapify(pQueue, iQueueLength - 1, 0);
            return pOutResult;
        }
    }

    void ZNodeCompiled::GetObjects(SRecurseInfoCompiled* pInfo)
    {
        uint32_t numObjects = GetNumObjects();
        if (numObjects > 0)
        {
            CObjectInfo* pObj = GetFirstObjectPtr(pInfo->pxBasePtrObjects);
            
            for (uint32_t i = 0; i < numObjects; ++i)
            {
                pInfo->pChkFunc(pObj->iID, pInfo);
                pObj++;
            }
        }

        ZNodeCompiled* pChild = GetFirstChildPtr(pInfo->pxBasePtrNodes);
        if (pChild)
        {
            bool isLast = false;
            do
            {
                pChild->GetObjects(pInfo);
                isLast = pChild->IsLast();
                pChild++;
            } 
            while (!isLast);
        }
    }

    void ZNodeCompiled::RemapObjects(SRecurseInfoCompiled* pInfo)
    {
        uint32_t numObjects = GetNumObjects();
        if (numObjects > 0)
        {
            CObjectInfo* pObj = GetFirstObjectPtr(pInfo->pxBasePtrObjects);

            // rly? - I guess so. In MiniNinjas we have a direct call into pUserData
            auto pFnRemap = reinterpret_cast<ZOctree::RemapObjectIdFn>(pInfo->pUserData);
            
            for (uint32_t i = 0; i < numObjects; ++i)
            {
                pObj->iID = pFnRemap(pObj->iID);
                pObj++;
            }
        }

        ZNodeCompiled* pChild = GetFirstChildPtr(pInfo->pxBasePtrNodes);
        if (pChild)
        {
            bool isLast = false;
            do
            {
                pChild->RemapObjects(pInfo);
                isLast = pChild->IsLast();
                pChild++;
            } 
            while (!isLast);
        }
    }

    void ZNodeCompiled::CheckPoint(SRecurseInfoCompiled* pInfo, int iNodeCenterX, int iNodeCenterY, int iNodeCenterZ)
    {
        ZASSERT(pInfo->iDepth >= 0 && pInfo->iDepth < 16);

        pInfo->iDepth++;

        uint32_t numObjects = GetNumObjects();
        if (numObjects > 0)
        {
            CObjectInfo* pObj = GetFirstObjectPtr(pInfo->pxBasePtrObjects);
            for (uint32_t i = 0; i < numObjects; ++i)
            {
                if (pInfo->vPos[0] >= pObj->iMinX && pObj->iMaxX >= pInfo->vPos[0] &&
                    pInfo->vPos[1] >= pObj->iMinY && pObj->iMaxY >= pInfo->vPos[1] &&
                    pInfo->vPos[2] >= pObj->iMinZ && pObj->iMaxZ >= pInfo->vPos[2])
                {
                    pInfo->pChkFunc(pObj->iID, pInfo);
                }
                pObj++;
            }
        }

        ZNodeCompiled* pChild = GetFirstChildPtr(pInfo->pxBasePtrNodes);
        if (pChild)
        {
            int iNodeDim = 0x10000 >> pInfo->iDepth;
            int iHalfDim = iNodeDim >> 1;

            bool isLast = false;
            do
            {
                uint16_t nodeID = pChild->GetNodeID();

                int iChildCenX = ((nodeID & 1) == 0) ? (iNodeCenterX - iHalfDim) : (iNodeCenterX + iHalfDim);
                int iChildCenY = ((nodeID & 2) != 0) ? (iNodeCenterY + iHalfDim) : (iNodeCenterY - iHalfDim);
                int iChildCenZ = ((nodeID & 4) != 0) ? (iNodeCenterZ + iHalfDim) : (iNodeCenterZ - iHalfDim);

                if (pInfo->vPos[0] < (iChildCenX + iNodeDim) && pInfo->vPos[0] >= (iChildCenX - iNodeDim) &&
                    pInfo->vPos[1] < (iChildCenY + iNodeDim) && pInfo->vPos[1] >= (iChildCenY - iNodeDim) &&
                    pInfo->vPos[2] < (iChildCenZ + iNodeDim) && pInfo->vPos[2] >= (iChildCenZ - iNodeDim))
                {
                    pChild->CheckPoint(pInfo, iChildCenX, iChildCenY, iChildCenZ);
                }

                isLast = pChild->IsLast();
                pChild++;
            } 
            while (!isLast);
        }
        
        pInfo->iDepth--;
    }

    bool ZNodeCompiled::CheckLinesegment2(SRecurseInfoCompiled* pInfo, int iNodeDim, int iCenX, int iCenY, int iCenZ)
    {
        const float fLinePosX = pInfo->pA[0];
        const float fLinePosY = pInfo->pA[1];
        const float fLinePosZ = pInfo->pA[2];
        const float fLineVecX = pInfo->pB[0] - fLinePosX;
        const float fLineVecY = pInfo->pB[1] - fLinePosY;
        const float fLineVecZ = pInfo->pB[2] - fLinePosZ;

        bool bHit = false;

        uint16_t numObjects = GetNumObjects();
        if (numObjects > 0)
        {
            CObjectInfo* pObject = GetFirstObjectPtr(pInfo->pxBasePtrObjects);
            for (uint16_t i = 0; i < numObjects; ++i)
            {
                float fT = -1.0f;
                if (ZCommonAlgorithms::LineVS_AABB(
                        static_cast<float>(pObject->iMinX), static_cast<float>(pObject->iMinY), static_cast<float>(pObject->iMinZ),
                        static_cast<float>(pObject->iMaxX), static_cast<float>(pObject->iMaxY), static_cast<float>(pObject->iMaxZ),
                        fLinePosX, fLinePosY, fLinePosZ,
                        fLineVecX, fLineVecY, fLineVecZ,
                        &fT) && pInfo->fResT > fT)
                {
                    bHit |= pInfo->pChkFunc(pObject->iID, pInfo);
                }

                pObject++;
            }
        }

        ZNodeCompiled* pChild = GetFirstChildPtr(pInfo->pxBasePtrNodes);
        if (pChild)
        {
            SlocSubNode queue[12];
            int iQueueLength = 0;

            const int iChildDim = iNodeDim >> 1;
            const int iChildOffset = iNodeDim >> 2;

            bool isLast = false;
            do
            {
                const uint16_t childBits = pChild->GetNodeID();
                const int iChildCenX = iCenX + ((childBits & 1) ? iChildOffset : -iChildOffset);
                const int iChildCenY = iCenY + ((childBits & 2) ? iChildOffset : -iChildOffset);
                const int iChildCenZ = iCenZ + ((childBits & 4) ? iChildOffset : -iChildOffset);

                float fT = -1.0f;
                if (ZCommonAlgorithms::LineVS_AABB(
                        static_cast<float>(iChildCenX - iChildDim), static_cast<float>(iChildCenY - iChildDim), static_cast<float>(iChildCenZ - iChildDim),
                        static_cast<float>(iChildCenX + iChildDim), static_cast<float>(iChildCenY + iChildDim), static_cast<float>(iChildCenZ + iChildDim),
                        fLinePosX, fLinePosY, fLinePosZ,
                        fLineVecX, fLineVecY, fLineVecZ,
                        &fT))
                {
                    SlocSubNode subNode;
                    subNode.fT = fT;
                    subNode.pNode = pChild;
                    subNode.uBits = childBits;
                    AddToHeap(queue, iQueueLength++, subNode);
                }

                isLast = pChild->IsLast();
                pChild++;
            }
            while (!isLast);

            for (int i = 0; i < iQueueLength; ++i)
            {
                SlocSubNode subNode;
                ExtractMin(&subNode, queue, iQueueLength - i);

                if (pInfo->fResT <= subNode.fT)
                    break;

                const int iChildCenX = iCenX + ((subNode.uBits & 1) ? iChildOffset : -iChildOffset);
                const int iChildCenY = iCenY + ((subNode.uBits & 2) ? iChildOffset : -iChildOffset);
                const int iChildCenZ = iCenZ + ((subNode.uBits & 4) ? iChildOffset : -iChildOffset);

                bHit |= static_cast<ZNodeCompiled*>(subNode.pNode)->CheckLinesegment2(pInfo, iChildDim, iChildCenX, iChildCenY, iChildCenZ);
            }
        }

        return bHit;
    }

    bool ZNodeCompiled::CheckCube(SRecurseInfoCompiled* pInfo, int iDepth, int iNodeCenterX, int iNodeCenterY, int iNodeCenterZ)
    {
        uint16_t numObjects = GetNumObjects();
        if (numObjects > 0)
        {
            CObjectInfo* pObject = GetFirstObjectPtr(pInfo->pxBasePtrObjects);

            for (uint16_t i = 0; i < numObjects; ++i)
            {
                if (pInfo->iMinX < pObject->iMaxX && pObject->iMinX < pInfo->iMaxX &&
                    pInfo->iMinY < pObject->iMaxY && pObject->iMinY < pInfo->iMaxY &&
                    pInfo->iMinZ < pObject->iMaxZ && pObject->iMinZ < pInfo->iMaxZ)
                {
                    pInfo->pChkFunc(pObject->iID, pInfo);
                }
                pObject++;
            }
        }

        if (m_iFirstChild && pInfo->pxBasePtrNodes)
        {
            int nextDepth = iDepth + 1;
            int halfDim = 0x10000 >> nextDepth;
            int quarterDim = halfDim >> 1;

            ZNodeCompiled* pChild = GetFirstChildPtr(pInfo->pxBasePtrNodes);

            while (true)
            {
                int childCenX = iNodeCenterX + ((pChild->m_iInfo & 1) ? quarterDim : -quarterDim);
                int childCenY = iNodeCenterY + ((pChild->m_iInfo & 2) ? quarterDim : -quarterDim);
                int childCenZ = iNodeCenterZ + ((pChild->m_iInfo & 4) ? quarterDim : -quarterDim);

                if (pInfo->iMinX < (childCenX + halfDim) && pInfo->iMaxX >= (childCenX - halfDim) &&
                    pInfo->iMinY < (childCenY + halfDim) && pInfo->iMaxY >= (childCenY - halfDim) &&
                    pInfo->iMinZ < (childCenZ + halfDim) && pInfo->iMaxZ >= (childCenZ - halfDim))
                {
                    if (pInfo->iMinX > (childCenX - halfDim) || pInfo->iMaxX < (childCenX + halfDim) ||
                        pInfo->iMinY > (childCenY - halfDim) || pInfo->iMaxY < (childCenY + halfDim) ||
                        pInfo->iMinZ > (childCenZ - halfDim) || pInfo->iMaxZ < (childCenZ + halfDim))
                    {
                        pChild->CheckCube(pInfo, nextDepth, childCenX, childCenY, childCenZ);
                    }
                    else
                    {
                        pChild->GetObjects(pInfo);
                    }
                }

                if (pChild->m_iInfo & 0x8000)
                    break;

                pChild++;
            }
        }

        return true;
    }
    
    bool ZNodeCompiled::IsLast() const
    {
        return (m_iInfo & 0x8000) != 0;
    }

    uint16_t ZNodeCompiled::GetNodeID() const
    {
        return m_iInfo & 7;
    }

    uint16_t ZNodeCompiled::GetNumObjects() const
    {
        return (m_iInfo >> 3) & 0xFFF;
    }

    CObjectInfo* ZNodeCompiled::GetFirstObjectPtr(const void* pxTree) const
    {
        const char* pBase = static_cast<const char*>(pxTree);
        return const_cast<CObjectInfo*>(reinterpret_cast<const CObjectInfo*>(pBase + sizeof(CObjectInfo) * m_iFirstObject));
    }

    ZNodeCompiled* ZNodeCompiled::GetFirstChildPtr(const void* pxTree) const
    {
        if (m_iFirstChild == 0)
            return nullptr;

        const char* pBase = static_cast<const char*>(pxTree);
        return const_cast<ZNodeCompiled*>(reinterpret_cast<const ZNodeCompiled*>(pBase + sizeof(ZNodeCompiled) * m_iFirstChild));
    }
}
