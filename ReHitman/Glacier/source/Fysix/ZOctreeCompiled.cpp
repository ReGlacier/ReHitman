#include <Glacier/Fysix/ZCommonAlgorithms.h>
#include <Glacier/Fysix/ZOctreeCompiled.h>
#include <Glacier/ZSTL/CObjectInfo.h>
#include <Glacier/ZSTL/CQuadtree.h> // SRecurseInfoCompiled
#include <Glacier/ZUniAssert.h>
#include <cmath>


namespace Glacier
{
    bool ZCommonAlgorithms::LineVS_AAbox(
        const Vector3& vStart, const Vector3& vDir, const Vector3& vMin, 
        const Vector3& vMax, 
        float& fOutT, 
        float fMaxT)
    {
        if (vStart.x >= vMin.x && vStart.x <= vMax.x &&
            vStart.y >= vMin.y && vStart.y <= vMax.y &&
            vStart.z >= vMin.z && vStart.z <= vMax.z)
        {
            fOutT = 0.0f;
            return true;
        }

        float tMin = -1.0f;

        if (std::abs(vDir.x) > 1e-6f) 
        {
            float t = ((vDir.x > 0.0f ? vMin.x : vMax.x) - vStart.x) / vDir.x;
            if (t > tMin) tMin = t;
        }
        
        if (std::abs(vDir.y) > 1e-6f) 
        {
            float t = ((vDir.y > 0.0f ? vMin.y : vMax.y) - vStart.y) / vDir.y;
            if (t > tMin) tMin = t;
        }
        
        if (std::abs(vDir.z) > 1e-6f) 
        {
            float t = ((vDir.z > 0.0f ? vMin.z : vMax.z) - vStart.z) / vDir.z;
            if (t > tMin) tMin = t;
        }

        if (tMin < 0.0f || tMin >= fMaxT) 
            return false;

        Vector3 vHit = { vStart.x + vDir.x * tMin, vStart.y + vDir.y * tMin, vStart.z + vDir.z * tMin };
        
        const float eps = 1e-3f;
        if (vHit.x < vMin.x - eps || vHit.x > vMax.x + eps ||
            vHit.y < vMin.y - eps || vHit.y > vMax.y + eps ||
            vHit.z < vMin.z - eps || vHit.z > vMax.z + eps)
        {
            return false;
        }

        fOutT = tMin;
        return true;
    }

    struct SBinaryOctreeHeader
    {
        uint32_t m_iObjectsOffset;
        ZVector3 m_vOrigin;
        float    m_fScale;
    };

    // ZOctreeCompiled
    void ZOctreeCompiled::RemapObjects(RemapObjectIdFn pFnRemap)
    {
        if (!m_pxTree) return;

        SRecurseInfoCompiled recurseInfo;
        recurseInfo.pxBasePtrNodes = GetBasePtrNodes();
        recurseInfo.pxBasePtrObjects = GetBasePtrObjects();

        // Weird but like in game
        recurseInfo.pUserData = reinterpret_cast<void*>(pFnRemap); 

        ZNodeCompiled* pRoot = GetBasePtrNodes();
        if (pRoot)
        {
            pRoot->RemapObjects(&recurseInfo);
        }
    }

    bool ZOctreeCompiled::CheckLinesegment(SRecurseInfoCompiled* pInfo, float* pvA, float* pvB)
    {
        return CheckLinesegment2(pInfo, pvA, pvB);
    }

    void ZOctreeCompiled::CheckCube(SRecurseInfoCompiled* pInfo, float* pvMin, float* pvMax)
    {
        ZVector3 vMin { pvMin };
        ZVector3 vMax { pvMax };

        ConvToOCS_NoAssert(&pInfo->iMinX, vMin);
        ConvToOCS_NoAssert(&pInfo->iMaxX, vMax);

        if (pInfo->iMinX < 0x10000 && pInfo->iMinY < 0x10000 && pInfo->iMinZ < 0x10000 && pInfo->iMaxX > 0 && pInfo->iMaxY > 0 && pInfo->iMaxZ > 0)
        {
            pInfo->pxBasePtrNodes   = GetBasePtrNodes();
            pInfo->pxBasePtrObjects = GetBasePtrObjects();
            pInfo->iDepth           = 0;            

            ZNodeCompiled* pRootNode = GetBasePtrNodes();
            pRootNode->CheckCube(pInfo, 0, 0x8000, 0x8000, 0x8000);
        }
    }

