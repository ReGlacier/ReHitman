#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZOctree.h>
#include <cstdint>


namespace Glacier
{
    struct SRecurseInfoCompiled;
    struct CObjectInfo;
    struct ZNodeCompiled;

    struct ZNodeCompiled
    {
        // methods
        void GetObjects(SRecurseInfoCompiled* pInfo);
        void RemapObjects(SRecurseInfoCompiled* pInfo);
        void CheckPoint(SRecurseInfoCompiled* pInfo, int iNodeCenterX, int iNodeCenterY, int iNodeCenterZ);
        bool CheckLinesegment2(SRecurseInfoCompiled* pInfo, int iNodeDim, int iCenX, int iCenY, int iCenZ);
        bool CheckCube(SRecurseInfoCompiled* pInfo, int iDepth, int iNodeCenterX, int iNodeCenterY, int iNodeCenterZ);
        bool IsLast() const;
        uint16_t GetNodeID() const;
        uint16_t GetNumObjects() const;
        CObjectInfo* GetFirstObjectPtr(const void* pxTree) const;
        ZNodeCompiled* GetFirstChildPtr(const void* pxTree) const;

        // members
        uint16_t m_iInfo;
        uint16_t m_iFirstChild;
        uint16_t m_iFirstObject;
    };
    RE_VERIFY_SIZE(ZNodeCompiled, 0x6);

    struct ZOctreeCompiled : public ZOctree
    {
        // vtbl
        void RemapObjects(RemapObjectIdFn) override;
        bool CheckLinesegment(SRecurseInfoCompiled* pInfo, float* pvA, float* pvB) override;
        void CheckCube(SRecurseInfoCompiled* pInfo, float* pvMin, float* pvMax) override;
        void GetEverything(SRecurseInfoCompiled* pInfo) override;
        void CheckPoint(SRecurseInfoCompiled* pInfo, const float* pvPos) override;

        // methods
        ZOctreeCompiled();

        void Init(void* pxTree);
        void* GetTreeDataPtr() const;
        CObjectInfo* GetBasePtrObjects() const;
        ZNodeCompiled* GetBasePtrNodes() const;
        bool CheckLinesegment2(SRecurseInfoCompiled* pInfo, float* pA, float* pB);
        bool CheckCube(SRecurseInfoCompiled* pInfo, int iMinX, int iMinY, int iMinZ, int iMaxX, int iMaxY, int iMaxZ);

        // members
        void* m_pxTree;
    };
    RE_VERIFY_SIZE(ZOctreeCompiled, 0x18); // Verified by alloc at ZCollisionBase::InstallInsideBuffer
}