#include <Glacier/Physics/Fysix/Fysix.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::Fysix
{
    ZConstraintSystem::ZConstraintSystem()
    {
        m_pBodies = nullptr;
        m_fTimeStepSqr = 0.0016f;
        m_fInvTimeStep = 25.f;
        m_pPars = nullptr;
        m_fDamping = 1.0f;
        m_pGrps = nullptr;
        m_pConsIndex = nullptr;
        m_pConsProps = nullptr;
        m_wBodyPars = 0;
        m_wBodsCount = 0;
        m_wParsCount = 0;
        m_wGrpsCount = 0;
        m_wConsCount = 0;
        m_iBodyType = eVOID;
        m_wIterations = 1;
    }

    ZConstraintSystem::~ZConstraintSystem()
    {
        Clear();
    }
    
    void ZConstraintSystem::Clear()
    {
        if (m_wBodsCount > 0 && m_pBodies)
        {
            for (uint16_t i = 0; i < m_wBodsCount; ++i)
            {
                if (m_pBodies[i].m_sLinkage)
                {
                    ZUniMemory::Delete(m_pBodies[i].m_sLinkage);
                    m_pBodies[i].m_sLinkage = nullptr;
                }
            }
        }

        if (m_pBodies)
        {
            ZUniMemory::Delete(m_pBodies);
            m_pBodies = nullptr;
        }

        if (m_pPars)
        {
            ZUniMemory::Delete(m_pPars);
            m_pPars = nullptr;
        }

        if (m_pGrps)
        {
            ZUniMemory::Delete(m_pGrps);
            m_pGrps = nullptr;
        }

        if (m_pConsIndex)
        {
            ZUniMemory::Delete(m_pConsIndex);
            m_pConsIndex = nullptr;
        }

        if (m_pConsProps)
        {
            ZUniMemory::Delete(m_pConsProps);
            m_pConsProps = nullptr;
        }

        m_wBodsCount = 0;
        m_wConsCount = 0;
        m_wGrpsCount = 0;
        m_wParsCount = 0;
        m_wBodyPars = 0;
    }

    bool ZConstraintSystem::Init(const PP_& pp)
    {
        Clear();

        if (pp.b_cnt == 0 || pp.p_cnt == 0)
            return false;

        m_wBodsCount = pp.b_cnt;
        m_wIterations = pp.itors;
        m_fDamping = pp.damping;
        m_fTimeStepSqr = 0.0016f;
        m_fInvTimeStep = 25.0f;

        m_vPersistentForces[0] = pp.fx;
        m_vPersistentForces[1] = pp.fy;
        m_vPersistentForces[2] = pp.fz;

        if (!InitManifold(pp.type))
            return false;

        m_wBodyPars = pp.p_cnt;
        const uint16_t parsPerBody = static_cast<uint16_t>(pp.p_cnt + pp.ghost);
        m_wParsCount = parsPerBody * m_wBodsCount;

        if (m_wParsCount > 0)
        {
            m_pPars = ZUniMemory::NewArray<SParticle>(m_wParsCount);
            memset(m_pPars, 0, sizeof(SParticle) * m_wParsCount);
        }
        else
        {
            m_pPars = nullptr;
        }

        SLinkedObj<PP_Par>* currPar = pp.particles;
        for (uint16_t i = 0; i < m_wBodyPars && currPar; ++i)
        {
            PP_Par* obj = currPar->obj;
            SParticle& p = m_pPars[i];

            p.x[0] = obj->x;
            p.x[1] = obj->y;
            p.x[2] = obj->z;
            p.m = obj->m + (obj->fixed ? sm_INFINITE : 0.0f);
            p.r = obj->r;
            p.d = 0;

            currPar = currPar->next;
        }

        m_wGrpsCount = pp.g_cnt;
        if (m_wGrpsCount > 0)
        {
            m_pGrps = ZUniMemory::NewArray<ZConstraintGroup>(m_wGrpsCount);

            SLinkedObj<PP_Grp>* currGrp = pp.groups;
            for (uint16_t i = 0; i < m_wGrpsCount && currGrp; ++i)
            {
                PP_Grp* obj = currGrp->obj;
                ZConstraintGroup& grp = m_pGrps[i];

                grp.m_eType = static_cast<EGroupType>(obj->type);
                grp.m_fStrength = obj->g;
                grp.m_fAberration = obj->m;
                grp.m_fStrain = obj->e;
                grp.m_pOwner = this;
                grp.m_bEnable = (obj->enabled != 0);

                currGrp = currGrp->next;
            }
        }
        else
        {
            m_pGrps = nullptr;
        }

        m_wConsCount = pp.c_cnt * pp.type;

        if (m_wConsCount > 0)
        {
            m_pConsProps = ZUniMemory::NewArray<SConstraintProps>(m_wConsCount);
            m_pConsIndex = ZUniMemory::NewArray<SConstraintIndex>(m_wConsCount);
        }
        else
        {
            m_pConsProps = nullptr;
            m_pConsIndex = nullptr;
        }

        ZConstraintGroup* currentGrp = nullptr;
        uint16_t grpConsCount = 0;
        uint16_t globalConsIndex = 0;

        SLinkedObj<PP_Con>* currCon = pp.constraints;
        for (uint16_t i = 0; i < pp.c_cnt && currCon; ++i)
        {
            PP_Con* obj = currCon->obj;
            ZConstraintGroup* targetGrp = &m_pGrps[obj->g];

            if (grpConsCount > 0 && currentGrp != targetGrp)
            {
                currentGrp->m_wConsCount = grpConsCount;
                grpConsCount = 0;
                globalConsIndex += (pp.type - 1) * grpConsCount;
            }

            SConstraintIndex& idx = m_pConsIndex[globalConsIndex];
            idx.ix1 = obj->x1;
            idx.ix2 = obj->x2;

            if (currentGrp != targetGrp)
            {
                currentGrp = targetGrp;
                currentGrp->m_wConsStart = globalConsIndex;
            }

            ++grpConsCount;
            if (i == pp.c_cnt - 1 && currentGrp)
            {
                currentGrp->m_wConsCount = grpConsCount;
            }

            if (currentGrp->m_eType == eLENGHT || currentGrp->m_eType == eBREAK)
            {
                m_pPars[idx.ix1].d++;
                m_pPars[idx.ix2].d++;
            }

            globalConsIndex++;
            currCon = currCon->next;
        }

        for (uint16_t b = 1; b < m_wBodsCount; ++b)
        {
            SParticle* dst = &m_pPars[parsPerBody * b];
            memcpy(dst, m_pPars, sizeof(SParticle) * m_wBodyPars);
        }

        if (m_wBodsCount > 0)
        {
            m_pBodies = ZUniMemory::NewArray<ZConstraintBody>(m_wBodsCount);
        }
        else
        {
            m_pBodies = nullptr;
        }

        SLinkedObj<PP_Bdy>* currBdy = pp.bodies;
        for (uint16_t b = 0; b < m_wBodsCount && currBdy; ++b)
        {
            PP_Bdy* bdyObj = currBdy->obj;
            ZConstraintBody& body = m_pBodies[b];

            body.m_pOwner = this;
            body.m_wGhostCount = 0;
            body.m_wParsOffset = parsPerBody * b;

            const ZVector3 vPos { &bdyObj->x };
            const ZMat3x3 mRot {
                bdyObj->r0x, bdyObj->r0y, bdyObj->r0z,
                bdyObj->r1x, bdyObj->r1y, bdyObj->r1z,
                bdyObj->r2x, bdyObj->r2y, bdyObj->r2z
            };
            
            // Compute transform
            for (uint16_t p = 0; p < m_wBodyPars; ++p)
            {
                SParticle& particle = m_pPars[body.m_wParsOffset + p];

                const ZVector3 vLocal(particle.x);

                ZVector3 vWorld = 
                    vPos + (mRot.XAxis() * vLocal.x) 
                         + (mRot.YAxis() * vLocal.y) 
                         + (mRot.ZAxis() * vLocal.z);
                
                // backward copy
                *reinterpret_cast<ZVector3*>(particle.x) = vWorld;
                *reinterpret_cast<ZVector3*>(particle.o) = vWorld;
            }

            if (pp.ghost > 0)
            {
                if (m_wConsCount > 0)
                {
                    body.m_pIndex = ZUniMemory::NewArray<SConstraintIndex>(m_wConsCount);
                    memcpy(body.m_pIndex, m_pConsIndex, sizeof(SConstraintIndex) * m_wConsCount);
                }
                else
                {
                    body.m_pIndex = nullptr;
                }
            }

            currBdy = currBdy->next;
        }

        InitGroups();
        InitConstraints();

        return true;
    }

    int ZConstraintSystem::InitGroups()
    {
        if (m_wGrpsCount == 0)
            return 1;

        for (uint16_t i = 0; i < m_wGrpsCount; ++i)
        {
            ZConstraintGroup& grp = m_pGrps[i];

            // Precompute coffs of group for QuickSolver
            grp.m_fQuickStrength = ((grp.m_fStrength - 4.0f) * grp.m_fStrength) + 4.0f;
            grp.m_fQuickAberration = -(((2.0f - grp.m_fAberration) * grp.m_fAberration) - 1.0f);
            grp.m_fQuickStrain = ((grp.m_fStrain + 2.0f) * grp.m_fStrain) + 1.0f;

            // Check are we able to use QuickSolver here or not
            grp.m_bQuick = (grp.m_fStrength == 1.0f) && (grp.m_fAberration == 0.0f);
        }

        return 1;
    }

    int ZConstraintSystem::InitConstraints()
    {
        if (m_wConsCount == 0)
            return 1;

        for (uint16_t i = 0; i < m_wConsCount; ++i)
        {
            const auto& indices = m_pConsIndex[i];
            auto& props = m_pConsProps[i];

            const SParticle& p1 = m_pPars[indices.ix1];
            const SParticle& p2 = m_pPars[indices.ix2];

            const float dx = p2.x[0] - p1.x[0];
            const float dy = p2.x[1] - p1.x[1];
            const float dz = p2.x[2] - p1.x[2];

            props.restLengthSqr = (dx * dx) + (dy * dy) + (dz * dz);

            const float totalMass = p1.m + p2.m;

            if (totalMass == 0.0f)
            {
                props.weightedInvMass = sm_INFINITE;
            }
            else
            {
                props.weightedInvMass = p2.m / totalMass;
            }
        }

        return 1;
    }

    bool ZConstraintSystem::InitManifold(uint16_t type)
    {
        if (m_iBodyType == eVOID)
        {
            m_iBodyType = static_cast<EManifold>(type);
            
            if (type > eVOID && type <= eSOLID)
            {
                return true;
            }
        }

        return false;
    }

    void ZConstraintSystem::Integration(const ZConstraintBody& body, const float (&moved)[3])
    {
        const uint16_t totalPars = body.m_wGhostCount + m_wBodyPars;
        if (totalPars == 0)
            return;

        const ZVector3 vMoved(moved);
        const ZVector3 vForces(m_vPersistentForces);

        for (uint16_t i = 0; i < totalPars; ++i)
        {
            SParticle& particle = m_pPars[body.m_wParsOffset + i];

            if (particle.m < sm_INFINITE)
            {
                ZVector3& vCurrPos = *reinterpret_cast<ZVector3*>(particle.x);
                ZVector3& vOldPos = *reinterpret_cast<ZVector3*>(particle.o);

                const ZVector3 vVelocity = (vCurrPos - vOldPos) * m_fDamping;
                const ZVector3 vPrevPosCopy = vCurrPos;

                vCurrPos = vCurrPos + vVelocity + (vForces * m_fTimeStepSqr);
                vOldPos = vPrevPosCopy;
            }
            else
            {
                ZVector3& vCurrPos = *reinterpret_cast<ZVector3*>(particle.x);
                vCurrPos += vMoved;
            }
        }
    }

    ZConstraintBody* ZConstraintSystem::GetBody(uint16_t body)
    {
        if (body >= m_wBodsCount)
        {
            return nullptr;
        } 

        return &m_pBodies[body];
    }
}