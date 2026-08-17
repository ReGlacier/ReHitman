#include <Glacier/Fysix/ZBoneExtend.h>
#include <Glacier/Fysix/ZCommonAlgorithms.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <cmath>


namespace Glacier
{
    namespace
    {
        // TODO: Register "BXPar" debug var (ZDebugIntRef: "Bone Extend Test Hit Particle (0:first, -1:last)", min -1000000, max 1000000, step 1)
        int32_t g_iBXPar = -1;
        // TODO: Register "BXHit" debug var (ZDebugFloatRef: "Bone Extend Test Hit Strength")
        float g_fBXHit = 1.0f;

        void RandomUnitVector(float* vec)
        {
            float fLength;

            do
            {
                vec[0] = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 0.5f;
                vec[1] = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 0.5f;
                vec[2] = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 0.5f;
                fLength = vlen(vec);
            }
            while (fLength < 0.000001f);

            vscalar(vec, 1.0f / fLength);
        }
    }

    SBoneCollision::SRegions::SRegions()
    {
        changed = false;
        enabled = true;
        center.Reset();
        size.Reset();
    }

    SBoneCollision::SBoneCollision(uint16_t bodies)
    {
        SBoneCollision::SRegions* pRegions = nullptr;

        if (bodies)
        {
            pRegions = (SBoneCollision::SRegions*)ZUniMemory::Allocate(sizeof(SBoneCollision::SRegions) * bodies);
            for (uint16_t i = 0; i < bodies; ++i)
            {
                new (&pRegions[i]) SBoneCollision::SRegions();
            }
        }

        regions = pRegions;
        coliBox = nullptr;
        last = -1;
    }

    SBoneCollision::~SBoneCollision()
    {
        if (regions)
        {
            ZUniMemory::Delete(regions);
        }

        if (coliBox)
        {
            ZUniMemory::Delete(coliBox);
        }
    }

    SBoneMapper::SBoneMapper()
    {
        m_wBone = 0;
        m_wPar = 0;
        m_wLinkBone = 0;
        m_wLinksCount = 0;
        m_pLinks = nullptr;
    }

    SBoneMapper::~SBoneMapper()
    {
        if (m_pLinks)
        {
            ZUniMemory::Free(m_pLinks);
        }
    }

    ZBoneExtend::ZBoneExtend()
        : ZDynamicsExtend()
    {
        m_pCollision = nullptr;
    }

    ZBoneExtend::~ZBoneExtend()
    {
        DestroyMapper();

        if (m_pCollision)
        {
            ZUniMemory::Delete(m_pCollision);
        }
    }

    bool ZBoneExtend::Create(const Fysix::PP_* pp, const SGenericMapper* mpr, uint16_t mappings)
    {
        if (ZDynamicsExtend::Create(pp, mpr, mappings))
        {
            m_pCollision = ZUniMemory::New<SBoneCollision>(pp->b_cnt);
            return m_pCollision != nullptr;
        }

        return false;
    }

    bool ZBoneExtend::Update(uint16_t body, ZGEOM* geom)
    {
        ZLNKOBJ* pLnkObj = geom->IsDerivedFrom<ZLNKOBJ>() ? static_cast<ZLNKOBJ*>(geom) : nullptr;
        ZASSERT(pLnkObj);

        if (m_pProps[body].visible)
        {
            return UpdateBones(body, pLnkObj, pLnkObj->m_Model->m_Bones);
        }

        return false;
    }

    bool ZBoneExtend::CreateMapper(const SGenericMapper* mpr, uint16_t mappings)
    {
        if (!mappings)
        {
            return false;
        }

        m_wMappings = mappings;

        // Allocate array with count header: [count][SBoneMapper x mappings]
        void* pRaw = ZUniMemory::Allocate(sizeof(uint32_t) + sizeof(SBoneMapper) * mappings);
        SBoneMapper* pMappers = nullptr;

        if (pRaw)
        {
            *static_cast<uint32_t*>(pRaw) = mappings;

            pMappers = reinterpret_cast<SBoneMapper*>(static_cast<uint8_t*>(pRaw) + sizeof(uint32_t));
            for (uint16_t i = 0; i < mappings; ++i)
            {
                new (&pMappers[i]) SBoneMapper();
            }
        }

        m_pMapper = pMappers;

        // Copy data from source mapper
        const auto* pSrc = reinterpret_cast<const SBoneMapper*>(mpr);

        for (uint16_t i = 0; i < mappings; ++i)
        {
            pMappers[i].m_wBone = pSrc[i].m_wBone;
            pMappers[i].m_wPar = pSrc[i].m_wPar;
            pMappers[i].m_wLinkBone = pSrc[i].m_wLinkBone;
            pMappers[i].m_wLinksCount = pSrc[i].m_wLinksCount;

            if (pSrc[i].m_wLinksCount)
            {
                pMappers[i].m_pLinks = static_cast<uint16_t*>(ZUniMemory::Allocate(2 * pSrc[i].m_wLinksCount));
                for (uint16_t j = 0; j < pSrc[i].m_wLinksCount; ++j)
                {
                    pMappers[i].m_pLinks[j] = pSrc[i].m_pLinks[j];
                }
            }
            else
            {
                pMappers[i].m_pLinks = nullptr;
            }
        }

        return true;
    }

