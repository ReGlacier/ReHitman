#include <Glacier/Fysix/ZCommonAlgorithms.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <Glacier/ZSTL/CMemPool.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <bit>


namespace Glacier
{
    bool ZCommonAlgorithms::LineVS_AABB(
        float fMinX, float fMinY, float fMinZ,
        float fMaxX, float fMaxY, float fMaxZ,
        float fLinePosX, float fLinePosY, float fLinePosZ,
        float fLineVecX, float fLineVecY, float fLineVecZ,
        float* pOutT)
    {
        ZASSERT(fMinX <= fMaxX && fMinY <= fMaxY && fMinZ <= fMaxZ);

        if (fLinePosX >= fMinX && fLinePosX <= fMaxX &&
            fLinePosY >= fMinY && fLinePosY <= fMaxY &&
            fLinePosZ >= fMinZ && fLinePosZ <= fMaxZ)
        {
            if (pOutT) *pOutT = 0.0f;
            return true;
        }

        float tX = -1.0f;
        float tY = -1.0f;
        float tZ = -1.0f;

        if (fLineVecX != 0.0f)
        {
            float fNearX = (fLineVecX >= 0.0f) ? fMinX : fMaxX;
            tX = (fNearX - fLinePosX) / fLineVecX;
        }

        if (fLineVecY != 0.0f)
        {
            float fNearY = (fLineVecY >= 0.0f) ? fMinY : fMaxY;
            tY = (fNearY - fLinePosY) / fLineVecY;
        }

        if (fLineVecZ != 0.0f)
        {
            float fNearZ = (fLineVecZ >= 0.0f) ? fMinZ : fMaxZ;
            tZ = (fNearZ - fLinePosZ) / fLineVecZ;
        }

        float tMax = -1.0f;

        if (tX >= tY && tX >= tZ)
        {
            tMax = tX;

            float hitY = (tMax * fLineVecY) + fLinePosY;
            float hitZ = (tMax * fLineVecZ) + fLinePosZ;

            if (hitY < fMinY || hitY > fMaxY || hitZ < fMinZ || hitZ > fMaxZ)
                return false;
        }
        else if (tY >= tX && tY >= tZ)
        {
            tMax = tY;

            float hitX = (tMax * fLineVecX) + fLinePosX;
            float hitZ = (tMax * fLineVecZ) + fLinePosZ;

            if (hitX < fMinX || hitX > fMaxX || hitZ < fMinZ || hitZ > fMaxZ)
                return false;
        }
        else
        {
            tMax = tZ;

            float hitX = (tMax * fLineVecX) + fLinePosX;
            float hitY = (tMax * fLineVecY) + fLinePosY;

            if (hitX < fMinX || hitX > fMaxX || hitY < fMinY || hitY > fMaxY)
                return false;
        }

        if (pOutT) 
            *pOutT = tMax;

        return (tMax >= 0.0f && tMax <= 1.0f);
    }

    /**
     * @brief Fast 2D line segment vs AABB intersection test on the XZ plane.
     * @details Used for rapid quadtree node pruning during ray casting.
     * 
     * @param fMinX   Minimum X boundary of the 2D node.
     * @param fMinZ   Minimum Z boundary of the 2D node.
     * @param fMaxX   Maximum X boundary of the 2D node.
     * @param fMaxZ   Maximum Z boundary of the 2D node.
     * @param pInfo   Pointer to the compiled ray/intersection context.
     * @param pOutT   Output parameter for the intersection time t.
     * @return true if the 2D segment intersects the node bounds within t in [0.0, 1.0].
     */
    bool ZCommonAlgorithms::Line2D_VS_AABB(
        float fMinX, float fMinZ, 
        float fMaxX, float fMaxZ, 
        SRecurseInfoCompiled* pInfo, 
        float* pOutT)
    {
        ZASSERT(fMinX <= fMaxX && fMinZ <= fMaxZ);

        float fLocalRayStartX = pInfo->pA[0] - fMinX;
        float fLocalRayStartZ = pInfo->pA[2] - fMinZ;
        
        float fNodeWidth = fMaxX - fMinX;
        float fNodeDepth = fMaxZ - fMinZ;

        if (fLocalRayStartX >= 0.0f && fLocalRayStartX <= fNodeWidth &&
            fLocalRayStartZ >= 0.0f && fLocalRayStartZ <= fNodeDepth)
        {
            if (pOutT) *pOutT = 0.0f;
            return true;
        }

        float tX = -1.0f;
        float tZ = -1.0f;

        if (pInfo->vDir.x != 0.0f)
        {
            tX = -fLocalRayStartX * pInfo->vInvDir.x;
        }

        if (pInfo->vDir.z != 0.0f)
        {
            tZ = -fLocalRayStartZ * pInfo->vInvDir.z;
        }

        float tMax = -1.0f;

        if (tZ > tX)
        {
            tMax = tZ;

            float fHitX = (tMax * pInfo->vDir.x) + fLocalRayStartX;
            if (fHitX < 0.0f || fHitX > fNodeWidth)
                return false;
        }
        else
        {
            tMax = tX;

            float fHitZ = (tMax * pInfo->vDir.z) + fLocalRayStartZ;
            if (fHitZ < 0.0f || fHitZ > fNodeDepth)
                return false;
        }

        if (pOutT) 
            *pOutT = tMax;

        return (tMax >= 0.0f && tMax <= 1.0f);
    }

