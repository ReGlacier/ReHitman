#include <Glacier/Fysix/ConstrainedParticleSystem.h>
#include <Glacier/Fysix/ZCommonAlgorithms.h>
#include <Glacier/Fysix/ZFastBoxColi.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <xmmintrin.h>


namespace Glacier
{
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

        // Compute solution using Cramer's rule
        lincomb[0] = (target2_global[0] * base2[4] * base2[8] +
                      base2[1] * base2[5] * target2_global[2] +
                      base2[2] * target2_global[1] * base2[7] -
                      base2[2] * base2[4] * target2_global[2] -
                      base2[1] * target2_global[1] * base2[8] -
                      target2_global[0] * base2[5] * base2[7]) * invDet;

        lincomb[1] = (base2[0] * target2_global[1] * base2[8] +
                      target2_global[0] * base2[5] * base2[6] +
                      base2[2] * base2[3] * target2_global[2] -
                      base2[2] * target2_global[1] * base2[6] -
                      target2_global[0] * base2[3] * base2[8] -
                      base2[0] * base2[5] * target2_global[2]) * invDet;

        lincomb[2] = (base2[0] * base2[4] * target2_global[2] +
                      base2[1] * target2_global[1] * base2[6] +
                      target2_global[0] * base2[3] * base2[7] -
                      target2_global[0] * base2[4] * base2[6] -
                      base2[1] * base2[3] * target2_global[2] -
                      base2[0] * target2_global[1] * base2[7]) * invDet;

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
        float len;
        do
        {
            uint32_t seed = *reinterpret_cast<uint32_t*>(&g_pSysInterface->m_fTimeMultiplier_override);

            seed = 69069 * seed + 1;
            g_pSysInterface->m_fTimeMultiplier_override = *reinterpret_cast<float*>(&seed);
            const float rx = static_cast<float>((seed >> 8) & 0x7FFF) * 0.000030517578f - 0.5f;

            seed = 69069 * seed + 1;
            g_pSysInterface->m_fTimeMultiplier_override = *reinterpret_cast<float*>(&seed);
            const float ry = static_cast<float>((seed >> 8) & 0x7FFF) * 0.000030517578f - 0.5f;

            seed = 69069 * seed + 1;
            g_pSysInterface->m_fTimeMultiplier_override = *reinterpret_cast<float*>(&seed);
            const float rz = static_cast<float>((seed >> 8) & 0x7FFF) * 0.000030517578f - 0.5f;

            v[0] = rx;
            v[1] = ry;
            v[2] = rz;

            len = sqrtf(rx * rx + ry * ry + rz * rz);
        } while (len < 0.000001f);

        const float invLen = 1.0f / len;
        v[0] *= invLen;
        v[1] *= invLen;
        v[2] *= invLen;
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
}


namespace Glacier
{
    static ZDebugInt g_lGravity("gravity", "Gravity force in particle dynamics", -1000000, 1000000, 1, nullptr);

    ConstrainedParticleSystem::ConstrainedParticleSystem(int iType, int nMaxNumPartices)
    {
        m_bReallyInWater = false;
        m_fPrevTimeStep = -1.0f;
        m_bReallyInWaterOld = false;
        m_fDamping = 0.0099999998f;
        m_bInWater = false;
        m_pFastBox = nullptr;
        m_pParticles = nullptr;
        m_pConstraints = nullptr;
        m_pSpecialConstraints = nullptr;
        m_iNumSpecialConstraints = 0;
        m_iNumConstraints = 0;
        m_iNumParticles = 0;

        if (nMaxNumPartices > 0)
        {
            m_iNumParticles = nMaxNumPartices;
            m_pParticles = ZUniMemory::NewArray<Particle>(nMaxNumPartices);

            m_iNumConstraints = 4 * m_iNumParticles;
            m_pConstraints = ZUniMemory::NewArray<ParticleConstraint>(m_iNumConstraints);

            m_iNumSpecialConstraints = 150;
            m_pSpecialConstraints = ZUniMemory::NewArray<ParticleConstraint>(m_iNumSpecialConstraints);

            if (iType == 3)
                m_bFollow = false;

            InitOkX();

            m_pLnkObj = nullptr;
        }
        else
        {
            ZASSERT(false);
        }
    }

    ConstrainedParticleSystem::~ConstrainedParticleSystem()
    {
        ZUniMemory::DeleteArray(m_pParticles, m_iNumParticles);
        ZUniMemory::DeleteArray(m_pConstraints, m_iNumConstraints);
        ZUniMemory::DeleteArray(m_pSpecialConstraints, m_iNumSpecialConstraints);

        if (m_pFastBox)
        {
            ZUniMemory::Delete(m_pFastBox);
        }

        m_pParticles = nullptr;
        m_pConstraints = nullptr;
        m_pSpecialConstraints = nullptr;
        m_pLnkObj = nullptr;
    }

