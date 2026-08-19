#include <Glacier/Physics/ZCommonAlgorithms.h>


namespace Glacier
{
    namespace 
    {
        void ShortestDistancePointLineSegment(ZVector3& x, const ZVector3& p, const ZVector3& p0, const ZVector3& p1)
        {
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            float dz = p1.z - p0.z;

            float lenSq = dx * dx + dy * dy + dz * dz;

            float t = 0.0f;
            if (lenSq > 0.0f)
            {
                float dot = dx * (p.x - p0.x) + dy * (p.y - p0.y) + dz * (p.z - p0.z);
                t = dot / lenSq;

                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
            }

            x.x = p0.x + dx * t;
            x.y = p0.y + dy * t;
            x.z = p0.z + dz * t;
        }
    }

    int ZCommonAlgorithms::CapsuleSphereCollision(ZVector3& vDir, float& fLen, const ZVector3& cp0, const ZVector3& cp1, const float& cr, const ZVector3& sc, const float& sr)
    {
        ZVector3 closestPoint;
        ShortestDistancePointLineSegment(closestPoint, sc, cp0, cp1);

        float dx = sc.x - closestPoint.x;
        float dy = sc.y - closestPoint.y;
        float dz = sc.z - closestPoint.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        float combinedRadius = cr + sr;

        if (distSq >= combinedRadius * combinedRadius)
        {
            return 0;
        }

        float dist = sqrtf(distSq);
        if (dist > 0.0f)
        {
            float invDist = 1.0f / dist;
            vDir.x = dx * invDist;
            vDir.y = dy * invDist;
            vDir.z = dz * invDist;
        }
        else
        {
            vDir.x = 0.0f;
            vDir.y = 0.0f;
            vDir.z = 0.0f;
        }

        fLen = combinedRadius - dist;
        return 1;
    }

    bool ZCommonAlgorithms::PolySphColl(const float *SphPos, const float *SphMat, const float *inPos1, const float *inPos2, const float *inPos3)
    {
        // NOTE: the three transformed vertices must stay contiguous in memory:
        // CheckCutInside() walks them through the first pointer (3 x 3 floats),
        // exactly like the original PC build where v12/v13/v14 were adjacent
        // stack locals ([ebp-24h], [ebp-18h], [ebp-0Ch]).
        float vPos[9];
        float* const vPos1 = &vPos[0];
        float* const vPos2 = &vPos[3];
        float* const vPos3 = &vPos[6];

        // Transform the triangle into unit-sphere local space.
        vsub(vPos1, inPos1, SphPos);
        vsub(vPos2, inPos2, SphPos);
        vsub(vPos3, inPos3, SphPos);
        vmtmul(vPos1, SphMat);
        vmtmul(vPos2, SphMat);
        vmtmul(vPos3, SphMat);

        float vEdge1[3];
        float vEdge2[3];
        float vNormal[3];

        vsub(vEdge1, vPos2, vPos1);
        vsub(vEdge2, vPos3, vPos2);
        vcross(vNormal, vEdge1, vEdge2);

        // Reject (near-)degenerate triangles.
        if (vlen2(vNormal) < 0.00012207031f)
        {
            return false;
        }

        // Any edge of the triangle touching the unit sphere means a hit.
        if (LnSphColl(vPos1, vPos2) || LnSphColl(vPos2, vPos3) || LnSphColl(vPos3, vPos1))
        {
            return true;
        }

        // Otherwise the sphere may still overlap the triangle interior.
        vnorm(vNormal);
        const float fDist = vdot(vPos1, vNormal);

        if (std::fabs(fDist) > 1.0f)
        {
            return false;
        }

        // Closest point on the triangle plane to the sphere center (origin).
        vscalar(vNormal, fDist);

        return CheckCutInside(vPos1, vNormal, vNormal);
    }

    bool ZCommonAlgorithms::LnSphColl(float *Pos1, float *Pos2)
    {
        // Start point already inside the unit sphere.
        if (vlen2(Pos1) < 1.0f)
        {
            return true;
        }

        float vDir[3];
        vsub(vDir, Pos2, Pos1);
        const float fLen = vnorm(vDir);

        // Distance from Pos1 to the closest point of the line on the sphere center (origin).
        const float fProj = -vdot(vDir, Pos1);

        if (fProj > fLen)
        {
            return false;
        }

        if (fProj >= 0.0f)
        {
            float vClosest[3];
            vscalar(vClosest, vDir, fProj);
            vadd(vClosest, Pos1);

            if (vlen(vClosest) <= 1.0f)
            {
                return true;
            }
        }

        return false;
    }

    bool ZCommonAlgorithms::CheckCutInside(const float *vertexptr, const float *pn, const float *cp)
    {
        // Pick the projection plane by dropping the dominant normal axis:
        // (iU, iV) are the two remaining component indices.
        int iU = 1;
        int iV = 2;
        float fMax = std::fabs(pn[0]);
        const float fAbsY = std::fabs(pn[1]);

        if (fMax < fAbsY)
        {
            iU = 2;
            fMax = fAbsY;
            iV = 0;
        }

        if (fMax < std::fabs(pn[2]))
        {
            iU = 0;
            iV = 1;
        }

        // 2D half-space tests of cp against every triangle edge. All edge cross
        // products share the winding sign of the first one (fWinding).
        const float fEdge0U = vertexptr[iU] - vertexptr[iU + 3];
        const float fEdge0V = vertexptr[iV + 3] - vertexptr[iV];
        const float fWinding = (vertexptr[iU + 6] - vertexptr[iU + 3]) * fEdge0U
            + (vertexptr[iV + 6] - vertexptr[iV + 3]) * fEdge0V;
        const float fCpU = cp[iU] - vertexptr[iU + 3];
        const float fCpV = cp[iV] - vertexptr[iV + 3];

        if ((fCpV * fEdge0V + fCpU * fEdge0U) * fWinding >= -0.00012207031f
            && ((vertexptr[iU + 6] - vertexptr[iU + 3]) * fCpV + (vertexptr[iV + 3] - vertexptr[iV + 6]) * fCpU) * fWinding >= -0.00012207031f
            && ((vertexptr[iU] - vertexptr[iU + 6]) * (cp[iV] - vertexptr[iV])
                + (cp[iU] - vertexptr[iU]) * (vertexptr[iV + 6] - vertexptr[iV])) * fWinding >= -0.00012207031f)
        {
            return true;
        }

        return false;
    }
}