    namespace
	{
        int Log2(int iVal)
        {
            ZASSERT(iVal > 0);
            return std::bit_width(static_cast<unsigned int>(iVal)) - 1;
        }
	}
	

    void CNodeQuad::Init(CMemPool* pPool, CNodeQuad* pParent)
    {
        if (pParent)
        {
            ZASSERT(pParent->m_iDepth >= 0 && pParent->m_iDepth <= 15);

            uintptr_t offset = reinterpret_cast<uintptr_t>(pParent) - reinterpret_cast<uintptr_t>(pPool->GetPoolPtr());
            m_iParent = static_cast<uint16_t>(offset / sizeof(CNodeQuad));

            ZASSERT(m_iParent >= 0 && m_iParent < pPool->GetNumEntries());
            m_iDepth = pParent->m_iDepth + 1;
        }
        else
        {
            // ROOT
            m_iParent = 0;
            m_iDepth = 0;
        }

        m_tObjectList.m_pHead = nullptr;

        m_aiChildren[0] = 0;
        m_aiChildren[1] = 0;
        m_aiChildren[2] = 0;
        m_aiChildren[3] = 0;
    }

    void CNodeQuad::DeleteEmpty(CMemPool* pPool)
    {
        if (m_iDepth != 0 && IsEmpty() && IsLeaf())
        {
            CNodeQuad* pParent = GetParentPtr(pPool);
            
            ZASSERT(m_iDepth == (pParent->GetDepth() + 1));
            
            for (int i = 0; i < 4; ++i)
            {
                if (pParent->GetChildPtr(pPool, i) == this)
                {
                    pParent->m_aiChildren[i] = 0;                    
                    pParent->DeleteEmpty(pPool);
                    pPool->DeAlloc(this);
                    return;
                }
            }
        }

    }

    void CNodeQuad::DeleteEmptySingle(CMemPool* pPool)
    {
        if (m_iDepth != 0 && IsEmpty() && IsLeaf())
        {
            CNodeQuad* pParent = GetParentPtr(pPool);
            
            ZASSERT(m_iDepth == (pParent->GetDepth() + 1));
            
            for (int i = 0; i < 4; ++i)
            {
                if (pParent->GetChildPtr(pPool, i) == this)
                {
                    pParent->m_aiChildren[i] = 0;
                    
                    pPool->DeAlloc(this);
                    return;
                }
            }
        }
    }

    void CNodeQuad::Add(SRecurseAdd* pInfo)
    {
        ZASSERT(pInfo != nullptr);

        if (pInfo->iCurDepth == pInfo->iWantedDepth)
        {
            Attach(pInfo->pObject);
            return;
        }

        pInfo->iCurDepth++;

        int iNodeDim = 0x10000 >> pInfo->iCurDepth;
        int mask = ~(iNodeDim - 1);

        int iChildIdx = 0;
        if ((pInfo->iX & mask) != 0) iChildIdx |= 1;
        if ((pInfo->iZ & mask) != 0) iChildIdx |= 2;

        pInfo->iX &= ~mask;
        pInfo->iZ &= ~mask;

        if (m_aiChildren[iChildIdx] == 0)
        {
            if (pInfo->pPool->IsFull())
            {
                printf("WARNING: Quadtree. Not enough nodes available - object will be placed at non-optimal depth!");
                
                CNodeQuad::Attach(pInfo->pObject);
                return;
            }

            int iNewNodeBlockNum = 0;
            CNodeQuad* pNewNode = reinterpret_cast<CNodeQuad*>(pInfo->pPool->Alloc(&iNewNodeBlockNum));
            
            m_aiChildren[iChildIdx] = static_cast<uint16_t>(iNewNodeBlockNum);            
            pNewNode->Init(pInfo->pPool, this);
        }

        CNodeQuad* pChild = GetChildPtr(pInfo->pPool, iChildIdx);
        ZASSERT(pChild->GetDepth() == (GetDepth() + 1));
        
        pChild->Add(pInfo);
    }

