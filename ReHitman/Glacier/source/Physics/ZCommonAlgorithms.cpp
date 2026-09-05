#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/Physics/SFastBoxColiTri.h>
#include <Glacier/Physics/SCapsuleColiInfo.h>
#include <Glacier/System/ZSysInterface.h>


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

    void ZCommonAlgorithms::ProjectSphereOutFromPlane(float* vCenResult, const float* vSrcCenter, float* vDir, const float* vDesiredNormal, const float* vPushDir, float fPenetrationDepth)
    {
        /* vDir = vDesiredNormal */
        vDir[0] = vDesiredNormal[0];
        vDir[1] = vDesiredNormal[1];
        vDir[2] = vDesiredNormal[2];

        /* If push direction and desired normal oppose each other, no correction */
        const float fDot = (vPushDir[0] * vDesiredNormal[0]) +
                           (vPushDir[1] * vDesiredNormal[1]) +
                           (vPushDir[2] * vDesiredNormal[2]);

        if (fDot < 0.0f)
        {
            vDir[0] = 0.0f;
            vDir[1] = 0.0f;
            vDir[2] = 0.0f;
        }

        /* vCenResult = vSrcCenter + vPushDir * fPenetrationDepth */
        vCenResult[0] = vSrcCenter[0] + vPushDir[0] * fPenetrationDepth;
        vCenResult[1] = vSrcCenter[1] + vPushDir[1] * fPenetrationDepth;
        vCenResult[2] = vSrcCenter[2] + vPushDir[2] * fPenetrationDepth;
    }

    void ZCommonAlgorithms::AdjustPart2rigid(float* x1, float* x2, float m1, float m2, float dist)
    {
        // Load positions as SSE vectors (x, y, z, unused)
        __m128 v1 = _mm_loadu_ps(x1);
        __m128 v2 = _mm_loadu_ps(x2);
        __m128 delta = _mm_sub_ps(v2, v1);

        // Compute squared length
        __m128 deltaSq = _mm_mul_ps(delta, delta);
        float lenSqr = _mm_cvtss_f32(deltaSq)
                     + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(1, 1, 1, 1)))
                     + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(2, 2, 2, 2)));

        float len = sqrtf(lenSqr);

        // Avoid division by zero with random perturbation using g_pSysInterface->m_fTimeMultiplier_override as seed
        while (len < 0.000001f)
        {
            // Original uses g_pSysInterface->m_fTimeMultiplier_override as a pseudo-random seed
            uint32_t seed = *reinterpret_cast<uint32_t*>(&g_pSysInterface->m_fTimeMultiplier_override);
            seed = 69069 * seed + 1;
            g_pSysInterface->m_fTimeMultiplier_override = *reinterpret_cast<float*>(&seed);

            const float rx = (static_cast<float>((seed >> 8) & 0x7FFF) - 16384.0f) * 0.000061035156f;

            seed = 69069 * seed + 1;
            g_pSysInterface->m_fTimeMultiplier_override = *reinterpret_cast<float*>(&seed);
            const float ry = (static_cast<float>((seed >> 8) & 0x7FFF) - 16384.0f) * 0.000061035156f;

            seed = 69069 * seed + 1;
            g_pSysInterface->m_fTimeMultiplier_override = *reinterpret_cast<float*>(&seed);
            const float rz = (static_cast<float>((seed >> 8) & 0x7FFF) - 16384.0f) * 0.000061035156f;

            delta = _mm_set_ps(0.0f, rz, ry, rx);
            deltaSq = _mm_mul_ps(delta, delta);
            lenSqr = _mm_cvtss_f32(deltaSq)
                   + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(1, 1, 1, 1)))
                   + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(2, 2, 2, 2)));
            len = sqrtf(lenSqr);
        }

        const float invLen = 1.0f / len;
        const float invMassSum = 1.0f / (m1 + m2);
        const float correction = (len - dist) * m2 * invMassSum;

        // Normalize delta
        const __m128 invLenVec = _mm_set1_ps(invLen);
        const __m128 normal = _mm_mul_ps(delta, invLenVec);

        // Compute correction vector
        const __m128 correctionVec = _mm_mul_ps(normal, _mm_set1_ps(correction));

        // Update x1: x1 += correction
        v1 = _mm_add_ps(v1, correctionVec);
        _mm_storeu_ps(x1, v1);

        // Update x2: x2 = normal * dist + x1 (which is now updated)
        const __m128 distVec = _mm_mul_ps(normal, _mm_set1_ps(dist));
        v2 = _mm_add_ps(v1, distVec);
        _mm_storeu_ps(x2, v2);
    }

    bool ZCommonAlgorithms::Solve3x3System(const float* base2, const float* target2_global, float* lincomb)
    {
        // Compute determinant using Cramer's rule
        const float det =
            base2[0] * base2[4] * base2[8] +
            base2[1] * base2[5] * base2[6] +
            base2[2] * base2[3] * base2[7] -
            base2[2] * base2[4] * base2[6] -
            base2[1] * base2[3] * base2[8] -
            base2[0] * base2[5] * base2[7];

        if (fabsf(det) < 0.000001f)
            return false;

        const float invDet = 1.0f / det;

        // Compute solution using Cramer's rule (column-major, matches PC 0x581C80).
        lincomb[0] = (base2[3] * base2[7] * target2_global[2] +
                      base2[6] * target2_global[1] * base2[5] +
                      base2[8] * target2_global[0] * base2[4] -
                      base2[6] * base2[4] * target2_global[2] -
                      base2[8] * base2[3] * target2_global[1] -
                      target2_global[0] * base2[7] * base2[5]) * invDet;

        lincomb[1] = (base2[6] * base2[1] * target2_global[2] +
                      base2[8] * base2[0] * target2_global[1] +
                      base2[2] * target2_global[0] * base2[7] -
                      base2[6] * base2[2] * target2_global[1] -
                      base2[8] * base2[1] * target2_global[0] -
                      base2[0] * base2[7] * target2_global[2]) * invDet;

        lincomb[2] = (base2[0] * base2[4] * target2_global[2] +
                      base2[2] * base2[3] * target2_global[1] +
                      base2[1] * target2_global[0] * base2[5] -
                      base2[2] * target2_global[0] * base2[4] -
                      base2[1] * base2[3] * target2_global[2] -
                      base2[0] * target2_global[1] * base2[5]) * invDet;

        return true;
    }

    bool ZCommonAlgorithms::DistPointLineVar2(const float* a1, const float* b1, const float* b2,
                                              float* t, float* fMinDist, float* fDist, float* vDir)
    {
        // Compute line direction
        const float dx = b2[0] - b1[0];
        const float dy = b2[1] - b1[1];
        const float dz = b2[2] - b1[2];

        const float lineLenSqr = dx * dx + dy * dy + dz * dz;

        // Compute vector from b1 to a1
        const float ax = a1[0] - b1[0];
        const float ay = a1[1] - b1[1];
        const float az = a1[2] - b1[2];

        // Project a1 onto line
        float dot = ax * dx + ay * dy + az * dz;
        *t = dot;

        if (dot >= 0.0f)
        {
            if (dot <= lineLenSqr)
            {
                // Closest point is on segment
                *t = dot / lineLenSqr;
                const float closestX = b1[0] + dx * (*t);
                const float closestY = b1[1] + dy * (*t);
                const float closestZ = b1[2] + dz * (*t);

                vDir[0] = a1[0] - closestX;
                vDir[1] = a1[1] - closestY;
                vDir[2] = a1[2] - closestZ;
            }
            else
            {
                // Closest point is b2
                *t = 1.0f;
                vDir[0] = a1[0] - b2[0];
                vDir[1] = a1[1] - b2[1];
                vDir[2] = a1[2] - b2[2];
            }
        }
        else
        {
            // Closest point is b1
            *t = 0.0f;
            vDir[0] = ax;
            vDir[1] = ay;
            vDir[2] = az;
        }

        const float distSqr = vDir[0] * vDir[0] + vDir[1] * vDir[1] + vDir[2] * vDir[2];

        if (distSqr > (*fMinDist) * (*fMinDist))
            return false;

        const float dist = sqrtf(distSqr);
        *fDist = dist;

        if (dist <= 0.000001f)
        {
            RandomUnitVector(vDir);
        }
        else
        {
            const float invDist = 1.0f / dist;
            vDir[0] *= invDist;
            vDir[1] *= invDist;
            vDir[2] *= invDist;
        }

        return true;
    }

    void ZCommonAlgorithms::RandomUnitVector(float* v)
    {
        float fLen = 0.0f;

        do
        {
            const float rx = g_pSysInterface->FRand(nullptr, 0);
            const float ry = g_pSysInterface->FRand(nullptr, 0);
            const float rz = g_pSysInterface->FRand(nullptr, 0);

            v[0] = rx;
            v[1] = ry;
            v[2] = rz;

            fLen = sqrtf(rx * rx + ry * ry + rz * rz);
        } while (fLen < 0.000001f);

        const float fInvLen = 1.0f / fLen;
        vscalar(v, fInvLen);
    }

    bool ZCommonAlgorithms::IntersectTriangleAndLine3(float* coli_x, const float* orig, const float* dir,
                                                      const float* vert0, const float* inv, float* t, bool bBothSides)
    {
        // Compute denominator
        const float denom = dir[0] * inv[2] + dir[1] * inv[5] + dir[2] * inv[8];

        if (!bBothSides && denom > 0.0f)
            return false;

        if (fabsf(denom) < 0.000001f)
            return false;

        // Compute vector from vert0 to orig
        const float ox = orig[0] - vert0[0];
        const float oy = orig[1] - vert0[1];
        const float oz = orig[2] - vert0[2];

        // Compute t
        const float tVal = -(ox * inv[2] + oy * inv[5] + oz * inv[8]) / denom;
        *t = tVal;

        if (tVal <= 0.0f || tVal >= 1.0f)
            return false;

        // Compute intersection point
        const float px = ox + dir[0] * tVal;
        const float py = oy + dir[1] * tVal;
        const float pz = oz + dir[2] * tVal;

        // Compute barycentric coordinates
        const float u = px * inv[0] + py * inv[3] + pz * inv[6];
        const float v = px * inv[1] + py * inv[4] + pz * inv[7];

        if (u < 0.0f || u > 1.0f || v < 0.0f || (u + v) > 1.0f)
            return false;

        // Compute collision point
        coli_x[0] = vert0[0] + px;
        coli_x[1] = vert0[1] + py;
        coli_x[2] = vert0[2] + pz;

        return true;
    }

    bool ZCommonAlgorithms::IntersectTriangleAndSphere(const float (*triverts)[3], const float (*spherepos)[3],
                                                       float fRadius, float (*vDir0)[3], float* fScaledDist, float* a6)
    {
        // Compute triangle edges
        const float edge1[3] = {
            triverts[1][0] - triverts[0][0],
            triverts[1][1] - triverts[0][1],
            triverts[1][2] - triverts[0][2]
        };
        const float edge2[3] = {
            triverts[2][0] - triverts[0][0],
            triverts[2][1] - triverts[0][1],
            triverts[2][2] - triverts[0][2]
        };

        // Compute triangle normal (cross product)
        float normal[3] = {
            edge1[1] * edge2[2] - edge1[2] * edge2[1],
            edge1[2] * edge2[0] - edge1[0] * edge2[2],
            edge1[0] * edge2[1] - edge1[1] * edge2[0]
        };

        const float normalLen = sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
        if (normalLen < 0.000001f)
            return false;

        const float invNormalLen = 1.0f / normalLen;
        normal[0] *= invNormalLen;
        normal[1] *= invNormalLen;
        normal[2] *= invNormalLen;

        // Build 3x3 system matrix (columns: edge1, edge2, normal)
        const float matrix[9] = {
            edge1[0], edge1[1], edge1[2],
            edge2[0], edge2[1], edge2[2],
            normal[0], normal[1], normal[2]
        };

        // Vector from triangle vertex 0 to sphere center
        const float target[3] = {
            (*spherepos)[0] - triverts[0][0],
            (*spherepos)[1] - triverts[0][1],
            (*spherepos)[2] - triverts[0][2]
        };

        float solution[3];
        bool hit = false;

        if (Solve3x3System(matrix, target, solution))
        {
            const float u = solution[0];
            const float v = solution[1];
            const float w = solution[2];

            if (u >= 0.0f && u < 1.0f && v >= 0.0f && (u + v) >= 0.0f && (u + v) < 1.0f)
            {
                if (w >= 0.0f && w <= fRadius)
                {
                    // Sphere is above triangle
                    hit = true;
                    *a6 = fRadius - w;
                    fScaledDist[0] = -normal[0];
                    fScaledDist[1] = -normal[1];
                    fScaledDist[2] = -normal[2];
                }
                else if (w < 0.0f && w >= -fRadius)
                {
                    // Sphere is below triangle
                    hit = true;
                    *a6 = w + fRadius;
                    fScaledDist[0] = normal[0];
                    fScaledDist[1] = normal[1];
                    fScaledDist[2] = normal[2];
                }
            }
        }

        if (hit)
            return true;

        // Check edges
        float minDist = fRadius;
        float edgeDist;
        float edgeT;
        float edgeDir[3];

        // Edge 0-1
        if (DistPointLineVar2(&(*spherepos)[0], &triverts[0][0], &triverts[1][0], &edgeT, &minDist, &edgeDist, edgeDir))
        {
            hit = true;
            *a6 = fRadius - edgeDist;
            fScaledDist[0] = -edgeDir[0];
            fScaledDist[1] = -edgeDir[1];
            fScaledDist[2] = -edgeDir[2];
            minDist = edgeDist;
        }

        // Edge 0-2
        if (DistPointLineVar2(&(*spherepos)[0], &triverts[0][0], &triverts[2][0], &edgeT, &minDist, &edgeDist, edgeDir))
        {
            hit = true;
            *a6 = fRadius - edgeDist;
            fScaledDist[0] = -edgeDir[0];
            fScaledDist[1] = -edgeDir[1];
            fScaledDist[2] = -edgeDir[2];
            minDist = edgeDist;
        }

        // Edge 1-2
        if (DistPointLineVar2(&(*spherepos)[0], &triverts[1][0], &triverts[2][0], &edgeT, &minDist, &edgeDist, edgeDir))
        {
            hit = true;
            *a6 = fRadius - edgeDist;
            fScaledDist[0] = -edgeDir[0];
            fScaledDist[1] = -edgeDir[1];
            fScaledDist[2] = -edgeDir[2];
        }

        return hit;
    }

    bool ZCommonAlgorithms::DistPointLineVar(const float* const b1, const float* const a1, const float* a2,
                                             float& s, float& fMinDist, float& fDist, float* vDir)
    {
        float vToPoint[3];
        float vLine[3];
        vsub(vToPoint, b1, a1);
        vsub(vLine, a2, a1);

        const float fLineLenSqr = vlen2(vLine);
        const float fDot = vdot(vToPoint, vLine);

        s = fDot;
        if (fDot < 0.0f)
            return false;
        if (fDot > fLineLenSqr)
            return false;

        s = fDot / fLineLenSqr;

        float vClosest[3];
        vaddscalar(vClosest, a1, vLine, s);
        vsub(vDir, vClosest, b1);

        const float fDistSqr = vlen2(vDir);
        if (fDistSqr > fMinDist * fMinDist)
            return false;

        fDist = sqrtf(fDistSqr);
        if (fDist <= 0.000001f)
            RandomUnitVector(vDir);
        else
            vscalar(vDir, 1.0f / fDist);

        return true;
    }

    bool ZCommonAlgorithms::DistPointLineVar2(const float* const a1, const float* const b1, const float* const b2,
                                              float& t, float& fMinDist, float& fDist, float* vDir)
    {
        float vToPoint[3];
        float vLine[3];
        vsub(vToPoint, a1, b1);
        vsub(vLine, b2, b1);

        const float fLineLenSqr = vlen2(vLine);
        const float fDot = vdot(vToPoint, vLine);

        t = fDot;

        float vClosest[3];
        if (fDot >= 0.0f)
        {
            if (fDot <= fLineLenSqr)
            {
                t = fDot / fLineLenSqr;
                vaddscalar(vClosest, b1, vLine, t);
            }
            else
            {
                t = 1.0f;
                vcpy(vClosest, b2);
            }
        }
        else
        {
            t = 0.0f;
            vcpy(vClosest, b1);
        }

        vsub(vDir, a1, vClosest);

        const float fDistSqr = vlen2(vDir);
        if (fDistSqr > fMinDist * fMinDist)
            return false;

        fDist = sqrtf(fDistSqr);
        if (fDist <= 0.000001f)
            RandomUnitVector(vDir);
        else
            vscalar(vDir, 1.0f / fDist);

        return true;
    }

    bool ZCommonAlgorithms::DistLineLineVar(const float* const a1, const float* const a2,
                                            const float* const b1, const float* const b2,
                                            float& s, float& t, float& fMinDist, float& fDist, float* vDir)
    {
        const float fMinDistSqr = fMinDist * fMinDist;

        float vD0[3];
        float vDirA[3];
        float vB1B2[3];
        vsub(vD0, a1, b1);
        vsub(vDirA, a2, a1);
        vsub(vB1B2, b1, b2);

        const float fA0 = vdot(vDirA, vD0);
        const float fAB = vdot(vB1B2, vDirA);
        const float fLenA = vlen2(vDirA);

        const float fDenom = vlen2(vB1B2) * fLenA - fAB * fAB;
        if (std::fabs(fDenom) < 0.000001f)
            return false;

        t = (fAB * fA0 - vdot(vB1B2, vD0) * fLenA) / fDenom;

        if (std::fabs(fLenA) < 0.000001f)
            return false;

        s = -((t * fAB + fA0) / fLenA);

        if (s < 0.0f || s > 1.0f)
            return false;

        if (t < 0.0f)
        {
            t = 0.0f;
            return DistPointLineVar(b1, a1, a2, s, fMinDist, fDist, vDir);
        }

        if (t > 1.0f)
        {
            t = 1.0f;
            return DistPointLineVar(b2, a1, a2, s, fMinDist, fDist, vDir);
        }

        float vClosestA[3];
        float vClosestB[3];
        vaddscalar(vClosestA, a1, vDirA, s);
        vaddscalar(vClosestB, b1, vB1B2, -t);

        vsub(vDir, vClosestA, vClosestB);

        const float fDistSqr = vlen2(vDir);
        if (fDistSqr > fMinDistSqr)
            return false;

        fDist = sqrtf(fDistSqr);
        if (fDist <= 0.000001f)
            RandomUnitVector(vDir);
        else
            vscalar(vDir, 1.0f / fDist);

        return true;
    }

    bool ZCommonAlgorithms::CollideCapsuleAndTriangle(const float (&vCap0)[3], const float (&vCap1)[3],
                                                      float fRadius, const SFastBoxColiTri* pTriangle,
                                                      SCapsuleColiInfo& rResult)
    {
        const ZVector3* pVerts = pTriangle->m_avVerts;
        const ZVector3& vTriNorm = pTriangle->m_vTriNorm;

        // Face-plane test: signed distances of the capsule endpoints to the triangle plane.
        const float fPlaneDist = vdot(pVerts[0].Get(), vTriNorm.Get());
        const float fDist0 = vdot(vCap0, vTriNorm.Get()) - fPlaneDist;
        const float fDist1 = vdot(vCap1, vTriNorm.Get()) - fPlaneDist;

        if (fDist0 - fRadius > 0.0f && fDist1 - fRadius > 0.0f)
            return false;
        if (fDist0 + fRadius < 0.0f && fDist1 + fRadius < 0.0f)
            return false;

        // Clip the capsule segment against the plane slab [-fRadius, fRadius].
        float tMin = 0.0f;
        float tMax = 1.0f;
        const float fDelta = fDist1 - fDist0;

        const bool bA0 = (fDist0 - fRadius) > 0.0f;
        const bool bA1 = (fDist1 - fRadius) > 0.0f;
        if (bA0 != bA1 && std::fabs(fDelta) > 0.0001f)
        {
            const float t = -(fDist0 - fRadius) / fDelta;
            if (fDist0 - fRadius <= 0.0f)
            {
                if (t <= 1.0f)
                    tMax = t;
            }
            else if (t >= 0.0f)
            {
                tMin = t;
            }
        }

        const bool bB0 = (fDist0 + fRadius) < 0.0f;
        const bool bB1 = (fDist1 + fRadius) < 0.0f;
        if (bB0 != bB1 && std::fabs(fDelta) > 0.0001f)
        {
            const float t = -(fDist0 + fRadius) / fDelta;
            if (bB0)
            {
                if (tMin <= t)
                    tMin = t;
            }
            else if (tMax >= t)
            {
                tMax = t;
            }
        }

        // Clip against the three edge perpendicular planes.
        auto clipEdge = [&](const ZVector3& vPerp, const ZVector3& vRef) -> bool
        {
            const float fRef = vdot(vRef.Get(), vPerp.Get());
            const float fE0 = vdot(vCap0, vPerp.Get()) - fRef;
            const float fE1 = vdot(vCap1, vPerp.Get()) - fRef;

            const float fE0r = fE0 - fRadius;
            const float fE1r = fE1 - fRadius;

            if (fE0r > 0.0f && fE1r > 0.0f)
                return false;

            if ((fE0r > 0.0f) != (fE1r > 0.0f))
            {
                const float fEdgeDelta = fE1 - fE0;
                if (std::fabs(fEdgeDelta) > 0.0001f)
                {
                    const float t = -(fE0r / fEdgeDelta);
                    if (fE0r <= 0.0f)
                    {
                        if (tMax >= t)
                            tMax = t;
                    }
                    else if (tMin <= t)
                    {
                        tMin = t;
                    }
                }
            }
            return true;
        };

        if (!clipEdge(pTriangle->m_avEdgePerps[1], pTriangle->m_avVerts[0]))
            return false;
        if (tMin > tMax)
            return false;

        if (!clipEdge(pTriangle->m_avEdgePerps[2], pTriangle->m_avVerts[0]))
            return false;
        if (tMin > tMax)
            return false;

        if (!clipEdge(pTriangle->m_avEdgePerps[0], pTriangle->m_avVerts[1]))
            return false;
        if (tMin > tMax)
            return false;

        // Basis vectors: capsule direction and the two triangle edges.
        float vCapDir[3];
        float vEdge1[3];
        float vEdge2[3];
        vsub(vCapDir, vCap1, vCap0);
        vsub(vEdge1, pVerts[1].Get(), pVerts[0].Get());
        vsub(vEdge2, pVerts[2].Get(), pVerts[0].Get());

        // 3x3 system whose columns are the capsule direction and the two edges.
        float mSystem[9] = {
            vCapDir[0], vCapDir[1], vCapDir[2],
            vEdge1[0], vEdge1[1], vEdge1[2],
            vEdge2[0], vEdge2[1], vEdge2[2]
        };

        float vRel0[3];
        vsub(vRel0, vCap0, pVerts[0].Get());

        float vSolution[3];

        // Interior test: capsule segment crossing the triangle interior.
        if (Solve3x3System(mSystem, vRel0, vSolution))
        {
            const float t = -vSolution[0];
            if (vSolution[1] >= 0.0f && vSolution[1] < 1.0f && vSolution[2] >= 0.0f)
            {
                const float fUV = vSolution[2] + vSolution[1];
                if (fUV >= 0.0f && fUV < 1.0f && t >= 0.0f && t <= 1.0f)
                {
                    ZVector3 vNormal;
                    vcross(vNormal.Get(), vEdge2, vEdge1);
                    vnorm(vNormal.Get());

                    if (vdot(vNormal.Get(), vRel0) > 0.0f)
                        vscalar(vNormal.Get(), -1.0f);

                    float vRel1[3];
                    vsub(vRel1, vCap1, pVerts[0].Get());

                    rResult.t0 = 1.0f;
                    rResult.vDir = vNormal;
                    rResult.fScaledDist = vdot(vNormal.Get(), vRel1) + fRadius;
                    return true;
                }
            }
        }

        // Face / vertex region tests using the triangle normal.
        ZVector3 vNormal;
        vcross(vNormal.Get(), vEdge2, vEdge1);
        vnorm(vNormal.Get());

        float mNormalSystem[9] = {
            vNormal.x, vNormal.y, vNormal.z,
            vEdge1[0], vEdge1[1], vEdge1[2],
            vEdge2[0], vEdge2[1], vEdge2[2]
        };

        float fBestDist = fRadius;
        bool bHaveResult = false;

        vsub(vRel0, vCap0, pVerts[0].Get());
        if (Solve3x3System(mNormalSystem, vRel0, vSolution))
        {
            const float fD = vSolution[0];
            if (vSolution[1] >= 0.0f && vSolution[1] < 1.0f && vSolution[2] >= 0.0f)
            {
                const float fUV = vSolution[2] + vSolution[1];
                if (fUV >= 0.0f && fUV < 1.0f)
                {
                    if (fD < 0.0f || fD > fRadius)
                    {
                        if (fD < 0.0f && -fRadius <= fD)
                        {
                            rResult.t0 = 0.0f;
                            rResult.fScaledDist = fD + fRadius;
                            rResult.vDir = vNormal;
                            fBestDist = -fD;
                            bHaveResult = true;
                        }
                    }
                    else
                    {
                        rResult.t0 = 0.0f;
                        vscalar(rResult.vDir.Get(), vNormal.Get(), -1.0f);
                        fBestDist = fD;
                        rResult.fScaledDist = fRadius - fD;
                        bHaveResult = true;
                    }
                }
            }
        }

        float vRel1[3];
        vsub(vRel1, vCap1, pVerts[0].Get());
        if (Solve3x3System(mNormalSystem, vRel1, vSolution))
        {
            const float fD = vSolution[0];
            if (vSolution[1] >= 0.0f && vSolution[1] < 1.0f && vSolution[2] >= 0.0f)
            {
                const float fUV = vSolution[2] + vSolution[1];
                if (fUV >= 0.0f && fUV < 1.0f && std::fabs(fD) <= fRadius)
                {
                    if (fD >= 0.0f && fD <= fBestDist)
                    {
                        rResult.t0 = 1.0f;
                        vscalar(rResult.vDir.Get(), vNormal.Get(), -1.0f);
                        rResult.fScaledDist = fRadius - fD;
                        return true;
                    }
                    if (fD < 0.0f && -fBestDist <= fD)
                    {
                        rResult.fScaledDist = fD + fRadius;
                        rResult.t0 = 1.0f;
                        rResult.vDir = vNormal;
                        return true;
                    }
                }
            }
        }

        if (bHaveResult)
            return true;

        // Edge tests: capsule segment against each triangle edge.
        float fMinDist = fRadius;
        float s, t, fDist;
        float vDir[3];
        bool bEdgeHit = false;

        if (DistLineLineVar(pVerts[1].Get(), pVerts[2].Get(), vCap0, vCap1, s, t, fMinDist, fDist, vDir))
        {
            fMinDist = fDist;
            rResult.fScaledDist = fRadius - fDist;
            rResult.t0 = t;
            rResult.vDir = vDir;
            bEdgeHit = true;
        }

        if (DistLineLineVar(pVerts[0].Get(), pVerts[1].Get(), vCap0, vCap1, s, t, fMinDist, fDist, vDir))
        {
            fMinDist = fDist;
            rResult.fScaledDist = fRadius - fDist;
            rResult.t0 = t;
            rResult.vDir = vDir;
            bEdgeHit = true;
        }

        if (DistLineLineVar(pVerts[2].Get(), pVerts[0].Get(), vCap0, vCap1, s, t, fMinDist, fDist, vDir))
        {
            rResult.t0 = t;
            rResult.fScaledDist = fRadius - fDist;
            rResult.vDir = vDir;
            return true;
        }

        if (bEdgeHit)
            return true;

        // Vertex tests: capsule segment against each triangle vertex.
        bool bVertexHit = false;
        for (int i = 0; i < 3; ++i)
        {
            if (DistPointLineVar2(pVerts[i].Get(), vCap0, vCap1, t, fMinDist, fDist, vDir))
            {
                fMinDist = fDist;
                rResult.fScaledDist = fRadius - fDist;
                rResult.t0 = t;
                rResult.vDir = vDir;
                bVertexHit = true;
            }
        }

        return bVertexHit;
    }

    void ZCommonAlgorithms::PullTriangleCyl2(float* vCap0, float* vCap1, float* vVel0, float* vVel1, SCapsuleColiInfo& sColiInfo)
    {
        const float fW0 = 1.0f - sColiInfo.t0;
        const float fW1 = sColiInfo.t0;

        const float fDenom = fW0 * fW0 + fW1 * fW1;
        if (fabsf(fDenom) < 0.001f)
            return;

        const float fInvDenom = 1.0f / fDenom;

        // Pull the capsule endpoints out of the triangle along the contact normal,
        // weighted so the contact point stays fixed.
        const float fPush = sColiInfo.fScaledDist * fInvDenom;
        vaddscalar(vCap0, vCap0, sColiInfo.vDir.Get(), -fW0 * fPush);
        vaddscalar(vCap1, vCap1, sColiInfo.vDir.Get(), -fW1 * fPush);

        // Interpolated velocity at the contact point.
        ZVector3 vContactVel;
        vscalar(vContactVel, vVel0, fW0);
        vaddscalar(vContactVel, vContactVel, vVel1, fW1);

        vnorm(sColiInfo.vDir.Get());

        const float fDot = vdot(sColiInfo.vDir.Get(), vContactVel.Get());
        if (fDot <= 0.0f)
            return;

        // Remove the outgoing normal component of the contact velocity.
        vaddscalar(vVel0, vVel0, sColiInfo.vDir.Get(), -fW0 * fDot * fInvDenom);
        vaddscalar(vVel1, vVel1, sColiInfo.vDir.Get(), -fW1 * fDot * fInvDenom);

        // Damp the tangential component of the contact velocity.
        ZVector3 vTangentVel;
        vaddscalar(vTangentVel, vContactVel, sColiInfo.vDir.Get(), -fDot);
        const float fTanLen = vnorm(vTangentVel.Get());

        const float fTanPush = (fTanLen <= 1.0f || fTanLen <= 2.0f * fDot) ? fTanLen : 2.0f * fDot;

        vaddscalar(vVel0, vVel0, vTangentVel.Get(), -fW0 * fTanPush * fInvDenom);
        vaddscalar(vVel1, vVel1, vTangentVel.Get(), -fW1 * fTanPush * fInvDenom);
    }

    float ZCommonAlgorithms::DistanceBoxAndPoint(const float* p0, const float* s0)
    {
        const float fDx = fabsf(p0[0]) - s0[0];
        const float fDy = fabsf(p0[1]) - s0[1];
        const float fDz = fabsf(p0[2]) - s0[2];

        if (fDx >= 0.0f)
        {
            if (fDy >= 0.0f)
            {
                if (fDz >= 0.0f)
                {
                    return sqrtf(fDx * fDx + fDy * fDy + fDz * fDz);
                }
                return sqrtf(fDx * fDx + fDy * fDy);
            }

            if (fDz >= 0.0f)
            {
                return sqrtf(fDx * fDx + fDz * fDz);
            }

            return fDx;
        }

        if (fDz >= 0.0f)
        {
            if (fDy >= 0.0f)
            {
                return sqrtf(fDy * fDy + fDz * fDz);
            }
            return fDz;
        }

        return fDy;
    }
}
