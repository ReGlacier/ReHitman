#include <Glacier/Physics/Fysix/Fysix.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier::Fysix
{
    ZConstraintBody::~ZConstraintBody()
    {
        ZUniMemory::Delete(m_sLinkage);
    }

    void ZConstraintBody::Save(SLoadSave& save)
    {
        // Save reference position
        save.ref_point[0] = m_LastPos[0];
        save.ref_point[1] = m_LastPos[1];
        save.ref_point[2] = m_LastPos[2];

        // Save particles
        save.used_pars = m_pOwner->m_wBodyPars + m_wGhostCount;

        if (save.used_pars)
        {
            save.pars = ZUniMemory::NewArray<SLoadSavePar>(save.used_pars);

            for (uint16_t i = 0; i < save.used_pars; ++i)
            {
                SParticle* par = GetParticle(i);
                save.pars[i].pos[0] = par->x[0];
                save.pars[i].pos[1] = par->x[1];
                save.pars[i].pos[2] = par->x[2];
                save.pars[i].mass = par->m;
                save.pars[i].degree = par->d;
            }
        }
        else
        {
            save.pars = nullptr;
        }

        // Save constraints
        if (m_pIndex)
        {
            save.own_cons = m_pOwner->m_wConsCount;
        }
        else
        {
            save.own_cons = 0;
        }

        if (save.own_cons)
        {
            save.cons = ZUniMemory::NewArray<SLoadSaveCon>(save.own_cons);

            for (uint16_t i = 0; i < save.own_cons; ++i)
            {
                save.cons[i].index1 = m_pIndex[i].ix1;
                save.cons[i].index2 = m_pIndex[i].ix2;
            }
        }
        else
        {
            save.cons = nullptr;
        }
    }

    void ZConstraintBody::Load(const SLoadSave& load)
    {
        // Load reference position
        m_LastPos[0] = load.ref_point[0];
        m_LastPos[1] = load.ref_point[1];
        m_LastPos[2] = load.ref_point[2];

        // Load particles
        m_wGhostCount = load.used_pars - m_pOwner->m_wBodyPars;

        for (uint16_t i = 0; i < load.used_pars; ++i)
        {
            SParticle* par = GetParticle(i);
            par->x[0] = load.pars[i].pos[0];
            par->x[1] = load.pars[i].pos[1];
            par->x[2] = load.pars[i].pos[2];

            // Reset old position to current (no velocity)
            par->o[0] = par->x[0];
            par->o[1] = par->x[1];
            par->o[2] = par->x[2];

            par->m = load.pars[i].mass;
            par->d = load.pars[i].degree;
        }

        // Load constraints
        if (m_pIndex)
        {
            for (uint16_t i = 0; i < load.own_cons; ++i)
            {
                m_pIndex[i].ix1 = load.cons[i].index1;
                m_pIndex[i].ix2 = load.cons[i].index2;
            }
        }
    }

    bool ZConstraintBody::IsLocked(uint16_t par)
    {
        return GetParticle(par)->m >= ZConstraintSystem::sm_INFINITE;
    }

    int ZConstraintBody::GetParticleMass(float& mass, uint16_t par)
    {
        mass = GetParticle(par)->m;

        return 1;
    }

    int ZConstraintBody::GetParticleRadius(float& radius, uint16_t par)
    {
        radius = GetParticle(par)->r;

        return 1;
    }

    ZConstraintBody* ZConstraintBody::Linkage(const ZConstraintBody* body, uint16_t srcLinkPar, uint16_t bodyLinkPar)
    {
        if (body)
        {
            if (!m_sLinkage)
            {
                m_sLinkage = ZUniMemory::New<SLinkage>();
            }

            const auto* pSrcParticle = GetParticle(srcLinkPar);
            const auto* pBodyParticle = body->GetParticle(bodyLinkPar);

            float srcMass = pSrcParticle->m;
            float bodyMass = pBodyParticle->m;
            float totalMass = srcMass + bodyMass;

            m_sLinkage->con.indices.ix1 = bodyLinkPar;
            m_sLinkage->con.indices.ix2 = srcLinkPar;

            if (totalMass == 0.0f)
            {
                m_sLinkage->con.property.weightedInvMass = ZConstraintSystem::sm_INFINITE;
            }
            else
            {
                m_sLinkage->con.property.weightedInvMass = srcMass / totalMass;
            }

            m_sLinkage->con.property.restLengthSqr = 0.0f;
            m_sLinkage->target = body;
        }
        else if (m_sLinkage)
        {
            m_sLinkage->con.property.restLengthSqr = -1.0f;
        }

        return this;
    }

    int ZConstraintBody::SetReferencePosition(const float (&p0)[3])
    {
        m_LastPos[0] = p0[0];
        m_LastPos[1] = p0[1];
        m_LastPos[2] = p0[2];
        
        return 1;
    }

    int ZConstraintBody::LockParticle(uint16_t par)
    {
        auto* pParticle = GetParticle(par);

        if (pParticle->m < ZConstraintSystem::sm_INFINITE)
        {
            pParticle->m += ZConstraintSystem::sm_INFINITE;

            pParticle->o[0] = pParticle->x[0];
            pParticle->o[1] = pParticle->x[1];
            pParticle->o[2] = pParticle->x[2];
        }

        return 1;
    }
    
    int ZConstraintBody::ReleaseParticle(uint16_t par)
    {
        auto* pParticle = GetParticle(par);

        if (pParticle->m <= ZConstraintSystem::sm_INFINITE)
            return 0;

        pParticle->m -= ZConstraintSystem::sm_INFINITE;

        pParticle->o[0] = pParticle->x[0];
        pParticle->o[1] = pParticle->x[1];
        pParticle->o[2] = pParticle->x[2];

        return 1;
    }

    int ZConstraintBody::ReleaseAllParticles()
    {
        uint16_t totalParticles = m_pOwner->m_wBodyPars + m_wGhostCount;

        for (uint16_t i = 0; i < totalParticles; ++i)
        {
            auto* pParticle = GetParticle(i);

            if (pParticle->m > ZConstraintSystem::sm_INFINITE)
            {
                pParticle->m -= ZConstraintSystem::sm_INFINITE;

                pParticle->o[0] = pParticle->x[0];
                pParticle->o[1] = pParticle->x[1];
                pParticle->o[2] = pParticle->x[2];
            }
        }

        return 1;
    }

    float ZConstraintBody::KineticEnergy()
    {
        float fTotalEnergy = 0.0f;
        uint16_t totalParticles = m_pOwner->m_wBodyPars + m_wGhostCount;

        for (uint16_t i = 0; i < totalParticles; ++i)
        {
            auto* pParticle = GetParticle(i); 

            if (pParticle->m < ZConstraintSystem::sm_INFINITE)
            {                
                float invdt = m_pOwner->m_fInvTimeStep;
                float vx = (pParticle->x[0] - pParticle->o[0]) * invdt;
                float vy = (pParticle->x[1] - pParticle->o[1]) * invdt;
                float vz = (pParticle->x[2] - pParticle->o[2]) * invdt;

                float vSq = (vx * vx) + (vy * vy) + (vz * vz);

                fTotalEnergy += vSq * pParticle->m;
            }
        }

        return fTotalEnergy * 0.5f;
    }

    int ZConstraintBody::GetParticlePos(float(&pos)[3], uint16_t par)
    {
        auto* pParticle = GetParticle(par);
        pos[0] = pParticle->x[0];
        pos[1] = pParticle->x[1];
        pos[2] = pParticle->x[2];

        return 1;
    }

    int ZConstraintBody::GetParticlePos(float(&pos)[3], uint16_t par, float fraction)
    {
        auto* pParticle = GetParticle(par);
        
        pos[0] = pParticle->o[0] + (pParticle->x[0] - pParticle->o[0]) * fraction;
        pos[1] = pParticle->o[1] + (pParticle->x[1] - pParticle->o[1]) * fraction;
        pos[2] = pParticle->o[2] + (pParticle->x[2] - pParticle->o[2]) * fraction;

        return 1;
    }

    int ZConstraintBody::MoveParticle(uint16_t par, const float(&displacement)[3], bool moveFixed, bool resetVelocity)
    {
        auto* pParticle = GetParticle(par);

        if (!moveFixed && pParticle->m >= ZConstraintSystem::sm_INFINITE)
            return 0;

        pParticle->x[0] += displacement[0];
        pParticle->x[1] += displacement[1];
        pParticle->x[2] += displacement[2];

        if (resetVelocity)
        {
            pParticle->o[0] = pParticle->x[0];
            pParticle->o[1] = pParticle->x[1];
            pParticle->o[2] = pParticle->x[2];
        }
        
        return 1;
    }

    void ZConstraintBody::HandleCollision(const REFTAB* faces, bool penetrations, bool collisions)
    {
        if (!const_cast<REFTAB*>(faces)->Count())
            return;

        if (!penetrations && !collisions)
            return;

        ZConstraintSystem* pOwner = m_pOwner;
        const uint16_t bodyPars = pOwner->m_wBodyPars;

        bool anyCollision = false;

        for (uint16_t i = 0; i < bodyPars; ++i)
        {
            SParticle* par = &pOwner->m_pPars[m_wParsOffset + i];

            if (par->m >= ZConstraintSystem::sm_INFINITE)
                continue;

            const float radius = par->r;
            if (radius < 0.0f)
                continue;

            // Compute velocity vector
            const float velX = par->x[0] - par->o[0];
            const float velY = par->x[1] - par->o[1];
            const float velZ = par->x[2] - par->o[2];

            const float velLenSqr = velX * velX + velY * velY + velZ * velZ;

            if (velLenSqr < 0.000001f)
                continue;

            float dirX = velX;
            float dirY = velY;
            float dirZ = velZ;
            float moveDist = 0.0f;

            if (penetrations)
            {
                moveDist = sqrtf(velLenSqr);
                const float invLen = 1.0f / moveDist;
                dirX *= invLen;
                dirY *= invLen;
                dirZ *= invLen;
            }

            // Iterate over faces using REFTAB iterator
            REFTAB* nonConstFaces = const_cast<REFTAB*>(faces);
            for (auto faceRef : nonConstFaces->As<uint32_t>())
            {
                const float* faceData = reinterpret_cast<const float*>(faceRef);
                const float* normal = faceData + 1;
                const float* vert0 = faceData + 13;
                const float* invMatrix = faceData + 4;

                // Check if particle is moving towards the face (dot product with normal)
                const float dot = normal[0] * dirX + normal[1] * dirY + normal[2] * dirZ;
                if (dot > 0.0f)
                    continue;

                bool hit = false;
                float pushX = 0.0f;
                float pushY = 0.0f;
                float pushZ = 0.0f;

                if (penetrations)
                {
                    float t;
                    float coliPoint[3];
                    if (ZCommonAlgorithms::IntersectTriangleAndLine3(coliPoint, par->o, &dirX, vert0, invMatrix, &t, false))
                    {
                        hit = true;
                        const float scale = t - 1.0f;
                        pushX = dirX * scale * (radius + moveDist);
                        pushY = dirY * scale * (radius + moveDist);
                        pushZ = dirZ * scale * (radius + moveDist);
                    }
                }

                if (collisions && !hit)
                {
                    if (radius > 0.0f)
                    {
                        float scaledDist[3];
                        float penetration;
                        if (ZCommonAlgorithms::IntersectTriangleAndSphere(
                                reinterpret_cast<const float(*)[3]>(vert0),
                                reinterpret_cast<const float(*)[3]>(&par->x[0]),
                                radius,
                                reinterpret_cast<float(*)[3]>(&scaledDist[0]),
                                scaledDist,
                                &penetration))
                        {
                            hit = true;
                            pushX = scaledDist[0] * penetration;
                            pushY = scaledDist[1] * penetration;
                            pushZ = scaledDist[2] * penetration;
                        }
                    }
                }

                if (hit)
                {
                    // Project push onto normal
                    const float pushDot = pushX * normal[0] + pushY * normal[1] + pushZ * normal[2];
                    pushX = normal[0] * pushDot;
                    pushY = normal[1] * pushDot;
                    pushZ = normal[2] * pushDot;

                    const float pushLenSqr = pushX * pushX + pushY * pushY + pushZ * pushZ;

                    if (pushLenSqr > 0.000001f)
                    {
                        // Apply push to particle position
                        par->x[0] += pushX;
                        par->x[1] += pushY;
                        par->x[2] += pushZ;

                        // Recompute velocity
                        const float newVelX = par->x[0] - par->o[0];
                        const float newVelY = par->x[1] - par->o[1];
                        const float newVelZ = par->x[2] - par->o[2];

                        const float newVelLenSqr = newVelX * newVelX + newVelY * newVelY + newVelZ * newVelZ;

                        if (newVelLenSqr >= 0.000001f)
                        {
                            if (penetrations)
                            {
                                const float newVelLen = sqrtf(newVelLenSqr);
                                const float invNewLen = 1.0f / newVelLen;
                                dirX = newVelX * invNewLen;
                                dirY = newVelY * invNewLen;
                                dirZ = newVelZ * invNewLen;
                                moveDist = newVelLen;
                            }
                            anyCollision = true;
                        }
                    }
                }
            }
        }
    }
    
    void ZConstraintBody::Simulate(SSimResult& result, const float(&p0)[3], const REFTAB* faces)
    {
        // Compute displacement from last position
        const float displacement[3] = {
            p0[0] - m_LastPos[0],
            p0[1] - m_LastPos[1],
            p0[2] - m_LastPos[2]
        };

        bool hadCollision = false;

        // Handle penetrations
        if (faces)
        {
            HandleCollision(faces, true, false);
            hadCollision = true;
        }

        // Integrate
        m_pOwner->Integration(*this, displacement);

        // Update last position
        m_LastPos[0] = p0[0];
        m_LastPos[1] = p0[1];
        m_LastPos[2] = p0[2];

        // Iterate solver
        ZConstraintSystem* pOwner = m_pOwner;
        const uint16_t iterations = pOwner->m_wIterations;

        for (uint16_t iter = 0; iter < iterations; ++iter)
        {
            // Solve linkage if present
            if (m_sLinkage && pOwner->m_wGrpsCount > 0)
                pOwner->m_pGrps[0].LinkSolver(*m_sLinkage);

            // Solve groups
            ZConstraintGroup* pGroup = pOwner->m_pGrps;
            const uint16_t groupCount = pOwner->m_wGrpsCount;

            for (uint16_t i = 0; i < groupCount; ++i, ++pGroup)
            {
                if (!pGroup->m_bEnable || pGroup->m_fStrength == 0.0f)
                    continue;

                const uint16_t consStart = pGroup->m_wConsStart;
                const SConstraintProps* pProps = &pGroup->m_pOwner->m_pConsProps[consStart];

                const SConstraintIndex* pIdx;
                if (m_pIndex)
                    pIdx = &m_pIndex[consStart];
                else
                    pIdx = &pGroup->m_pOwner->m_pConsIndex[consStart];

                const uint32_t consCount = pGroup->m_wConsCount;

                if (pGroup->m_bQuick)
                    pGroup->QuickSolver(consCount, *this, *pProps, *pIdx);
                else
                    pGroup->NormalSolver(consCount, *this, *pProps, *pIdx);
            }
        }

        // Handle collisions
        if (faces)
        {
            HandleCollision(faces, false, true);
            hadCollision = true;
        }

        // Handle fractures
        bool hadFracture = false;
        ZConstraintGroup* pGroup = pOwner->m_pGrps;
        const uint16_t groupCount = pOwner->m_wGrpsCount;

        for (uint16_t i = 0; i < groupCount; ++i, ++pGroup)
        {
            if (!pGroup->m_bEnable || pGroup->m_fStrength == 0.0f)
                continue;

            if (pGroup->HandleFracture(*this, m_wFractId, m_FracVel))
                hadFracture = true;
        }

        // Fill result
        result.energy = KineticEnergy();
        result.collision = hadCollision;
        result.fracture = hadFracture;
    }
    
    void ZConstraintBody::Stabilize(bool integrate, uint16_t iterations)
    {
        if (integrate)
        {
            const float zero[3] = { 0.0f, 0.0f, 0.0f };
            m_pOwner->Integration(*this, zero);
        }

        for (uint16_t iter = 0; iter < iterations; ++iter)
        {
            ZConstraintSystem* pOwner = m_pOwner;
            ZConstraintGroup* pGroup = pOwner->m_pGrps;

            const uint16_t groupCount = pOwner->m_wGrpsCount;
            for (uint16_t i = 0; i < groupCount; ++i, ++pGroup)
            {
                if (!pGroup->m_bEnable || pGroup->m_fStrength == 0.0f)
                    continue;

                const uint16_t consStart = pGroup->m_wConsStart;
                const SConstraintProps* pProps = &pGroup->m_pOwner->m_pConsProps[consStart];

                const SConstraintIndex* pIdx;
                if (m_pIndex)
                    pIdx = &m_pIndex[consStart];
                else
                    pIdx = &pGroup->m_pOwner->m_pConsIndex[consStart];

                const uint32_t consCount = pGroup->m_wConsCount;

                if (pGroup->m_bQuick)
                    pGroup->QuickSolver(consCount, *this, *pProps, *pIdx);
                else
                    pGroup->NormalSolver(consCount, *this, *pProps, *pIdx);
            }
        }
    }
    
    float ZConstraintBody::AverageStrain(uint16_t& id)
    {
        id = 0;
        uint16_t totalAmount = 0;
        float totalStrain = 0.0f;

        ZConstraintSystem* pOwner = m_pOwner;
        ZConstraintGroup* pGroup = pOwner->m_pGrps;

        const uint16_t groupCount = pOwner->m_wGrpsCount;
        for (uint16_t i = 0; i < groupCount; ++i, ++pGroup)
        {
            if (!pGroup->m_bEnable || pGroup->m_fStrength == 0.0f)
                continue;

            float groupStrain = 0.0f;
            uint16_t groupAmount = 0;
            uint16_t groupStrainPar = 0;

            if (pGroup->Strain(*this, groupStrain, groupAmount, groupStrainPar))
            {
                if (groupStrainPar > id)
                    id = groupStrainPar;

                totalStrain += groupStrain;
                totalAmount += groupAmount;
            }
        }

        if (totalAmount)
            return totalStrain / static_cast<float>(totalAmount);

        return 0.0f;
    }

    void ZConstraintBody::FractureInfo(uint16_t& id, float(&vel)[3])
    {
        id = m_wFractId;
        vel[0] = m_FracVel[0];
        vel[1] = m_FracVel[1];
        vel[2] = m_FracVel[2];
    }

    int ZConstraintBody::GetParticleVel(float(&vel)[3], uint16_t par)
    {
        auto* pParticle = GetParticle(par);

        const float dx = pParticle->x[0] - pParticle->o[0];
        const float dy = pParticle->x[1] - pParticle->o[1];
        const float dz = pParticle->x[2] - pParticle->o[2];

        const float invdt = m_pOwner->m_fInvTimeStep;

        vel[0] = dx * invdt;
        vel[1] = dy * invdt;
        vel[2] = dz * invdt;

        return 1;
    }

    SParticle* ZConstraintBody::GetParticle(uint16_t par) const
    {
        return &m_pOwner->m_pPars[par];
    }
}