    void CNodeQuad::GetObjects(SRecurseInfoCompiled* pInfo, CMemPool* pPool)
    {
        CTreeObject* pCurrent = m_tObjectList.GetHead();
        while (pCurrent != nullptr)
        {
            if (pInfo->pChkFunc)
            {
                pInfo->pChkFunc(pCurrent->m_tInfo.iID, pInfo);
            }
            
            pCurrent = pCurrent->m_pNext;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (m_aiChildren[i] != 0)
            {
                CNodeQuad* pChild = GetChildPtr(pPool, i);                
                ZASSERT(pChild->GetDepth() == (GetDepth() + 1));
                
                pChild->GetObjects(pInfo, pPool);
            }
        }
    }
    
    void CNodeQuad::CheckCube(SRecurseInfoCompiled* pInfo, CMemPool* pPool, int iDepth, int iNodeCenterX, int iNodeCenterZ)
    {
        CTreeObject* pCurrent = m_tObjectList.GetHead();
        while (pCurrent != nullptr)
        {
            if (pCurrent->m_tInfo.IsOverlapAABB(pInfo->iMinX, pInfo->iMinY, pInfo->iMinZ, 
                                                pInfo->iMaxX, pInfo->iMaxY, pInfo->iMaxZ))
            {
                if (pInfo->pChkFunc)
                {
                    pInfo->pChkFunc(pCurrent->m_tInfo.iID, pInfo);
                }
            }
            pCurrent = pCurrent->m_pNext;
        }

        int iNextDepth = iDepth + 1;
        int iChildHalfSize = 0x10000 >> iNextDepth;
        int iOffsetToChildCenter = iChildHalfSize >> 1;

        for (int i = 0; i < 4; ++i)
        {
            if (m_aiChildren[i] != 0)
            {
                int iChildCenterX = (((i ^ 1) & 1) != 0) ? (iNodeCenterX - iOffsetToChildCenter) : (iNodeCenterX + iOffsetToChildCenter);
                int iChildCenterZ = ((i & 2) != 0) ? (iNodeCenterZ + iOffsetToChildCenter) : (iNodeCenterZ - iOffsetToChildCenter);

                int iChildMinX = iChildCenterX - iChildHalfSize;
                int iChildMaxX = iChildCenterX + iChildHalfSize;
                int iChildMinZ = iChildCenterZ - iChildHalfSize;
                int iChildMaxZ = iChildCenterZ + iChildHalfSize;

                if (pInfo->iMinX < iChildMaxX && pInfo->iMaxX >= iChildMinX &&
                    pInfo->iMinZ < iChildMaxZ && pInfo->iMaxZ >= iChildMinZ)
                {
                    if (iChildMinX < pInfo->iMinX || pInfo->iMaxX < iChildMaxX ||
                        iChildMinZ < pInfo->iMinZ || pInfo->iMaxZ < iChildMaxZ)
                    {
                        CNodeQuad* pChild = GetChildPtr(pPool, i);
                        pChild->CheckCube(pInfo, pPool, iNextDepth, iChildCenterX, iChildCenterZ);
                    }
                    else
                    {
                        CNodeQuad* pChild = GetChildPtr(pPool, i);
                        pChild->GetObjects(pInfo, pPool);
                    }
                }
            }
        }
    }
    
