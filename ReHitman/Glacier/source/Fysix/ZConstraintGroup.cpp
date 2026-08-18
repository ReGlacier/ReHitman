#include <Glacier/Fysix/Fysix.h>

#include <xmmintrin.h>


namespace Glacier::Fysix
{
    ZConstraintGroup::~ZConstraintGroup() = default;

    SConstraintIndex* ZConstraintGroup::ConsIndexStart(const ZConstraintBody& body)
    {
        SConstraintIndex* pIndex = body.m_pIndex;
        if (pIndex)
        {
            return pIndex + m_wConsStart;
        }
        else
        {
            return m_pOwner->m_pConsIndex + m_wConsStart;
        }
    }

    void ZConstraintGroup::NormalSolver(const uint32_t lCount, const ZConstraintBody& body, const SConstraintProps& prop, const SConstraintIndex& idx)
    {
        if (!lCount)
            return;

        uint32_t count = lCount;

        const __m128 inf = _mm_set1_ps(ZConstraintSystem::sm_INFINITE);
        // Mask to keep the mass component when storing the updated position (xyz write, w keep).
        const __m128 keepMassMask = _mm_castsi128_ps(_mm_set_epi32(-1, 0, 0, 0));

        SParticle* const pars = body.m_pOwner->m_pPars;
        const uint32_t parsOffset = body.m_wParsOffset;

        const SConstraintProps* pProp = &prop;
        const SConstraintIndex* pIdx = &idx;

        do
        {
            const float restLengthSqr = pProp->restLengthSqr;
            const float aberration = m_fAberration;
            const float quickAberration = m_fQuickAberration;

            SParticle* const par1 = &pars[pIdx->ix1 + parsOffset];
            SParticle* const par2 = &pars[pIdx->ix2 + parsOffset];

            const __m128 pos1 = _mm_loadu_ps(&par1->x[0]);
            const __m128 pos2 = _mm_loadu_ps(&par2->x[0]);
            const __m128 delta = _mm_sub_ps(pos2, pos1);

            // dot = delta.x*d.x + delta.y*d.y + delta.z*d.z
            const __m128 deltaSq = _mm_mul_ps(delta, delta);
            const float dot = _mm_cvtss_f32(deltaSq) + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(1, 1, 1, 1)))
                            + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(2, 2, 2, 2)));

            const float limit = quickAberration * restLengthSqr;

            if (aberration == 0.0f || dot < limit)
            {
                // factor = (1.0f - (2.0f * limit) / (dot + limit)) * m_fStrength
                const float factor = (1.0f - (2.0f * limit) / (dot + limit)) * m_fStrength;

                // w1 = (m1 >= INF) ? 0.0f : ((m2 >= INF) ? 1.0f : weightedInvMass)
                const float m1 = par1->m;
                const float m2 = par2->m;
                const float w1 = (m1 >= ZConstraintSystem::sm_INFINITE) ? 0.0f : ((m2 >= ZConstraintSystem::sm_INFINITE) ? 1.0f : pProp->weightedInvMass);

                const float scale1 = w1 * factor;
                const float scale2 = scale1 - factor;

                const __m128 scaledDelta1 = _mm_mul_ps(delta, _mm_set1_ps(scale1));
                const __m128 scaledDelta2 = _mm_mul_ps(delta, _mm_set1_ps(scale2));

                // par1->x += scaledDelta1 (keep m)
                _mm_storeu_ps(&par1->x[0], _mm_or_ps(_mm_andnot_ps(keepMassMask, _mm_add_ps(pos1, scaledDelta1)), _mm_and_ps(keepMassMask, pos1)));
                // par2->x += scaledDelta2 (keep m)
                _mm_storeu_ps(&par2->x[0], _mm_or_ps(_mm_andnot_ps(keepMassMask, _mm_add_ps(pos2, scaledDelta2)), _mm_and_ps(keepMassMask, pos2)));
            }

            ++pProp;
            ++pIdx;
        } while (--count);
    }

    void ZConstraintGroup::QuickSolver(uint32_t lCount, const ZConstraintBody& body, const SConstraintProps& prop, const SConstraintIndex& idx)
    {
        if (!lCount)
            return;

        const __m128 inf = _mm_set1_ps(ZConstraintSystem::sm_INFINITE);
        // Mask to keep the mass component when storing the updated position (xyz write, w keep).
        const __m128 keepMassMask = _mm_castsi128_ps(_mm_set_epi32(-1, 0, 0, 0));

        SParticle* const pars = body.m_pOwner->m_pPars;
        const uint32_t parsOffset = body.m_wParsOffset;

        const SConstraintProps* pProp = &prop;
        const SConstraintIndex* pIdx = &idx;

        do
        {
            // restLength = sqrtf(pProp->restLengthSqr)
            const float restLengthSqr = pProp->restLengthSqr;
            const __m128 restLenSqrVec = _mm_set1_ps(restLengthSqr);
            __m128 rsqrt = _mm_rsqrt_ps(restLenSqrVec);
            // Two Newton-Raphson refinement steps: rsqrt = rsqrt * (1.5 - 0.5 * x * rsqrt * rsqrt)
            const __m128 halfX = _mm_mul_ps(_mm_set1_ps(0.5f), restLenSqrVec);
            const __m128 threeHalfs = _mm_set1_ps(1.5f);
            rsqrt = _mm_mul_ps(rsqrt, _mm_sub_ps(threeHalfs, _mm_mul_ps(halfX, _mm_mul_ps(rsqrt, rsqrt))));
            rsqrt = _mm_mul_ps(rsqrt, _mm_sub_ps(threeHalfs, _mm_mul_ps(halfX, _mm_mul_ps(rsqrt, rsqrt))));
            const float restLength = restLengthSqr * _mm_cvtss_f32(rsqrt);

            SParticle* const par1 = &pars[pIdx->ix1 + parsOffset];
            SParticle* const par2 = &pars[pIdx->ix2 + parsOffset];

            const __m128 pos1 = _mm_loadu_ps(&par1->x[0]);
            const __m128 pos2 = _mm_loadu_ps(&par2->x[0]);
            const __m128 delta = _mm_sub_ps(pos2, pos1);

            // dot = delta.x*d.x + delta.y*d.y + delta.z*d.z
            const __m128 deltaSq = _mm_mul_ps(delta, delta);
            const float dot = _mm_cvtss_f32(deltaSq) + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(1, 1, 1, 1)))
                            + _mm_cvtss_f32(_mm_shuffle_ps(deltaSq, deltaSq, _MM_SHUFFLE(2, 2, 2, 2)));

            // w1 = (m1 >= INF) ? 0.0f : ((m2 >= INF) ? 1.0f : weightedInvMass)
            const float m1 = par1->m;
            const float m2 = par2->m;
            const float w1 = (m1 >= ZConstraintSystem::sm_INFINITE) ? 0.0f : ((m2 >= ZConstraintSystem::sm_INFINITE) ? 1.0f : pProp->weightedInvMass);

            const float scale = (dot - restLength) * w1;
            const __m128 scaledDelta = _mm_mul_ps(delta, _mm_set1_ps(scale));

            // par1->x += scaledDelta (keep m)
            _mm_storeu_ps(&par1->x[0], _mm_or_ps(_mm_andnot_ps(keepMassMask, _mm_add_ps(pos1, scaledDelta)), _mm_and_ps(keepMassMask, pos1)));
            // par2->x -= scaledDelta (keep m)
            _mm_storeu_ps(&par2->x[0], _mm_or_ps(_mm_andnot_ps(keepMassMask, _mm_sub_ps(pos2, scaledDelta)), _mm_and_ps(keepMassMask, pos2)));

            ++pProp;
            ++pIdx;
        } while (--lCount);
    }

    void ZConstraintGroup::LinkSolver(const SLinkage& con)
    {
        if (con.con.property.restLengthSqr != 0.0f)
            return;
            
        auto* pSrcParticle = &m_pOwner->m_pPars[con.con.indices.ix2];
        auto* pTargetParticle = &con.target->m_pOwner->m_pPars[con.con.indices.ix1];

        const float deltaX = pSrcParticle->x[0] - pTargetParticle->x[0];
        const float deltaY = pSrcParticle->x[1] - pTargetParticle->x[1];
        const float deltaZ = pSrcParticle->x[2] - pTargetParticle->x[2];

        float weight = 0.0f;

        if (pTargetParticle->m < ZConstraintSystem::sm_INFINITE)
        {
            if (pSrcParticle->m < ZConstraintSystem::sm_INFINITE)
            {
                weight = con.con.property.weightedInvMass;
            }
            else
            {
                weight = 1.0f;
            }
        }

        pTargetParticle->x[0] += deltaX * weight;
        pTargetParticle->x[1] += deltaY * weight;
        pTargetParticle->x[2] += deltaZ * weight;

        float srcWeight = weight - 1.0f;
        pSrcParticle->x[0] += deltaX * srcWeight;
        pSrcParticle->x[1] += deltaY * srcWeight;
        pSrcParticle->x[2] += deltaZ * srcWeight;
    }

    bool ZConstraintGroup::HandleFracture(ZConstraintBody& body, uint16_t& id, float(&velocity)[3])
    {
        if (m_fStrain == 0.0f)
            return false;

        ZConstraintSystem* pOwner = m_pOwner;
        const EManifold bodyType = pOwner->m_iBodyType;

        // TODO: Finish this place after ZConstraintGroup::ConsIndexStart will be reversed
        // SConstraintIndex* pIdx = ConsIndexStart(body);
        SConstraintIndex* pIdx = nullptr; // placeholder

        const float* pRestLengthSqr = &pOwner->m_pConsProps[m_wConsStart].restLengthSqr;
        const uint32_t consCount = m_wConsCount;

        if (!consCount || !pIdx)
            return false;

        const uint32_t parsOffset = body.m_wParsOffset;
        SParticle* const pars = pOwner->m_pPars;

        for (uint32_t i = 0; i < consCount; ++i, ++pIdx, ++pRestLengthSqr)
        {
            const SParticle* const par1 = &pars[pIdx->ix1 + parsOffset];
            const SParticle* const par2 = &pars[pIdx->ix2 + parsOffset];

            const float dx = par2->x[0] - par1->x[0];
            const float dy = par2->x[1] - par1->x[1];
            const float dz = par2->x[2] - par1->x[2];
            const float distSqr = dx * dx + dy * dy + dz * dz;

            if (distSqr > (*pRestLengthSqr * m_fQuickStrain))
            {
                uint16_t fracturedIx = 0xFFFF;

                if (bodyType < par1->d || bodyType < par2->d)
                {
                    if (bodyType >= par1->d)
                    {
                        fracturedIx = pIdx->ix2;
                    }
                    else if (bodyType < par2->d)
                    {
                        const float ox1 = par1->x[0] - par1->o[0];
                        const float oy1 = par1->x[1] - par1->o[1];
                        const float oz1 = par1->x[2] - par1->o[2];
                        const float distSqr1 = ox1 * ox1 + oy1 * oy1 + oz1 * oz1;

                        const float ox2 = par2->x[0] - par2->o[0];
                        const float oy2 = par2->x[1] - par2->o[1];
                        const float oz2 = par2->x[2] - par2->o[2];
                        const float distSqr2 = ox2 * ox2 + oy2 * oy2 + oz2 * oz2;

                        fracturedIx = (distSqr1 <= distSqr2) ? pIdx->ix2 : pIdx->ix1;
                    }
                    else
                    {
                        fracturedIx = pIdx->ix1;
                    }
                }

                if (fracturedIx == 0xFFFF)
                    continue;

                SParticle* const fracturedPar = &pars[fracturedIx + parsOffset];
                --fracturedPar->d;

                const uint16_t ghostIx = pOwner->m_wBodyPars + body.m_wGhostCount;
                body.m_wGhostCount = body.m_wGhostCount + 1;

                memcpy(&pars[parsOffset + ghostIx], fracturedPar, sizeof(SParticle));

                body.ReleaseParticle(ghostIx);

                id = fracturedIx;

                if (fracturedIx == pIdx->ix1)
                {
                    pIdx->ix1 = ghostIx;
                    velocity[0] = par1->x[0] - par1->o[0];
                    velocity[1] = par1->x[1] - par1->o[1];
                    velocity[2] = par1->x[2] - par1->o[2];
                }
                else
                {
                    pIdx->ix2 = ghostIx;
                    velocity[0] = par2->x[0] - par2->o[0];
                    velocity[1] = par2->x[1] - par2->o[1];
                    velocity[2] = par2->x[2] - par2->o[2];
                }

                body.LockParticle(pIdx->ix1);
                body.LockParticle(pIdx->ix2);

                return true;
            }
        }

        return false;
    }

    bool ZConstraintGroup::Strain(const ZConstraintBody& body, float& strain, uint16_t& amount, uint16_t& strainPar)
    {
        if (m_eType != eLENGHT || !m_wConsCount)
            return false;

        strain = 0.0f;
        strainPar = 0;

        const float* pRestLengthSqr = &m_pOwner->m_pConsProps[m_wConsStart].restLengthSqr;

        // TODO: Finish this place after ZConstraintGroup::ConsIndexStart will be reversed
        // SConstraintIndex* pIdx = ConsIndexStart(body);
        SConstraintIndex* pIdx = nullptr; // placeholder

        if (!pIdx)
            return false;

        const uint32_t parsOffset = body.m_wParsOffset;
        const SParticle* const pars = body.m_pOwner->m_pPars;

        const uint32_t consCount = m_wConsCount;
        for (uint32_t i = 0; i < consCount; ++i, ++pIdx, ++pRestLengthSqr)
        {
            const SParticle* const par1 = &pars[pIdx->ix1 + parsOffset];
            const SParticle* const par2 = &pars[pIdx->ix2 + parsOffset];

            const float dx = par2->x[0] - par1->x[0];
            const float dy = par2->x[1] - par1->x[1];
            const float dz = par2->x[2] - par1->x[2];
            const float distSqr = dx * dx + dy * dy + dz * dz;

            const float strainVal = 1.0f - (distSqr / (*pRestLengthSqr * m_fQuickStrength));
            strain += strainVal;

            if (strainVal > 0.0f)
                strainPar = pIdx->ix1;
        }

        amount = consCount;
        strain = strain * 0.5f;

        return true;
    }
}