    void ConstrainedParticleSystem::InitOkX()
    {
        for (int i = 0; i < m_iNumParticles; ++i)
        {
            m_pParticles[i].ok_x = m_pParticles[i].x;
        }
    }

    void ConstrainedParticleSystem::SetNumConstraints(int n)
    {
        ZASSERT(n <= m_iNumConstraints);

        m_iNumConstraints = n;
    }

    void ConstrainedParticleSystem::SetParticleOldPos(int i, ZVector3& oldpos)
    {
        m_pParticles[i].oldx = oldpos;
    }

    void ConstrainedParticleSystem::GetParticlePos(int i, ZVector3& pos)
    {
        pos = m_pParticles[i].x;
    }

    void ConstrainedParticleSystem::GetParticleValues(int i, ZVector3& pos, ZVector3& v, float& mass)
    {
        Particle& par = m_pParticles[i];
        pos = par.x;
        v = par.v;
        mass = par.mass;
    }

    void ConstrainedParticleSystem::GetParticleValues(int i, ZVector3& pos, ZVector3& v, float& mass, ZVector3& oldpos)
    {
        Particle& par = m_pParticles[i];
        pos = par.x;
        v = par.v;
        mass = par.mass;
        oldpos = par.oldx;
    }

    void ConstrainedParticleSystem::SetParticleValues(int i, const ZVector3& pos, const ZVector3& v, float mass, const ZVector3& oldpos)
    {
        Particle& par = m_pParticles[i];
        par.x = pos;
        par.v = v;
        par.mass = mass;
        par.oldx = oldpos;
    }

    void ConstrainedParticleSystem::SetParticleVel(int i, const ZVector3& vel)
    {
        m_pParticles[i].v = vel;
    }

    void ConstrainedParticleSystem::InitParticle(int i, const ZVector3& x, const ZVector3& v, float mass)
    {
        if (i >= m_iNumParticles)
            return;

        m_pParticles[i].x = x;
        m_pParticles[i].v = v;
        m_pParticles[i].mass = mass;
    }

    void ConstrainedParticleSystem::InitConstraint(int i, int ix1, int ix2)
    {
        if (i < 0 || i >= m_iNumConstraints)
            return;

        Particle* const par1 = &m_pParticles[ix1];
        Particle* const par2 = &m_pParticles[ix2];
        ParticleConstraint* const con = &m_pConstraints[i];

        con->m_pPar1 = par1;
        con->m_pPar2 = par2;

        const float dx = par1->x.x - par2->x.x;
        const float dy = par1->x.y - par2->x.y;
        const float dz = par1->x.z - par2->x.z;

        con->m_fDist = sqrtf(dx * dx + dy * dy + dz * dz);
    }

    void ConstrainedParticleSystem::BlowBomb(const ZVector3& pos, float fForce0)
    {
        if (fForce0 < 0.001f)
            return;

        const float forceScale = fForce0 * 500.0f;

        for (int i = 0; i < m_iNumParticles; ++i)
        {
            Particle& par = m_pParticles[i];

            const float dx = par.x.x - pos.x;
            const float dy = par.x.y - pos.y;
            const float dz = par.x.z - pos.z;

            float distSqr = dx * dx + dy * dy + dz * dz;
            if (distSqr < 100.0f)
                distSqr = 100.0f;

            const float invDistSqr = 1.0f / distSqr;
            const float impulse = invDistSqr * forceScale;

            par.v.x += dx * impulse;
            par.v.y += dy * impulse;
            par.v.z += dz * impulse;

            par.oldx.x -= dx * impulse;
            par.oldx.y -= dy * impulse;
            par.oldx.z -= dz * impulse;
        }
    }

    void ConstrainedParticleSystem::MoveRigidBody(float fTimeStep)
    {
        float timeScale = 1.0f;
        if (m_fPrevTimeStep > 0.0f)
        {
            timeScale = fTimeStep / m_fPrevTimeStep;
            if (timeScale < 0.2f)
                timeScale = 0.2f;
            else if (timeScale > 5.0f)
                timeScale = 5.0f;
        }
        m_fPrevTimeStep = fTimeStep;

        const float gravity = static_cast<float>(g_lGravity.m_iValue);

        const float damping = powf(0.915f, fTimeStep * 20.0f);
        const float gravityStep = fTimeStep * fTimeStep * gravity * 1.333333f;

        for (int i = 0; i < m_iNumParticles; ++i)
        {
            Particle& par = m_pParticles[i];

            const float dx = par.x.x - par.oldx.x;
            const float dy = par.x.y - par.oldx.y;
            const float dz = par.x.z - par.oldx.z;

            par.oldx = par.x;

            if (par.mass < 10000.0f)
            {
                par.x.x += dx * damping * timeScale;
                par.x.y += dy * damping * timeScale;
                par.x.z += dz * damping * timeScale - gravityStep;
            }
        }
    }