    bool CNodeQuad::CheckLinesegmentB(SRecurseInfoCompiled* pInfo, CMemPool* pPool, int iNodeDim, int iCenX, int iCenZ)
    {
        bool bHit = false;

        // Current node traverse
        CTreeObject* pCurrent = m_tObjectList.GetHead();
        while (pCurrent != nullptr)
        {
            CObjectInfo* pObj = &pCurrent->m_tInfo;

            if (ZCommonAlgorithms::LineVS_AABB(
                    static_cast<float>(pObj->iMinX), static_cast<float>(pObj->iMinY), static_cast<float>(pObj->iMinZ),
                    static_cast<float>(pObj->iMaxX), static_cast<float>(pObj->iMaxY), static_cast<float>(pObj->iMaxZ),
                    pInfo->pA[0], pInfo->pA[1], pInfo->pA[2],
                    pInfo->vDir.x, pInfo->vDir.y, pInfo->vDir.z,
                    &pInfo->fResT))
            {
                if (pInfo->pChkFunc)
                {
                    unsigned int uObjPtr = reinterpret_cast<unsigned int>(pObj);
                    bHit |= pInfo->pChkFunc(uObjPtr, pInfo);
                }
            }
            pCurrent = pCurrent->m_pNext;
        }

        int iChildDim = iNodeDim >> 1;
        int iOffsetToCenter = iNodeDim >> 2;

        // Child traverse
        for (int i = 0; i < 4; ++i)
        {
            if (m_aiChildren[i] != 0)
            {
                int iChildCenterX = (((i ^ 1) & 1) != 0) ? (iCenX - iOffsetToCenter) : (iCenX + iOffsetToCenter);
                int iChildCenterZ = ((i & 2) != 0) ? (iCenZ + iOffsetToCenter) : (iCenZ - iOffsetToCenter);

                if (ZCommonAlgorithms::Line2D_VS_AABB(
                        static_cast<float>(iChildCenterX - iChildDim),
                        static_cast<float>(iChildCenterZ - iChildDim),
                        static_cast<float>(iChildCenterX + iChildDim),
                        static_cast<float>(iChildCenterZ + iChildDim),
                        pInfo, nullptr))
                {
                    CNodeQuad* pChild = GetChildPtr(pPool, i);
                    bHit |= pChild->CheckLinesegmentB(pInfo, pPool, iChildDim, iChildCenterX, iChildCenterZ);
                }
            }
        }

        return bHit;
    }

    bool CNodeQuad::IsEmpty() const
    {
        return m_tObjectList.m_pHead == nullptr;
    }

    bool CNodeQuad::IsLeaf() const
    {
        ZASSERT((reinterpret_cast<uintptr_t>(&m_aiChildren[0]) & 0x7) == 0);
        return m_aiChildren[0] == 0 && m_aiChildren[1] == 0 && m_aiChildren[2] == 0 && m_aiChildren[3] == 0;
    }

    CNodeQuad* CNodeQuad::GetParentPtr(CMemPool* pPool) const
    {
        return reinterpret_cast<CNodeQuad*>(pPool->GetPtrFromBlockNum(m_iParent));
    }

    CNodeQuad* CNodeQuad::GetChildPtr(CMemPool* pPool, int iChildIdx) const
    {
        ZASSERT(iChildIdx >= 0 && iChildIdx < 4);
        return reinterpret_cast<CNodeQuad*>(pPool->GetPtrFromBlockNum(m_aiChildren[iChildIdx]));
    }

    int CNodeQuad::GetDepth() const
    {
        return m_iDepth;
    }

    void CNodeQuad::Attach(CQuadtreeObj* pObject)
    {
        m_tObjectList.Attach(pObject);
        pObject->m_pNode = this;
    }
    
    void CNodeQuad::Detach(CQuadtreeObj* pObject)
    {
        m_tObjectList.Detach(pObject);
    }

    void CNodeQuad::SetChild(int iChildIndex, int16_t nChildId)
    {
        ZASSERT(iChildIndex >= 0 && iChildIndex < 4);
        m_aiChildren[iChildIndex] = nChildId;
    }
    
    bool CNodeQuad::ChildExists(int iChildIndex) const
    {
        ZASSERT(iChildIndex >= 0 && iChildIndex < 4);
        return m_aiChildren[iChildIndex] != 0;
    }

    CQuadtree::CQuadtree(int nMaxNodes)
        : m_pRoot{nullptr}
        , m_tPool{}
        , m_iObjects{0}
        , m_fScale{0.125f}
        , m_vOrigin{0.f,0.f,0.f}
    {
        ZASSERT(nMaxNodes > 0 && nMaxNodes <= 0x10000);
        
        m_tPool.Init(sizeof(CNodeQuad), nMaxNodes, 1);

        m_pRoot = reinterpret_cast<CNodeQuad*>(m_tPool.Alloc());

        if (m_pRoot)
        {
            m_pRoot->Init(&m_tPool, 0);
        }
    }

