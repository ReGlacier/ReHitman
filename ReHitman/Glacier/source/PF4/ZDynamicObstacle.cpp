#include <Glacier/PF4/ZDynamicObstacle.h>
#include <Glacier/ZUniAssert.h>

#include <cmath>
#include <cstring>
#include <limits>


namespace Glacier::PF4
{
    // 2D squared distance/helpers used by hull code (engine inlines them).
    namespace
    {
        float V2DistSq(float fAx, float fAy, float fBx, float fBy)
        {
            const float fDx = fAx - fBx;
            const float fDy = fAy - fBy;
            return fDx * fDx + fDy * fDy;
        }

        float V2Len(float fX, float fY)
        {
            return std::sqrt(fX * fX + fY * fY);
        }

        // Sign convention of the engine's LinePointSide(): the (signed) magnitude
        // of cross(P - A, B - A) for the directed line A -> B.
        float LinePointSideCross(const ZVector2& vA, const ZVector2& vB, float fPx, float fPy)
        {
            return (fPy - vA.y) * (vA.x - vB.x) + (fPx - vA.x) * (vB.y - vA.y);
        }
    }

    ZDynamicObstacle::ZDynamicObstacle()
    {
        m_HullSize = 0;
        m_Type = static_cast<EType>(0);
        m_Radius = 0.0f;
        m_Location.Reset();
        m_iLinks = 0;
    }

    ZDynamicObstacle::ZDynamicObstacle(const float (&vBox)[3], int iType)
    {
        m_Type = static_cast<EType>(iType);
        m_Location.Reset();
        m_iLinks = 0;

        const float vExtents[2] = { vBox[0], vBox[2] };
        CreateHullFromBox(vExtents);
    }

    ZDynamicObstacle& ZDynamicObstacle::operator=(const ZDynamicObstacle& rhs)
    {
        std::memcpy(this, &rhs, sizeof(*this));
        return *this;
    }

    void ZDynamicObstacle::CreateHullFromBox(const float (&vExtents)[2])
    {
        m_HullSize = 4;
        m_ConvexHull[0].x = vExtents[1];
        m_ConvexHull[0].y = vExtents[0];
        m_ConvexHull[1].x = -vExtents[1];
        m_ConvexHull[1].y = vExtents[0];
        m_ConvexHull[2].x = -vExtents[1];
        m_ConvexHull[2].y = -vExtents[0];
        m_ConvexHull[3].x = vExtents[1];
        m_ConvexHull[3].y = -vExtents[0];
        m_Radius = std::sqrt(vExtents[0] * vExtents[0] + vExtents[1] * vExtents[1]) * 1.5f;
    }

    void ZDynamicObstacle::DirXZ(const float (&vDir)[3])
    {
        m_Dir.x = vDir[0];
        m_Dir.y = vDir[2];
    }

    bool ZDynamicObstacle::Active() const
    {
        return m_iLinks != 0;
    }

    bool ZDynamicObstacle::IsInside(const float* pvPos) const
    {
        // Transform the world point into the obstacle's local frame.
        const float fDx = pvPos[0] - m_Location.m_vPos.x;
        const float fDz = pvPos[1] - m_Location.m_vPos.z;
        const float fX = fDx * m_Dir.x + fDz * m_Dir.y;
        const float fY = fDz * m_Dir.x - fDx * m_Dir.y;

        const int iHullSize = m_HullSize;
        for (int i = 0; i < iHullSize; ++i)
        {
            const int iNext = (i + 1 >= iHullSize) ? 0 : i + 1;
            const float fCross = (fY - m_ConvexHull[i].y) * (m_ConvexHull[i].x - m_ConvexHull[iNext].x)
                + (fX - m_ConvexHull[i].x) * (m_ConvexHull[iNext].y - m_ConvexHull[i].y);
            if (fCross > 0.0f)
            {
                return false;
            }
        }

        return true;
    }