    void ZOctreeCompiled::GetEverything(SRecurseInfoCompiled* pInfo)
    {
        pInfo->pxBasePtrNodes = GetBasePtrNodes();
        pInfo->pxBasePtrObjects = GetBasePtrObjects();

        ZNodeCompiled* pRoot = GetBasePtrNodes();
        if (pRoot)
        {
            pRoot->GetObjects(pInfo);
        }
    }

    void ZOctreeCompiled::CheckPoint(SRecurseInfoCompiled* pInfo, const float* pvPos)
    {
        ZVector3 vPos{pvPos};
        
        ConvToOCS_NoAssert(pInfo->vPos, vPos);

        pInfo->pxBasePtrNodes = GetBasePtrNodes();
        pInfo->pxBasePtrObjects = GetBasePtrObjects();
        pInfo->iDepth = 0;

        auto* pRoot = GetBasePtrNodes();
        ZASSERT(pRoot != nullptr);

        pRoot->CheckPoint(pInfo, 0x8000, 0x8000, 0x8000);
    }

    // methods
    ZOctreeCompiled::ZOctreeCompiled() 
        : m_pxTree{nullptr}
    {}

    void ZOctreeCompiled::Init(void* pxTree)
    {
        ZASSERT(pxTree != nullptr);

        m_pxTree = pxTree;
        SBinaryOctreeHeader* pHeader = static_cast<SBinaryOctreeHeader*>(m_pxTree);

        ZOctree::SetScale(pHeader->m_fScale);
        ZOctree::SetOrigin(pHeader->m_vOrigin);
    }

    void* ZOctreeCompiled::GetTreeDataPtr() const
    {
        return m_pxTree;
    }

    CObjectInfo* ZOctreeCompiled::GetBasePtrObjects() const
    {
        if (!m_pxTree) 
            return nullptr;
        
        SBinaryOctreeHeader* pHeader = static_cast<SBinaryOctreeHeader*>(m_pxTree);
        char* pBytes = static_cast<char*>(m_pxTree);
        return reinterpret_cast<CObjectInfo*>(pBytes + pHeader->m_iObjectsOffset);
    }

    ZNodeCompiled* ZOctreeCompiled::GetBasePtrNodes() const
    {
        if (!m_pxTree) 
            return nullptr;
        
        char* pBytes = static_cast<char*>(m_pxTree);
        return reinterpret_cast<ZNodeCompiled*>(pBytes + sizeof(SBinaryOctreeHeader));
    }

    bool ZOctreeCompiled::CheckLinesegment2(SRecurseInfoCompiled* pInfo, float* pA, float* pB)
    {
        // NOTE: NEXT CODE IS OCS!!!
        ZVector3 vSA { pA };
        ZVector3 vSB { pB };

        // Transformed vectors
        ZVector3 vA = ((vSA - m_vOrigin) * m_fScale) + 32768.f;
        ZVector3 vB = ((vSB - m_vOrigin) * m_fScale) + 32768.f;

        // Store ptrs
        pInfo->pA = (float*)&vA;
        pInfo->pB = (float*)&vB;

        auto* pRoot = GetBasePtrNodes();
        pInfo->pxBasePtrObjects = GetBasePtrObjects();
        pInfo->pxBasePtrNodes = pRoot;

        return pRoot->CheckLinesegment2(pInfo, 0x10000, 0x8000, 0x8000, 0x8000);
    }

    bool ZOctreeCompiled::CheckCube(SRecurseInfoCompiled* pInfo, int iMinX, int iMinY, int iMinZ, int iMaxX, int iMaxY, int iMaxZ)
    {
        auto* pRoot = GetBasePtrNodes();
        ZASSERT(pRoot != nullptr);

        pInfo->pxBasePtrNodes = pRoot;
        pInfo->pxBasePtrObjects = GetBasePtrObjects();
        pInfo->iMinX = iMinX;
        pInfo->iMinY = iMinY;
        pInfo->iMinZ = iMinZ;
        pInfo->iMaxX = iMaxX;
        pInfo->iMaxY = iMaxY;
        pInfo->iMaxZ = iMaxZ;
        pInfo->iDepth = 0;

        return pRoot->CheckCube(pInfo, 0, 0x8000, 0x8000, 0x8000);
    }
}