    CQuadtree::~CQuadtree()
    {
        // DronCode: I'm not sure that this is necessary
        m_tPool.DeInit();
    }

	void CQuadtree::SetOrigin(const ZVector3& vOrigin)
    {
        m_vOrigin = vOrigin;
    }

    void CQuadtree::SetScale(float fScale)
    {
        ZASSERT(fScale > 0.0001f);
        m_fScale = fScale;
    }

    int CQuadtree::GetDepth(int iDimension) const
    {
        // Validate that the requested cell size doesn't exceed the global coordinate universe boundary
        // 0xFFFF = 65535, which defines the absolute max bounds of the Quadtree area.
        ZASSERT(iDimension < 0xFFFF);

        // Leaf node resolution boundary: if the box is smaller than 2x2 units, 
        // it cannot be subdivided further, hitting the absolute max depth floor (15).
        if (iDimension < 2)
        {
            return 0xF;
        }

        // Higher level nodes have smaller depth indices (Root = 0).
        // As iDimension decreases, Log2 decreases, causing the depth index to increase.
        return 0xF - Log2(iDimension);
    }

    void CQuadtree::AddMinMax(int iMinX, int iMinY, int iMinZ, int iMaxX, int iMaxY, int iMaxZ, unsigned int iID)
    {
        // Clamp to avoid out of bounds
        if (static_cast<unsigned int>(iMinX) >= 0x10000 || static_cast<unsigned int>(iMaxX) >= 0x10000 ||
            static_cast<unsigned int>(iMinY) >= 0x10000 || static_cast<unsigned int>(iMaxY) >= 0x10000 ||
            static_cast<unsigned int>(iMinZ) >= 0x10000 || static_cast<unsigned int>(iMaxZ) >= 0x10000)
        {
            iMinX = iMinX - iMaxX + 0xFFFF;
            iMaxX = 0xFFFF;
            
            iMinY = iMinY - iMaxY + 0xFFFF;
            iMaxY = 0xFFFF;
            
            iMinZ = iMinZ - iMaxZ + 0xFFFF;
            iMaxZ = 0xFFFF;
        }

        // Compute sizes & check boundaries
        int iSizeX = iMaxX - iMinX + 1;
        int iSizeY = iMaxY - iMinY + 1;
        int iSizeZ = iMaxZ - iMinZ + 1;

        ZASSERT(iSizeX < 0xFFFF);
        ZASSERT(iSizeY < 0xFFFF);
        ZASSERT(iSizeZ < 0xFFFF);

        // Compute center & find max bounds
        int iCenterX = iMinX + (iSizeX >> 1);
        int iCenterY = iMinY + (iSizeY >> 1);
        int iCenterZ = iMinZ + (iSizeZ >> 1);

        int iMaxDimension = iSizeZ;
        if (iSizeY >= iSizeX)
        {
            if (iSizeY > iSizeZ) 
                iMaxDimension = iSizeY;
        }
        else
        {
            if (iSizeX > iSizeZ) 
                iMaxDimension = iSizeX;
        }

        // Compute optimal depth for this object
        int iTargetDepth = GetDepth(iMaxDimension);

        // Create object
        CQuadtreeObj* pObject = CreateObject(iID);

        // Store to OCS world
        pObject->m_tInfo.iMinX = iMinX;
        pObject->m_tInfo.iMinY = iMinY;
        pObject->m_tInfo.iMinZ = iMinZ;
        pObject->m_tInfo.iMaxX = iMaxX;
        pObject->m_tInfo.iMaxY = iMaxY;
        pObject->m_tInfo.iMaxZ = iMaxZ;

        SRecurseAdd sAddInfo;
        sAddInfo.pPool = &m_tPool;
        sAddInfo.pObject = pObject;
        sAddInfo.iCurDepth = 0;
        sAddInfo.iWantedDepth = iTargetDepth;
        sAddInfo.iX = iCenterX;
        sAddInfo.iY = iCenterY;
        sAddInfo.iZ = iCenterZ;
        
        m_pRoot->Add(&sAddInfo);

        ++m_iObjects;
    }

