#include <Glacier/PF4/ZStaticObstacle.h>
#include <Glacier/ZUniAssert.h>

#include <cmath>


namespace Glacier::PF4
{

    ZStaticObstacle& ZStaticObstacle::operator=(const ZStaticObstacle& rhs)
    {
        m_FirstVertex = rhs.m_FirstVertex;
        m_Vertices = rhs.m_Vertices;
        return *this;
    }

    float ZStaticObstacle::Intersect(const ZData& rData, const ZVector2& vStart, const ZVector2& vEnd,
        float fMaxDistance, float fBest, float (*pvFoundPoints)[3])
    {
        const int iVertexCount = m_Vertices;
        if (iVertexCount <= 0)
        {
            return fBest;
        }

        // Ray direction from start to end (world X/Z).
        const float fDx = vEnd.x - vStart.x;
        const float fDy = vEnd.y - vStart.y;
        const float fRayLen = std::sqrt(fDx * fDx + fDy * fDy);
        if (fRayLen <= 0.0f)
        {
            return -1.0f;
        }

        const float fDirX = fDx / fRayLen;
        const float fDirY = fDy / fRayLen;

        // Find the first ring edge (i, i+1) whose two vertices lie on
        // opposite sides of the ray line (the ray enters through this edge).
        int iHitEdge = iVertexCount;
        for (int i = 0; i < iVertexCount; ++i)
        {
            const int iNext = (i + 1 == iVertexCount) ? 0 : i + 1;
            const ZVertex& vertA = rData.m_pVertices[m_FirstVertex + i];
            const ZVertex& vertB = rData.m_pVertices[m_FirstVertex + iNext];

            const float fSideA = (vertA.m_kPos.x - vStart.x) * fDirY - (vertA.m_kPos.y - vStart.y) * fDirX;
            const float fSideB = (vertB.m_kPos.x - vStart.x) * fDirY - (vertB.m_kPos.y - vStart.y) * fDirX;
            if (fSideA <= 0.0f && fSideB > 0.0f)
            {
                iHitEdge = i;
                break;
            }
        }

        if (iHitEdge == iVertexCount)
        {
            return fBest;
        }

        // Distance along the ray to the chosen edge line.
        const int iHitNext = (iHitEdge + 1 == iVertexCount) ? 0 : iHitEdge + 1;
        const ZVertex& vertA = rData.m_pVertices[m_FirstVertex + iHitEdge];
        const ZVertex& vertB = rData.m_pVertices[m_FirstVertex + iHitNext];

        const float fEdgeX = vertB.m_kPos.x - vertA.m_kPos.x;
        const float fEdgeY = vertB.m_kPos.y - vertA.m_kPos.y;
        const float fOriginToEdgeX = vStart.x - vertA.m_kPos.x;
        const float fOriginToEdgeY = vStart.y - vertA.m_kPos.y;
        const float fCrossOriginEdge = fOriginToEdgeX * fEdgeY - fOriginToEdgeY * fEdgeX;
        const float fCrossDirEdge = fDirX * fEdgeY - fDirY * fEdgeX;
        const float fT = -fCrossOriginEdge / fCrossDirEdge;

        if (!(fT > 0.0f && fT < fMaxDistance && fT < fBest))
        {
            return fBest;
        }

        // The two world vertices of the crossing edge, stored as (x, 0, z)
        // triples; the height slot is never filled by the engine.
        pvFoundPoints[0][0] = vertA.m_kPos.x;
        pvFoundPoints[0][1] = 0.0f;
        pvFoundPoints[0][2] = vertA.m_kPos.y;
        pvFoundPoints[1][0] = vertB.m_kPos.x;
        pvFoundPoints[1][1] = 0.0f;
        pvFoundPoints[1][2] = vertB.m_kPos.y;

        return fT;
    }
}