#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/PF4/ZObstacle.h>
#include <Glacier/PF4/ZData.h>

#include <cstdint>

namespace Glacier::PF4
{
    // A static (pre-baked) line segment obstacle from the navmesh file. Unlike
    // ZDynamicObstacle it has no hull of its own: it refers to a run of world
    // vertices inside ZData::m_pVertices starting at m_FirstVertex. Every
    // consecutive pair (with wrap-around) is a wall candidate when a ray
    // crosses the polygon. (PC 004DA9A0 / PS2 0x1F1190 / xexe linesegment.cpp)
    struct ZStaticObstacle : public ZObstacle
    {
        // methods
        ZStaticObstacle() = default;
        ZStaticObstacle& operator=(const ZStaticObstacle& rhs);

        // Casts the ray from vStart towards vEnd (world X/Z pairs) against the
        // obstacle's vertex ring and finds the nearest crossing edge. The
        // crossing must be closer than both fBest and fMaxDistance; when found
        // the new distance is returned and the two world vertices of that edge
        // are stored into pvFoundPoints as two (x, height, z) triples with the
        // height filled with 0. Otherwise fBest is returned unchanged and
        // pvFoundPoints is untouched. (PC 004DA9A0 / PS2 0x1F1190)
        float Intersect(const ZData& rData, const ZVector2& vStart, const ZVector2& vEnd,
            float fMaxDistance, float fBest, float (*pvFoundPoints)[3]);

        // members
        ZIndex m_FirstVertex; // +0x00 first vertex in ZData::m_pVertices
        ZIndex m_Vertices;    // +0x02 number of obstacle vertices
    };
    RE_VERIFY_SIZE(ZStaticObstacle, 0x4);
    RE_VERIFY_OFFSET(ZStaticObstacle, m_FirstVertex, 0x0);
    RE_VERIFY_OFFSET(ZStaticObstacle, m_Vertices, 0x2);
}