    void CQuadtree::AddMinMax(const ZVector3& pMin, const ZVector3& pMax, unsigned int iID)
    {
        int vMin[3] { 0 };
        int vMax[3] { 0 };

        ConvToOCS(vMin, pMin);
        ConvToOCS(vMax, pMax);

        AddMinMax(vMin[0], vMin[1], vMin[2], vMax[0], vMax[1], vMax[2], iID);
    }

    void CQuadtree::Move(CQuadtreeObj* pObject, int* pMin, int* pMax)
    {
        ZASSERT(pObject != nullptr);
        ZASSERT(pObject->m_pNode != nullptr);
        ZASSERT(pMin[0] <= pMax[0]);
        ZASSERT(pMin[1] <= pMax[1]);
        ZASSERT(pMin[2] <= pMax[2]);

        // Check bounds
        if (pMin[0] < 0) pMin[0] = 0;
        if (pMin[1] < 0) pMin[1] = 0;
        if (pMin[2] < 0) pMin[2] = 0;

        if (pMax[0] > 0xFFFF) pMax[0] = 0xFFFF;
        if (pMax[1] > 0xFFFF) pMax[1] = 0xFFFF;
        if (pMax[2] > 0xFFFF) pMax[2] = 0xFFFF;

        // Compute new bounds
        int iNewSizeX = pMax[0] - pMin[0] + 1;
        int iNewSizeY = pMax[1] - pMin[1] + 1;
        int iNewSizeZ = pMax[2] - pMin[2] + 1;

        // Remember old center
        int iOldCenterX = pObject->m_tInfo.iMinX + ((pObject->m_tInfo.iMaxX - pObject->m_tInfo.iMinX + 1) >> 1);
        int iOldCenterZ = pObject->m_tInfo.iMinZ + ((pObject->m_tInfo.iMaxZ - pObject->m_tInfo.iMinZ + 1) >> 1);

        // Store new pos
        pObject->m_tInfo.iMinX = static_cast<uint16_t>(pMin[0]);
        pObject->m_tInfo.iMinY = static_cast<uint16_t>(pMin[1]);
        pObject->m_tInfo.iMinZ = static_cast<uint16_t>(pMin[2]);
        pObject->m_tInfo.iMaxX = static_cast<uint16_t>(pMax[0]);
        pObject->m_tInfo.iMaxY = static_cast<uint16_t>(pMax[1]);
        pObject->m_tInfo.iMaxZ = static_cast<uint16_t>(pMax[2]);

        // Find new dimension in tree
        int iMaxDimension = iNewSizeZ;
        if (iNewSizeY >= iNewSizeX)
        {
            if (iNewSizeY > iNewSizeZ) 
                iMaxDimension = iNewSizeY;
        }
        else
        {
            if (iNewSizeX > iNewSizeZ) 
                iMaxDimension = iNewSizeX;
        }

        // Compute new center & depth
        int iNewCenterX = pObject->m_tInfo.iMinX + (iNewSizeX >> 1);
        int iNewCenterZ = pObject->m_tInfo.iMinZ + (iNewSizeZ >> 1);

        int iCurrentNodeDepth = pObject->m_pNode->GetDepth();
        int iCurrentNodeSize  = 0x10000 >> iCurrentNodeDepth;

        if (iCurrentNodeSize < iMaxDimension || iMaxDimension < (iCurrentNodeSize >> 1))
        {
            // Slow path: drop old node and put new
            CNodeQuad* pOldNode = pObject->m_pNode;
            Detach(pObject);
            m_pRoot->DeleteEmpty(&m_tPool);
            pOldNode->DeleteEmpty(&m_tPool);

            // Compute new depth
            int iTargetDepth = GetDepth(iMaxDimension);

            SRecurseAdd sAddInfo;
            sAddInfo.pPool        = &m_tPool;
            sAddInfo.pObject      = pObject;
            sAddInfo.iCurDepth    = 0;
            sAddInfo.iWantedDepth = iTargetDepth;
            sAddInfo.iX           = iNewCenterX;
            sAddInfo.iY           = 0;
            sAddInfo.iZ           = iNewCenterZ;

            m_pRoot->Add(&sAddInfo);
            return;
        }

        ZASSERT(pObject->m_tInfo.iMinX <= pObject->m_tInfo.iMaxX);
        ZASSERT(pObject->m_tInfo.iMinY <= pObject->m_tInfo.iMaxY);
        ZASSERT(pObject->m_tInfo.iMinZ <= pObject->m_tInfo.iMaxZ);

        // Check is new center still in bounds of current node
        int iNodeOriginX = iOldCenterX & ~(iCurrentNodeSize - 1);
        int iNodeOriginZ = iOldCenterZ & ~(iCurrentNodeSize - 1);

        if (iNewCenterX >= iNodeOriginX && iNewCenterX < (iNodeOriginX + iCurrentNodeSize) &&
            iNewCenterZ >= iNodeOriginZ && iNewCenterZ < (iNodeOriginZ + iCurrentNodeSize))
        {
            // Still in bounds, will stop here because everything is ok
            return;
        }

        // Fast path: replace through parent node
        {
            CNodeQuad* pParent = pObject->m_pNode->GetParentPtr(&m_tPool);
            ZASSERT(pParent != nullptr);
            ZASSERT(pParent->GetDepth() >= 0 && pParent->GetDepth() <= 15);

            int iParentNodeSize    = 2 * iCurrentNodeSize;
            int iParentNodeOriginX = iOldCenterX & ~(iParentNodeSize - 1);
            int iParentNodeOriginZ = iOldCenterZ & ~(iParentNodeSize - 1);

            // Is our center in bounds of neighbour quadrant
            if (iNewCenterX >= iParentNodeOriginX && iNewCenterX < (iParentNodeOriginX + iParentNodeSize) &&
                iNewCenterZ >= iParentNodeOriginZ && iNewCenterZ < (iParentNodeOriginZ + iParentNodeSize))
            {
                // Need to find which one of 4 quadrants can fit our new center
                int iChildIndex = (iNewCenterX & iCurrentNodeSize) != 0 ? 1 : 0;
                if ((iNewCenterZ & iCurrentNodeSize) != 0)
                {
                    iChildIndex += 2; // Switch to the next row ([2] & [3])
                }

                // Detach object from prev node
                Detach(pObject);

                // Remove old node (single node)
                pObject->m_pNode->DeleteEmptySingle(&m_tPool);

                // Lookup  for new insertion quad
                CNodeQuad* pInsertNode = nullptr;
                if (pParent->ChildExists(iChildIndex))
                {
                    pInsertNode = pParent->GetChildPtr(&m_tPool, iChildIndex);
                }
                else
                {
                    // If no new quad allocated yet - allocate a new one
                    int iNodeHandle;
                    pInsertNode = static_cast<CNodeQuad*>(m_tPool.Alloc(&iNodeHandle));
                    ZASSERT(pInsertNode != nullptr);

                    pParent->SetChild(iChildIndex, iNodeHandle);
                    pInsertNode->Init(&m_tPool, pParent);
                }

                // Check alloc & attach
                ZASSERT(pInsertNode != nullptr);
                pInsertNode->Attach(pObject);
                return;
            }
        }

        CNodeQuad* pOldNode = pObject->m_pNode;
        Detach(pObject);
        pOldNode->DeleteEmpty(&m_tPool);

        int iTargetDepth = GetDepth(iMaxDimension);

        SRecurseAdd sAddInfo;
        sAddInfo.pPool        = &m_tPool;
        sAddInfo.pObject      = pObject;
        sAddInfo.iCurDepth    = 0;
        sAddInfo.iWantedDepth = iTargetDepth;
        sAddInfo.iX           = iNewCenterX;
        sAddInfo.iY           = 0;
        sAddInfo.iZ           = iNewCenterZ;

        m_pRoot->Add(&sAddInfo);
    }

