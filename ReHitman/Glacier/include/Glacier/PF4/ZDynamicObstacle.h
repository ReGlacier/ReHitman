#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/PF4/ZObstacle.h>
#include <Glacier/PF4/ZLocation.h>

#include <cstdint>

namespace Glacier::PF4
{
    struct ZDynamicObstacle;

    // Obstacle type/mask (PF4RunTime::EType on PC).
    enum EType : int32_t
    {
        eSolid = 0x1,
        eWarning = 0x2,
        eCustom = 0x4,
    };

    // Node of the per-component obstacle list. Every "in use" obstacle link of a
    // ZDynamicObstacle is chained into the component list head stored in
    // ZData::m_pDynamicObstacles[component]. m_This points back to the obstacle.
    struct ZDynamicObstacle_Link
    {
        ZDynamicObstacle_Link* m_Next;
        ZDynamicObstacle_Link* m_Prev;
        ZDynamicObstacle* m_This;
    };
    RE_VERIFY_SIZE(ZDynamicObstacle_Link, 0xC);

    struct ZDynamicObstacle : public ZObstacle
    {
        // methods
        ZDynamicObstacle();
        ZDynamicObstacle(const float (&vBox)[3], int iType);
        ~ZDynamicObstacle() = default;

        ZDynamicObstacle& operator=(const ZDynamicObstacle& rhs);

        void CreateHullFromBox(const float (&vExtents)[2]);
        void DirXZ(const float (&vDir)[3]);
        bool Active() const;
        bool IsInside(const float* pvPos) const;
        float ClosestPoint(const float* pvFrom, float* pvFound) const;

        // Casts the ray from pvStart towards pvEnd (world X/Z pairs) against the
        // obstacle's convex hull. On entry *iRightId / *iLeftId are set to -1.
        // When a crossing closer than both fBest and fMaxDistance is found the
        // crossing distance is returned and the two hull vertices whose sight
        // lines graze the obstacle (the right/left silhouette corners as seen
        // from pvStart) are stored into iRightId / iLeftId and their world X/Z
        // positions into pvTangentPoints[0] / pvTangentPoints[1]. Otherwise fBest
        // is returned unchanged (callers accumulate the best hit over obstacles).
        // Returns -1 for a degenerate (zero length) ray. (PC 004DAC20 / PS2 0x1F24D8)
        float Intersect(const float* pvStart, const float* pvEnd, float fMaxDistance, float fBest,
            float (*pvTangentPoints)[2], int& iRightId, int& iLeftId);

        // Walks the hull in both directions starting at the two silhouette
        // vertices (iRightId backwards, iLeftId forwards) and returns in
        // pvRoutePoint the world X/Z of the vertex that starts the shorter route
        // from pvFrom to pvTarget. (PC 004D9E80 / PS2 0x1F1D70)
        void FindShortestRouteToTarget(const float* pvFrom, const float* pvTarget, int iRightId,
            int iLeftId, float* pvRoutePoint);

        // members
        ZVector2 m_ConvexHull[10];           // +0x00 (10 points, 80 bytes)
        int m_HullSize;                      // +0x50
        EType m_Type;                        // +0x54
        float m_Radius;                      // +0x58
        ZVector2 m_Dir;                      // +0x5C
        ZLocation m_Location;                // +0x64
        ZDynamicObstacle_Link m_Linking[16]; // +0x78 (16 links, 192 bytes)
        int16_t m_Component[32];             // +0x138 (64 bytes)
        int m_iLinks;                        // +0x178
        // Total structure size: 380 bytes (0x17C)
    };
    RE_VERIFY_SIZE(ZDynamicObstacle, 0x17C);
    RE_VERIFY_OFFSET(ZDynamicObstacle, m_HullSize, 0x50);
    RE_VERIFY_OFFSET(ZDynamicObstacle, m_Type, 0x54);
    RE_VERIFY_OFFSET(ZDynamicObstacle, m_Radius, 0x58);
    RE_VERIFY_OFFSET(ZDynamicObstacle, m_Location, 0x64);
    RE_VERIFY_OFFSET(ZDynamicObstacle, m_Linking, 0x78);
    RE_VERIFY_OFFSET(ZDynamicObstacle, m_Component, 0x138);
    RE_VERIFY_OFFSET(ZDynamicObstacle, m_iLinks, 0x178);
}