    float ZDynamicObstacle::ClosestPoint(const float* pvFrom, float* pvFound) const
    {
        const int iHullSize = m_HullSize;
        if (!iHullSize)
        {
            return -1.0f;
        }

        // Transform into the obstacle's local frame.
        const float fDx = pvFrom[0] - m_Location.m_vPos.x;
        const float fDz = pvFrom[1] - m_Location.m_vPos.z;
        const float fX = fDx * m_Dir.x + fDz * m_Dir.y;
        const float fY = fDz * m_Dir.x - fDx * m_Dir.y;

        float fBest = 1000000.0f;
        float fFoundX = 0.0f;
        float fFoundY = 0.0f;
        for (int i = 0; i < iHullSize; ++i)
        {
            const int iNext = (i + 1 >= iHullSize) ? 0 : i + 1;
            const float fAx = m_ConvexHull[i].x;
            const float fAy = m_ConvexHull[i].y;

            // Closest hull vertex.
            const float fVertexDist = V2DistSq(fX, fY, fAx, fAy);
            if (fVertexDist < fBest)
            {
                fBest = fVertexDist;
                fFoundX = fAx;
                fFoundY = fAy;
            }

            // Closest point on the edge (project onto the inward/outward normal).
            const float fEx = m_ConvexHull[iNext].x - fAx;
            const float fEy = m_ConvexHull[iNext].y - fAy;
            const float fNx = -fEy;
            const float fNy = fEx;
            const float fTx = fX - fAx;
            const float fTy = fY - fAy;
            const float fAlong = fTx * fEx + fTy * fEy;
            const float fNorm2 = fNx * fNx + fNy * fNy;
            if (fAlong > 0.0f && fAlong < fNorm2)
            {
                const float fS = -(fTx * fNx + fTy * fNy) / fNorm2;
                const float fPx = fNx * fS;
                const float fPy = fNy * fS;
                const float fNormDist = fPx * fPx + fPy * fPy;
                if (fNormDist < fBest)
                {
                    fBest = fNormDist;
                    fFoundX = fX + fPx;
                    fFoundY = fY + fPy;
                }
            }
        }

        if (fBest < 1000000.0f)
        {
            pvFound[0] = fFoundX * m_Dir.x + m_Location.m_vPos.x - fFoundY * m_Dir.y;
            pvFound[1] = fFoundY * m_Dir.x + fFoundX * m_Dir.y + m_Location.m_vPos.z;
        }

        return fBest;
    }