    void CQuadtree::Move(CQuadtreeObj* pObject, const ZVector3& pMin, const ZVector3& pMax)
    {
        int vMin[3] { 0 };
        int vMax[3] { 0 };

        ConvToOCS(&vMin[0], pMin);
        ConvToOCS(&vMax[0], pMax);

        Move(pObject, &vMin[0], &vMax[0]);
    }

    void CQuadtree::Delete(CQuadtreeObj *pObject)
    {
        ZASSERT(pObject != nullptr);

        Detach(pObject);
        --m_iObjects;

        CNodeQuad* pOwner = pObject->m_pNode;
        pOwner->DeleteEmpty(&m_tPool);

        std::memset(pObject, 0, sizeof(CQuadtreeObj)); // DronCode: Really? Ok)
        ZUniMemory::Delete<CQuadtreeObj>(pObject);
    }

    void CQuadtree::Detach(CQuadtreeObj* pObject)
    {
        ZASSERT(pObject != nullptr);
        ZASSERT(pObject->m_pNode != nullptr);

        pObject->m_pNode->Detach(pObject);
    }
    
    void CQuadtree::GetEverything(SRecurseInfoCompiled* pInfo)
    {
        ZASSERT(pInfo->pChkFunc);
        m_pRoot->GetObjects(pInfo, &m_tPool);
    }