    void ConstrainedParticleSystem::ProjectConstraints2(int iters)
    {
        Particle* const particles = m_pParticles;

        // Load particle positions as SSE vectors
        const __m128 p1 = _mm_loadu_ps(reinterpret_cast<const float*>(&particles[1].x));
        const __m128 p2 = _mm_loadu_ps(reinterpret_cast<const float*>(&particles[2].x));
        const __m128 p3 = _mm_loadu_ps(reinterpret_cast<const float*>(&particles[3].x));
        const __m128 p4 = _mm_loadu_ps(reinterpret_cast<const float*>(&particles[4].x));

        // Compute vectors from particle 4 to particles 1, 2, 3
        const __m128 v1 = _mm_sub_ps(p1, p4);
        const __m128 v2 = _mm_sub_ps(p2, p4);
        const __m128 v3 = _mm_sub_ps(p3, p4);

        // Cross product v2 x v1 to get plane normal
        // normal.x = v2.y * v1.z - v2.z * v1.y
        // normal.y = v2.z * v1.x - v2.x * v1.z
        // normal.z = v2.x * v1.y - v2.y * v1.x
        const __m128 v2_yzx = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 0, 2, 1));
        const __m128 v1_yzx = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1));
        const __m128 v2_zxy = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 1, 0, 2));
        const __m128 v1_zxy = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 1, 0, 2));

        const __m128 normal = _mm_sub_ps(
            _mm_mul_ps(v2_yzx, v1_zxy),
            _mm_mul_ps(v2_zxy, v1_yzx)
        );

        // Dot product normal . v3
        const __m128 dotVec = _mm_mul_ps(normal, v3);
        const float dot = _mm_cvtss_f32(dotVec)
                        + _mm_cvtss_f32(_mm_shuffle_ps(dotVec, dotVec, _MM_SHUFFLE(1, 1, 1, 1)))
                        + _mm_cvtss_f32(_mm_shuffle_ps(dotVec, dotVec, _MM_SHUFFLE(2, 2, 2, 2)));

        // Check if particle 3 is below the plane
        if (dot < 0.0f)
        {
            // Find particle with maximum velocity (x - oldx) among first 5
            float maxVelSqr = 0.0f;
            int maxIdx = 0;

            for (int i = 1; i < 5; ++i)
            {
                const __m128 pos = _mm_loadu_ps(reinterpret_cast<const float*>(&particles[i].x));
                const __m128 old = _mm_loadu_ps(reinterpret_cast<const float*>(&particles[i].oldx));
                const __m128 vel = _mm_sub_ps(pos, old);
                const __m128 velSq = _mm_mul_ps(vel, vel);

                const float velSqr = _mm_cvtss_f32(velSq)
                                   + _mm_cvtss_f32(_mm_shuffle_ps(velSq, velSq, _MM_SHUFFLE(1, 1, 1, 1)))
                                   + _mm_cvtss_f32(_mm_shuffle_ps(velSq, velSq, _MM_SHUFFLE(2, 2, 2, 2)));

                if (velSqr > maxVelSqr)
                {
                    maxVelSqr = velSqr;
                    maxIdx = i;
                }
            }

            // Swap particle maxIdx with particle (maxIdx & 3) + 1
            const int swapIdx = (maxIdx & 3) + 1;
            Particle temp = particles[maxIdx];
            particles[maxIdx] = particles[swapIdx];
            particles[swapIdx] = temp;
        }

        // Iterate constraints
        for (int iter = 0; iter < iters; ++iter)
        {
            ParticleConstraint* con = m_pConstraints;
            for (int i = 0; i < m_iNumConstraints; ++i, ++con)
            {
                ZCommonAlgorithms::AdjustPart2rigid(
                    reinterpret_cast<float*>(&con->m_pPar1->x),
                    reinterpret_cast<float*>(&con->m_pPar2->x),
                    con->m_pPar1->mass,
                    con->m_pPar2->mass,
                    con->m_fDist
                );
            }
        }
    }
}