    void ZBoneExtend::DestroyMapper()
    {
        if (m_pMapper)
        {
            auto* pMappers = static_cast<SBoneMapper*>(m_pMapper);
            const uint32_t lCount = *reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(m_pMapper) - sizeof(uint32_t));

            // Destruct in reverse order
            for (int32_t i = static_cast<int32_t>(lCount) - 1; i >= 0; --i)
            {
                pMappers[i].~SBoneMapper();
            }

            ZUniMemory::Free(reinterpret_cast<uint8_t*>(m_pMapper) - sizeof(uint32_t));
        }

        m_pMapper = nullptr;
    }

    const REFTAB* ZBoneExtend::GetCollisionFaces(uint16_t body, ZGEOM* geom)
    {
        SBoneCollision::SRegions* pRegion = &m_pCollision->regions[body];

        if (!pRegion->enabled)
        {
            return nullptr;
        }

        if (!FindCollisionRegion(body))
        {
            return nullptr;
        }

        const bool bNewBody = m_pCollision->last != body;

        if (pRegion->changed || m_pCollision->last != body)
        {
            ZMat3x3 mIdentity;
            mIdentity.Reset();

            m_pCollision->coliBox->SetBox(pRegion->center, mIdentity, pRegion->size, bNewBody);
            m_pCollision->last = body;
            pRegion->changed = false;
        }

        if (m_pCollision->coliBox->GetNumFaces())
        {
            return m_pCollision->coliBox->GetFacesPtr();
        }

        return nullptr;
    }

    bool ZBoneExtend::Collision(uint16_t body, ZGEOM* geom)
    {
        ZASSERT(ZDynamicsExtend::m_msgCollision);

        geom->SendCommand(ZDynamicsExtend::m_msgCollision, (void*)this, nullptr);
        return true;
    }

    bool ZBoneExtend::Fracture(uint16_t body, ZGEOM* geom)
    {
        auto* pBody = m_kConSys.GetBody(body);

        SBodyFracture sFracInfo;
        sFracInfo.frac_obj = static_cast<uint16_t>(geom->GetRef());

        uint16_t wFractPar;
        pBody->FractureInfo(wFractPar, *reinterpret_cast<float(*)[3]>(sFracInfo.frac_vel.Get()));
        sFracInfo.frac_bone = FindBone(wFractPar);

        geom->SendCommand(ZDynamicsExtend::m_msgFracture, &sFracInfo, nullptr);
        return true;
    }

    bool ZBoneExtend::FindCollisionRegion(uint16_t body)
    {
        auto* pBody = m_kConSys.GetBody(body);
        const auto* pMapper = static_cast<const SBoneMapper*>(m_pMapper);

        float vMin[3] = { 1.0e30f, 1.0e30f, 1.0e30f };
        float vMax[3] = { -1.0e30f, -1.0e30f, -1.0e30f };

        for (uint16_t i = 0; i < m_wMappings; ++i, ++pMapper)
        {
            if (pMapper->m_wBone && !pBody->IsLocked(pMapper->m_wPar))
            {
                float fRadius;
                pBody->GetParticleRadius(fRadius, pMapper->m_wPar);

                if (fRadius >= 0.0f)
                {
                    float vPos[3];
                    pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vPos), pMapper->m_wPar);

                    vPos[0] -= fRadius;
                    vPos[1] -= fRadius;
                    vPos[2] -= fRadius;
                    vmin(vMin, vPos);

                    vPos[0] += fRadius + fRadius;
                    vPos[1] += fRadius + fRadius;
                    vPos[2] += fRadius + fRadius;
                    vmax(vMax, vPos);
                }
            }
        }

        if (vMax[0] > vMin[0] && vMax[1] > vMin[1] && vMax[2] > vMin[2])
        {
            float vSize[3];
            float vCenter[3];

            vsub(vSize, vMax, vMin);
            vscalar(vSize, 0.5f);
            vadd(vCenter, vMin, vSize);

            SetCollisionRegion(body, *reinterpret_cast<const ZVector3*>(vCenter), *reinterpret_cast<const ZVector3*>(vSize));
            return true;
        }

        RemoveCollisionRegion(body);
        return false;
    }

    void ZBoneExtend::SetCollisionRegion(uint16_t body, const ZVector3& center, const ZVector3& size)
    {
        ZASSERT(m_pCollision);

        if (!m_pCollision->coliBox)
        {
            m_pCollision->coliBox = ZUniMemory::New<ZFastBoxColi>(0.f, 0);
            m_pCollision->coliBox->EnableExtendedMode(true);
            m_pCollision->coliBox->IgnoreMovingObjects(true);
            m_pCollision->last = 0xFFFF;
        }

        SBoneCollision::SRegions* pRegion = &m_pCollision->regions[body];
        pRegion->center = center;
        pRegion->size = size;
        pRegion->changed = true;
        pRegion->enabled = true;
    }

    void ZBoneExtend::RemoveCollisionRegion(uint16_t body)
    {
        ZASSERT(m_pCollision);

        m_pCollision->regions[body].enabled = false;
    }

    bool ZBoneExtend::UpdateBones(uint16_t body, const ZLNKOBJ* linkObj, ZBone* bones)
    {
        ZASSERT(m_pMapper);
        ZASSERT(linkObj);

        auto* pBody = m_kConSys.GetBody(body);
        const auto* pMappers = static_cast<const SBoneMapper*>(m_pMapper);

        for (uint16_t i = 0; i < m_wMappings; ++i)
        {
            const SBoneMapper* pMapper = &pMappers[i];

            if (!pMapper->m_wBone)
            {
                continue;
            }

            ZBone* pBone = &bones[pMapper->m_wBone];
            pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(pBone->_Pos.Get()), pMapper->m_wPar);

            if (pMapper->m_wLinkBone && pMapper->m_wLinksCount)
            {
                float vPrevPos[3];
                float vNextPos[3];
                float vDir0[3];
                float vDir1[3];
                float vAxis[3];
                ZMat3x3 mRot;

                pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vPrevPos), pMappers[pMapper->m_wLinkBone].m_wPar);
                pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vNextPos), pMappers[pMapper->m_pLinks[0]].m_wPar);

                vsub(vDir0, pBone->_Pos.Get(), vPrevPos);
                vnorm(vDir0);
                linkObj->BaseGeom()->GetLocalVect(*reinterpret_cast<ZVector3*>(vDir0));

                vsub(vDir1, vNextPos, pBone->_Pos.Get());
                vnorm(vDir1);
                linkObj->BaseGeom()->GetLocalVect(*reinterpret_cast<ZVector3*>(vDir1));

                vcross(vAxis, vDir0, vDir1);
                const float fSin = vnorm(vAxis);
                const float fCos = vdot(vDir0, vDir1);

                mrotaxis2(fCos, -fSin, vAxis, mRot.Get());
                mmmul(pBone->_Mat.Get(), bones[pMapper->m_wLinkBone]._Mat.Get(), mRot.Get());
            }

            linkObj->GetLocalPoint(pBone->_Pos);
        }

        return true;
    }

    uint16_t ZBoneExtend::GetLinks()
    {
        uint16_t wLinks = 0;
        const auto* pMapper = static_cast<const SBoneMapper*>(m_pMapper);

        for (uint16_t i = 0; i < m_wMappings; ++i, ++pMapper)
        {
            if (pMapper->m_wLinkBone && pMapper->m_wLinksCount)
            {
                wLinks += pMapper->m_wLinksCount;
            }
        }

        return wLinks;
    }

    bool ZBoneExtend::LinkDeformations(uint16_t body, SLinkDeformation* deformations)
    {
        auto* pBody = m_kConSys.GetBody(body);
        const auto* pMappers = static_cast<const SBoneMapper*>(m_pMapper);

        for (uint16_t i = 0; i < m_wMappings; ++i)
        {
            const SBoneMapper* pMapper = &pMappers[i];

            if (pMapper->m_wLinkBone && pMapper->m_wLinksCount)
            {
                float vPos[3];
                float vLinkPos[3];
                float vDir[3];

                pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vPos), pMapper->m_wPar);
                pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vLinkPos), pMappers[pMapper->m_wLinkBone].m_wPar);
                vsub(vDir, vPos, vLinkPos);

                for (uint16_t j = 0; j < pMapper->m_wLinksCount; ++j)
                {
                    const SBoneMapper* pLinkMapper = &pMappers[pMapper->m_pLinks[j]];

                    if (pBody->IsLocked(pLinkMapper->m_wPar))
                    {
                        deformations->m_fDeformation = 1.0f;
                    }
                    else
                    {
                        float vOtherPos[3];
                        float vDelta[3];

                        pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vOtherPos), pLinkMapper->m_wPar);
                        vsub(vDelta, vOtherPos, vPos);

                        deformations->m_fDeformation = vdot(vDir, vDelta);
                    }

                    deformations->m_wBone = pLinkMapper->m_wBone;
                    ++deformations;
                }
            }
        }

        return true;
    }

    bool ZBoneExtend::Hit(uint16_t body)
    {
        auto* pBody = m_kConSys.GetBody(body);

        uint16_t wMapper;
        if (g_iBXPar == -1 || m_wMappings - 1 < g_iBXPar)
        {
            wMapper = m_wMappings - 1;
        }
        else
        {
            wMapper = static_cast<uint16_t>(g_iBXPar);
        }

        const auto* pMapper = static_cast<const SBoneMapper*>(m_pMapper) + wMapper;

        float vVel[3];
        pBody->GetParticleVel(*reinterpret_cast<float(*)[3]>(vVel), pMapper->m_wPar);

        if (vlen2(vVel) < 1.0f)
        {
            RandomUnitVector(vVel);
            vVel[1] = std::fabs(vVel[1]);
        }

        vnorm(vVel);
        vscalar(vVel, g_fBXHit);

        pBody->MoveParticle(pMapper->m_wPar, reinterpret_cast<const float(*)[3]>(vVel), false, false);
        WakeUp(body);
        return true;
    }

    bool ZBoneExtend::Hit(uint16_t body, const ZVector3& pos, const ZVector3& dir)
    {
        float fMaxDist = 0.0f;
        const uint16_t wPar = FindNearestParticle(body, pos, fMaxDist);

        auto* pBody = m_kConSys.GetBody(body);

        float fMass;
        pBody->GetParticleMass(fMass, wPar);

        float vDisplacement[3];
        vscalar(vDisplacement, dir.Get(), fMass);

        pBody->MoveParticle(wPar, reinterpret_cast<const float(*)[3]>(vDisplacement), false, false);
        WakeUp(body);
        return true;
    }

    bool ZBoneExtend::Push(uint16_t body, const ZVector3& vel)
    {
        auto* pBody = m_kConSys.GetBody(body);
        const auto* pMapper = static_cast<const SBoneMapper*>(m_pMapper);

        for (uint16_t i = 0; i < m_wMappings; ++i, ++pMapper)
        {
            pBody->MoveParticle(pMapper->m_wPar, reinterpret_cast<const float(*)[3]>(vel.Get()), false, false);
        }

        WakeUp(body);
        return true;
    }

    bool ZBoneExtend::Grab(uint16_t body, uint16_t& par, const ZVector3& pos, const float& radius)
    {
        par = 0;

        const uint16_t wPar = FindNearestParticle(body, pos, radius);
        if (!wPar)
        {
            return false;
        }

        auto* pBody = m_kConSys.GetBody(body);
        pBody->LockParticle(wPar);

        float vParPos[3];
        float vDisplacement[3];

        pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vParPos), wPar);
        vsub(vDisplacement, pos.Get(), vParPos);

        pBody->MoveParticle(wPar, reinterpret_cast<const float(*)[3]>(vDisplacement), true, true);
        WakeUp(body);

        par = wPar;
        return true;
    }

    bool ZBoneExtend::Grab(uint16_t body, const ZVector3& pos, uint16_t par)
    {
        if (!par)
        {
            return false;
        }

        auto* pBody = m_kConSys.GetBody(body);

        if (!pBody->IsLocked(par))
        {
            return false;
        }

        float vParPos[3];
        float vDisplacement[3];

        pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vParPos), par);
        vsub(vDisplacement, pos.Get(), vParPos);

        pBody->MoveParticle(par, reinterpret_cast<const float(*)[3]>(vDisplacement), true, true);
        WakeUp(body);
        return true;
    }

    bool ZBoneExtend::Release(uint16_t body, const ZVector3& pos, const float& radius)
    {
        return Release(body, FindNearestParticle(body, pos, radius));
    }

    bool ZBoneExtend::Release(uint16_t body, uint16_t par)
    {
        if (!par)
        {
            return false;
        }

        auto* pBody = m_kConSys.GetBody(body);

        if (pBody->ReleaseParticle(par))
        {
            m_pCollision->regions[body].enabled = true;
            WakeUp(body);
        }

        return true;
    }

    bool ZBoneExtend::ReleaseSystem(uint16_t body)
    {
        auto* pBody = m_kConSys.GetBody(body);
        pBody->ReleaseAllParticles();

        m_pCollision->regions[body].enabled = true;
        WakeUp(body);
        return true;
    }

    float ZBoneExtend::Strain(uint16_t body, uint16_t* bone)
    {
        auto* pBody = m_kConSys.GetBody(body);

        uint16_t wPar;
        const float fStrain = pBody->AverageStrain(wPar);

        if (bone)
        {
            *bone = FindBone(wPar);
        }

        return fStrain;
    }

    bool ZBoneExtend::Linkage(uint16_t body, uint16_t bone, const ZBoneExtend* other, uint16_t otherBody, uint16_t otherBone)
    {
        auto* pBody = m_kConSys.GetBody(body);
        auto* pOtherBody = const_cast<ZBoneExtend*>(other)->m_kConSys.GetBody(otherBody);

        const uint16_t wPar = FindPar(bone);
        const uint16_t wOtherPar = const_cast<ZBoneExtend*>(other)->FindPar(otherBone);

        pBody->Linkage(pOtherBody, wPar, wOtherPar);
        return true;
    }

    uint16_t ZBoneExtend::FindNearestParticle(uint16_t body, const ZVector3& pos, const float& maxDist)
    {
        uint16_t wNearest = 0;
        const float fMaxDistSqr = maxDist * maxDist;
        float fBestDist = 9.9999997e37f;

        auto* pBody = m_kConSys.GetBody(body);
        const auto* pMapper = static_cast<const SBoneMapper*>(m_pMapper);

        for (uint16_t i = 0; i < m_wMappings; ++i, ++pMapper)
        {
            float vPos[3];
            float vDelta[3];

            pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vPos), pMapper->m_wPar);
            vsub(vDelta, vPos, pos.Get());

            const float fDist = vlen2(vDelta);

            if ((fMaxDistSqr == 0.0f || fDist <= fMaxDistSqr) && fDist < fBestDist)
            {
                fBestDist = fDist;
                wNearest = pMapper->m_wPar;
            }
        }

        return wNearest;
    }

    uint16_t ZBoneExtend::FindBone(uint16_t par)
    {
        const auto* pMapper = static_cast<const SBoneMapper*>(m_pMapper);

        for (uint16_t i = 0; i < m_wMappings; ++i, ++pMapper)
        {
            if (pMapper->m_wPar == par)
            {
                return pMapper->m_wBone;
            }
        }

        return 0;
    }

    uint16_t ZBoneExtend::FindPar(uint16_t bone)
    {
        const auto* pMapper = static_cast<const SBoneMapper*>(m_pMapper);

        for (uint16_t i = 0; i < m_wMappings; ++i, ++pMapper)
        {
            if (pMapper->m_wBone == bone)
            {
                return pMapper->m_wPar;
            }
        }

        return 0;
    }

    bool ZBoneExtend::ObstacleCollision(uint16_t body, const ZGEOM* geom, const ZVector3& size)
    {
        SBoneCollision::SRegions* pRegion = &m_pCollision->regions[body];

        if (!pRegion->enabled)
        {
            return false;
        }

        if (!FindCollisionRegion(body))
        {
            return false;
        }

        ZVector3 vGeomCen;
        geom->GetCen(vGeomCen);
        geom->GetRootPoint(vGeomCen);

        const float fRegionExtent = (std::max)((std::max)(pRegion->size.z, pRegion->size.y), pRegion->size.x);
        const float fObstacleExtent = (std::max)((std::max)(size.z, size.y), size.x);
        const float fExtentSum = fRegionExtent + fObstacleExtent;

        float vDelta[3];
        vsub(vDelta, pRegion->center.Get(), vGeomCen.Get());

        if (vlen2(vDelta) - fExtentSum * fExtentSum > 0.0f)
        {
            return false;
        }

        // TODO: Finish this place after CapsuleSphereCollision will be reversed
        // Expected logic from PS2 (0x39BBE0):
        // - pick dominant axis of size: x when size.x > size.y && size.x > size.z, z when size.z >= size.y, y otherwise
        // - capsule radius = (size[(axis + 1) % 3] + size[(axis + 2) % 3]) * 0.5f
        // - capsule endpoints = vGeomCen +/- unitAxis * halfLength
        // - for each mapper particle (skip locked, skip radius < 0):
        //     if (CapsuleSphereCollision(vDisp, vCapsA, vCapsB, &fRadius, vParPos, &fParRadius))
        //     {
        //         pBody->MoveParticle(pMapper->m_wPar, &vDisp, false, false);
        //         bHit = true;
        //     }

        return false;
    }
}