    void CQuadtree::CheckCube(SRecurseInfoCompiled* pInfo, const ZVector3& pMin, const ZVector3& pMax)
    {
        ZASSERT(pInfo->pChkFunc);

        int vMin[3] { 0 };
        int vMax[3] { 0 };

        ConvToOCS(&vMin[0], pMin);
        ConvToOCS(&vMax[0], pMax);

        CheckCube(pInfo, vMin[0], vMin[1], vMin[2], vMax[0], vMax[1], vMax[2]);
    }

    void CQuadtree::CheckCube(SRecurseInfoCompiled* pInfo, int iMinX, int iMinY, int iMinZ, int iMaxX, int iMaxY, int iMaxZ)
    {
        ZASSERT(pInfo);
        ZASSERT(pInfo->pChkFunc);
        ZASSERT(iMinX <= iMaxX);
        ZASSERT(iMinY <= iMaxY);
        ZASSERT(iMinZ <= iMaxZ);

        pInfo->iMinX = iMinX;
        pInfo->iMinY = iMinY;
        pInfo->iMinZ = iMinZ;
        pInfo->iMaxX = iMaxX;
        pInfo->iMaxY = iMaxY;
        pInfo->iMaxZ = iMaxZ;

        m_pRoot->CheckCube(pInfo, &m_tPool, 0, 0x8000, 0x8000);
    }

    bool CQuadtree::CheckLinesegment(SRecurseInfoCompiled *pInfo, const ZVector3& pA, const ZVector3& pB)
    {
        ZASSERT(pInfo->pChkFunc != nullptr);

        float vOcsA[4];
        float vOcsB[4];

        vOcsA[0] = (pA.x - m_vOrigin.x) * m_fScale + 32768.0f;
        vOcsA[1] = (pA.y - m_vOrigin.y) * m_fScale + 32768.0f;
        vOcsA[2] = (pA.z - m_vOrigin.z) * m_fScale + 32768.0f;
        vOcsB[0] = (pB.x - m_vOrigin.x) * m_fScale + 32768.0f;
        vOcsB[1] = (pB.y - m_vOrigin.y) * m_fScale + 32768.0f;
        vOcsB[2] = (pB.z - m_vOrigin.z) * m_fScale + 32768.0f;

        pInfo->pA = vOcsA;
        pInfo->pB = vOcsB;

        pInfo->vDir.x = vOcsB[0] - vOcsA[0];
        pInfo->vDir.y = vOcsB[1] - vOcsA[1];
        pInfo->vDir.z = vOcsB[2] - vOcsA[2];

        if (pInfo->vDir.x != 0.0f)
        {
            pInfo->vInvDir.x = 1.0f / pInfo->vDir.x;
        }
        
        if (pInfo->vDir.z != 0.0f)
        {
            pInfo->vInvDir.z = 1.0f / pInfo->vDir.z;
        }

        return m_pRoot->CheckLinesegmentB(pInfo, &m_tPool, 0x10000, 0x8000, 0x8000);
    }

    void CQuadtree::ConvToOCS(int* pOutOcsPos, const float* pInWorldPos) const
    {
        ZASSERT(m_fScale > 0.0f);

        pOutOcsPos[0] = static_cast<int>((pInWorldPos[0] - m_vOrigin.x) * m_fScale + 32768.0f);
        pOutOcsPos[1] = static_cast<int>((pInWorldPos[1] - m_vOrigin.y) * m_fScale + 32768.0f);
        pOutOcsPos[2] = static_cast<int>((pInWorldPos[2] - m_vOrigin.z) * m_fScale + 32768.0f);
    }

    void CQuadtree::ConvToOCS(int* pOutOcsPos, const ZVector3& pInWorldPos) const
    {
        ConvToOCS(pOutOcsPos, &pInWorldPos.x);
    }

    CQuadtreeObj* CQuadtree::CreateObject(int iID) const
    {
        return ZUniMemory::New<CQuadtreeObj>(iID);
    }
}