    float ZDynamicObstacle::Intersect(const float* pvStart, const float* pvEnd, float fMaxDistance,
        float fBest, float (*pvTangentPoints)[2], int& iRightId, int& iLeftId)
    {
        iRightId = -1;
        iLeftId = -1;

        const float fStartToEndX = pvEnd[0] - pvStart[0];
        const float fStartToEndZ = pvEnd[1] - pvStart[1];
        if (fStartToEndX == 0.0f && fStartToEndZ == 0.0f)
        {
            return -1.0f;
        }

        // Ray direction (normalised) in the obstacle's local hull frame.
        const float fWorldDirLen = V2Len(fStartToEndX, fStartToEndZ);
        const float fWorldDirX = fStartToEndX / fWorldDirLen;
        const float fWorldDirZ = fStartToEndZ / fWorldDirLen;
        const float fOriginDx = pvStart[0] - m_Location.m_vPos.x;
        const float fOriginDz = pvStart[1] - m_Location.m_vPos.z;
        const float fOriginX = fOriginDx * m_Dir.x + fOriginDz * m_Dir.y;
        const float fOriginY = fOriginDz * m_Dir.x - fOriginDx * m_Dir.y;
        const float fDirX = fWorldDirX * m_Dir.x + fWorldDirZ * m_Dir.y;
        const float fDirY = fWorldDirZ * m_Dir.x - fWorldDirX * m_Dir.y;

        const int iHullSize = m_HullSize;

        // Find the first hull edge (i, i+1) whose two vertices lie on opposite
        // sides of the ray line (the ray enters the hull through this edge).
        int iHitEdge = iHullSize;
        for (int i = 0; i < iHullSize; ++i)
        {
            const int iNext = (i + 1 >= iHullSize) ? 0 : i + 1;
            const float fSide0 = (m_ConvexHull[i].x - fOriginX) * fDirY - (m_ConvexHull[i].y - fOriginY) * fDirX;
            const float fSide1 = (m_ConvexHull[iNext].x - fOriginX) * fDirY - (m_ConvexHull[iNext].y - fOriginY) * fDirX;
            if (fSide0 <= 0.0f && fSide1 > 0.0f)
            {
                iHitEdge = i;
                break;
            }
        }

        if (iHitEdge == iHullSize)
        {
            return fBest;
        }

        // Distance along the ray to the chosen hull edge line.
        const int iHitNext = (iHitEdge + 1 >= iHullSize) ? 0 : iHitEdge + 1;
        const float fEdgeX = m_ConvexHull[iHitNext].x - m_ConvexHull[iHitEdge].x;
        const float fEdgeY = m_ConvexHull[iHitNext].y - m_ConvexHull[iHitEdge].y;
        const float fOriginToEdgeX = fOriginX - m_ConvexHull[iHitEdge].x;
        const float fOriginToEdgeY = fOriginY - m_ConvexHull[iHitEdge].y;
        const float fCrossOriginEdge = fOriginToEdgeX * fEdgeY - fOriginToEdgeY * fEdgeX;
        const float fCrossDirEdge = fDirX * fEdgeY - fDirY * fEdgeX;
        const float fT = -fCrossOriginEdge / fCrossDirEdge;

        if (!(fT > 0.0f && fT < fBest && fT < fMaxDistance))
        {
            return fBest;
        }

        // Scan all hull vertices for the two silhouette corners (as seen from the
        // ray origin). Each vertex is classified into one of four angular
        // quadrants around the ray direction and ranked by the side/along ratio.
        const float fMaxFloat = std::numeric_limits<float>::max();
        float fMaxRatio = -fMaxFloat;
        float fMinRatio = fMaxFloat;
        int iMaxCode = 0;
        int iMinCode = 3;
        for (int j = 0; j < iHullSize; ++j)
        {
            const float fAlong = (m_ConvexHull[j].x - fOriginX) * fDirX + (m_ConvexHull[j].y - fOriginY) * fDirY;
            const float fSide = (m_ConvexHull[j].y - fOriginY) * fDirX - (m_ConvexHull[j].x - fOriginX) * fDirY;

            float fRatio;
            int iCode;
            if (fAlong <= 0.0f)
            {
                if (fSide <= 0.0f)
                {
                    iCode = 0;
                    fRatio = fAlong == 0.0f ? fMaxFloat : fSide / fAlong;
                }
                else
                {
                    iCode = 3;
                    fRatio = fAlong == 0.0f ? -fMaxFloat : fSide / fAlong;
                }
            }
            else
            {
                iCode = fSide <= 0.0f ? 1 : 2;
                fRatio = fSide / fAlong;
            }

            if ((fRatio > fMaxRatio && iCode == iMaxCode) || iCode > iMaxCode)
            {
                fMaxRatio = fRatio;
                iMaxCode = iCode;
                iLeftId = j;
            }
            if ((fRatio < fMinRatio && iCode == iMinCode) || iCode < iMinCode)
            {
                fMinRatio = fRatio;
                iMinCode = iCode;
                iRightId = j;
            }
        }

        ZASSERT(iRightId != -1 && iLeftId != -1 && iRightId != iLeftId);

        // World positions of the two silhouette corners (right first, then left).
        pvTangentPoints[0][0] = m_ConvexHull[iRightId].x * m_Dir.x
            - m_ConvexHull[iRightId].y * m_Dir.y + m_Location.m_vPos.x;
        pvTangentPoints[0][1] = m_ConvexHull[iRightId].x * m_Dir.y
            + m_ConvexHull[iRightId].y * m_Dir.x + m_Location.m_vPos.z;
        pvTangentPoints[1][0] = m_ConvexHull[iLeftId].x * m_Dir.x
            - m_ConvexHull[iLeftId].y * m_Dir.y + m_Location.m_vPos.x;
        pvTangentPoints[1][1] = m_ConvexHull[iLeftId].x * m_Dir.y
            + m_ConvexHull[iLeftId].y * m_Dir.x + m_Location.m_vPos.z;

        return fT;
    }

    void ZDynamicObstacle::FindShortestRouteToTarget(const float* pvFrom, const float* pvTarget,
        int iRightId, int iLeftId, float* pvRoutePoint)
    {
        const int iHullSize = m_HullSize;
        if (iHullSize <= 0)
        {
            return;
        }

        // Target position in the obstacle's local hull frame.
        const float fTargetDx = pvTarget[0] - m_Location.m_vPos.x;
        const float fTargetDz = pvTarget[1] - m_Location.m_vPos.z;
        const float fTargetX = fTargetDx * m_Dir.x + fTargetDz * m_Dir.y;
        const float fTargetY = fTargetDz * m_Dir.x - fTargetDx * m_Dir.y;
        const float fFromX = (pvFrom[0] - m_Location.m_vPos.x) * m_Dir.x
            + (pvFrom[1] - m_Location.m_vPos.z) * m_Dir.y;
        const float fFromY = (pvFrom[1] - m_Location.m_vPos.z) * m_Dir.x
            - (pvFrom[0] - m_Location.m_vPos.x) * m_Dir.y;

        // Route around the hull walking backwards (decreasing vertex indices)
        // from iRightId until the target is visible.
        float fLen = V2Len(m_ConvexHull[iRightId].x - fFromX, m_ConvexHull[iRightId].y - fFromY);
        int iCur = iRightId;
        float fLastEdge = 0.0f;
        int iPrev = iRightId;
        for (;;)
        {
            iPrev = iCur;
            iCur = iPrev - 1;
            if (iCur < 0)
            {
                iCur = iHullSize - 1;
            }

            const float fSide = LinePointSideCross(m_ConvexHull[iPrev], m_ConvexHull[iCur], fTargetX, fTargetY);
            fLastEdge = V2Len(m_ConvexHull[iPrev].x - m_ConvexHull[iCur].x,
                m_ConvexHull[iPrev].y - m_ConvexHull[iCur].y);
            fLen += fLastEdge;

            if (!(fSide >= 0.0f) || iCur == iRightId)
            {
                break;
            }
        }
        fLen = fLen - fLastEdge + V2Len(m_ConvexHull[iPrev].x - fTargetX, m_ConvexHull[iPrev].y - fTargetY);

        // Route around the hull walking forwards (increasing vertex indices)
        // from iLeftId until the target is visible.
        float fLenForward = V2Len(m_ConvexHull[iLeftId].x - fFromX, m_ConvexHull[iLeftId].y - fFromY);
        iCur = iLeftId;
        fLastEdge = 0.0f;
        iPrev = iLeftId;
        for (;;)
        {
            iPrev = iCur;
            iCur = iPrev + 1;
            if (iCur >= iHullSize)
            {
                iCur = 0;
            }

            const float fSide = LinePointSideCross(m_ConvexHull[iPrev], m_ConvexHull[iCur], fTargetX, fTargetY);
            fLastEdge = V2Len(m_ConvexHull[iPrev].x - m_ConvexHull[iCur].x,
                m_ConvexHull[iPrev].y - m_ConvexHull[iCur].y);
            fLenForward += fLastEdge;

            if (!(fSide <= 0.0f) || iCur == iLeftId)
            {
                break;
            }
        }
        fLenForward = fLenForward - fLastEdge
            + V2Len(m_ConvexHull[iPrev].x - fTargetX, m_ConvexHull[iPrev].y - fTargetY);

        // Start the route at the vertex whose detour is shorter.
        int iChosen = iRightId;
        if (fLenForward < fLen)
        {
            iChosen = iLeftId;
        }

        float fOutX = m_ConvexHull[iChosen].x * m_Dir.x - m_ConvexHull[iChosen].y * m_Dir.y + m_Location.m_vPos.x;
        float fOutZ = m_ConvexHull[iChosen].x * m_Dir.y + m_ConvexHull[iChosen].y * m_Dir.x + m_Location.m_vPos.z;

        // Move the way point 20 units beyond the hull vertex, away from the
        // obstacle centre, so the follow-up leg clears the corner (PC build).
        const float fDeltaX = fOutX - m_Location.m_vPos.x;
        const float fDeltaZ = fOutZ - m_Location.m_vPos.z;
        const float fDistSq = fDeltaX * fDeltaX + fDeltaZ * fDeltaZ;
        if (fDistSq > 0.0f)
        {
            const float fScale = 20.0f / std::sqrt(fDistSq);
            fOutX += fDeltaX * fScale;
            fOutZ += fDeltaZ * fScale;
        }

        pvRoutePoint[0] = fOutX;
        pvRoutePoint[1] = fOutZ;
    }
}
