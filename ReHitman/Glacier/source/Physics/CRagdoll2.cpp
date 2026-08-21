#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SBoneDefinition.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Physics/Fysix/ConstrainedParticleSystem.h>
#include <Glacier/Physics/Fysix/ZWaterBoxManager.h>
#include <Glacier/Physics/Fysix/ZWaterBox.h>
#include <Glacier/Physics/ZRagdollContainer.h>
#include <Glacier/Physics/CRagdoll2.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Animation/Model.h>


namespace Glacier
{
    CRagdoll2::CRagdoll2(bool bRagdoll)
    {
        m_nFixedPars = 0;
        m_pParticles = nullptr;
        m_bRagdoll = bRagdoll;
        m_fLastDeltaTime = 0.0f;
        m_lNumBones = 0;
        m_bDragdoll = false;
        m_bActive = false;
        m_fStartTime = {};
        m_pLnkObj = nullptr;
    }

    CRagdoll2::~CRagdoll2()
    {
        if (m_pParticles)
        {
            ZUniMemory::Delete(m_pParticles);
        }

        m_pParticles = nullptr;
    }

    CRagdoll2::ParticleProperty::ParticleProperty()
    {
        timeout = 0;
        fixed = false;
        fixed_pos[0] = 0.0f;
        fixed_pos[1] = 0.0f;
        fixed_pos[2] = 0.0f;
    }

    void CRagdoll2::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        auto rLnkObj = m_pLnkObj->GetRef();

        stream.Exchange("rLnkObj", rLnkObj);
        stream.ExchangeArray("m_vGroundToPelvis", m_vGroundToPelvis, 3);
        stream.Exchange("m_bActive", m_bActive);

        if (!bSaving)
        {
            auto* pLnkObj = ref_cast<ZLNKOBJ>(rLnkObj);
            Setup(pLnkObj);
        }

        m_pParticles->LoadSave(stream, bSaving);

        if (!bSaving)
        {
            const auto* pBones = m_pLnkObj->GetBoneModifier()->GetBones(m_pLnkObj);
            Activate(pBones, false);
        }
    }

    void CRagdoll2::Setup(ZLNKOBJ* pLnkObj)
    {
        if (!pLnkObj)
        {
            return;
        }

        for (int i = 0; i < MAX_PARTICLE_PROPS_NR; ++i)
        {
            new (&m_aParticleProperty[i]) ParticleProperty();
        }

        m_nFixedPars = 0;
        m_pJesusLocalBones = reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetLocalPrimBones(pLnkObj->Prim()));
        m_pLnkObj = pLnkObj;
        m_fStartTime = g_pSysInterface->FrameTime;

        InitIndices();
        CreateParticles();

        m_ColiInfo.bInWater = false;
        m_ColiInfo.bCollision = false;
    }

    void CRagdoll2::InitIndices()
    {
        m_nPelvisIx = GetBoneIndex(eBoneID::PELVIS);
        m_nSpineIx = GetBoneIndex(eBoneID::SPINE);
        m_nSpine1Ix = GetBoneIndex(eBoneID::SPINE_1);
        m_nNeckIx = GetBoneIndex(eBoneID::NECK);
        m_nHeadIx = GetBoneIndex(eBoneID::HEAD);
        m_nLClavicleIx = GetBoneIndex(eBoneID::LEFT_CLAVICLE);
        m_nLUpperArmIx = GetBoneIndex(eBoneID::LEFT_UPPER_ARM);
        m_nLLowerArmIx = GetBoneIndex(eBoneID::LEFT_FOREARM);
        m_nLHandIx = GetBoneIndex(eBoneID::LEFT_HAND);
        m_nRClavicleIx = GetBoneIndex(eBoneID::RIGHT_CLAVICLE);
        m_nRUpperArmIx = GetBoneIndex(eBoneID::RIGHT_UPPER_ARM);
        m_nRLowerArmIx = GetBoneIndex(eBoneID::RIGHT_FOREARM);
        m_nRHandIx = GetBoneIndex(eBoneID::RIGHT_HAND);
        m_nLThighIx = GetBoneIndex(eBoneID::LEFT_THIGH);
        m_nLCalfIx = GetBoneIndex(eBoneID::LEFT_CALF);
        m_nLFootIx = GetBoneIndex(eBoneID::LEFT_FOOT);
        m_nRThighIx = GetBoneIndex(eBoneID::RIGHT_THIGH);
        m_nRCalfIx = GetBoneIndex(eBoneID::RIGHT_CALF);
        m_nRFootIx = GetBoneIndex(eBoneID::RIGHT_FOOT);

        m_nLForeTwistIx = m_pLnkObj->GetBoneNrFromName("LEFT_FORETWIST");
        m_nRForeTwistIx = m_pLnkObj->GetBoneNrFromName("RIGHT_FORETWIST");
    }

    void CRagdoll2::CreateParticles()
    {
        End();
        m_pParticles = ZUniMemory::New<ConstrainedParticleSystem>();
    }

    bool CRagdoll2::IsMoving() const
    {
        if (!m_bActive)
        {
            return false;
        }

        if ((m_ColiInfo.bCollision || m_ColiInfo.bInWater)
            && m_ColiInfo.sPartColi[0].bCollision
            && (!m_bRagdoll || !m_nFixedPars))
        {
            if (m_fLastDeltaTime <= 0.0f)
            {
                return m_ColiInfo.nCollides < m_ColiInfo.sPartColi[0].bImpact + 4;
            }

            ZVector3 vPos;
            m_pLnkObj->GetWorldPosition(vPos);

            ZVector3 vDelta;
            vsub(vDelta, vPos, m_vOldPos);

            if (sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z) / m_fLastDeltaTime <= 10.0f)
            {
                return m_ColiInfo.nCollides < m_ColiInfo.sPartColi[0].bImpact + 4;
            }
        }

        return true;
    }

    void CRagdoll2::PrimChanged(uint32_t lPrim)
    {
        InitIndices();

        m_pJesusLocalBones = reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetLocalPrimBones(lPrim));
    }

    bool CRagdoll2::IsRagdoll() const
    {
        return m_bRagdoll;
    }

    bool CRagdoll2::IsActive() const
    {
        return m_bActive;
    }

    uint16_t CRagdoll2::GetBoneIndex(eBoneID eBone)
    {
        return m_pLnkObj->GetBoneNrFromId(eBone);
    }

    void CRagdoll2::End()
    {
        if (m_pParticles)
        {
            ZUniMemory::Delete(m_pParticles);
        }

        m_pParticles = nullptr;
    }

    bool CRagdoll2::Move(ZMat3x3& mMat, ZVector3& vPos, float fDt)
    {
        if (!IsActive())
            return false;

        const float fFlymo = g_pRenderDll->GetRagdollContainer()->GetFlymoValue();
        if (fFlymo == 0.0f)
            return true;

        if (m_pLnkObj)
            m_pLnkObj->GetWorldPosition(m_vOldPos);

        float fElevatorYDt = 0.0f;
        if (m_pLnkObj && m_pLnkObj->IsInElevator())
            fElevatorYDt = m_pLnkObj->GetElevatorDeltaY();

        // Update particle pos by elevator y dt
        if (std::fabsf(fElevatorYDt) > 0.05f)
        {
            for (int lParticleIdx = 0; lParticleIdx < MAX_PARTICLE_PROPS_NR; ++lParticleIdx)
            {
                // Update particle position
                ZVector3 vParticlePos;
                m_pParticles->GetParticlePos(lParticleIdx, vParticlePos);
                vParticlePos.y += fElevatorYDt;
                m_pParticles->SetParticlePos(lParticleIdx, vParticlePos);

                // Update particle OK pos
                ZVector3 vParticleOkPos;
                m_pParticles->GetParticleOKPos(lParticleIdx, vParticleOkPos);
                vParticleOkPos.y += fElevatorYDt;
                m_pParticles->SetParticleOKPos(lParticleIdx, vParticleOkPos);

                // Update particle old pos
                ZVector3 vParticleOldPos;
                m_pParticles->GetParticleOldPos(lParticleIdx, vParticleOldPos);
                vParticleOldPos.y += fElevatorYDt;
                m_pParticles->SetParticleOldPos(lParticleIdx, vParticleOldPos);
            }
        }

        // Check for bones
        if (m_lNumBones >= 100)
        {
            m_lNumBones -= 100;
            CalcGroundOffset(mMat, vPos);
            return true;
        }

        const float fBoneCount = 0.025f / fDt - 1.0f;
        if (fBoneCount > 0.0f)
        {
            m_lNumBones -= static_cast<int32_t>(fBoneCount * -100.0f);
        }

        const float fTimeMul = g_pSysInterface->GetTimeMultiplier();
        float fTimeStep;
        if (m_nFixedPars || fTimeMul < 1.0f)
        {
            fTimeStep = 0.01f;
            m_lNumBones = 0;
        }
        else
        {
            fTimeStep = 0.025f;
        }

        if (fDt > 0.1f)
            fDt = 0.1f;

        if (fTimeStep < fDt)
            fDt = fTimeStep;

        float fTimeDt = fFlymo * fDt;
        const uint32_t lOldWaterBox = m_ColiInfo.rWaterBox;

        auto* pRagdollContainer = g_pRenderDll->GetRagdollContainer();
        const float fNailGun = pRagdollContainer->GetNailGunValue();

        int lSteps = 1;
        float fRagdollSpeed = 0.0f;
        SRagdollCollisionInfo sRagdollColi = {};
        bool bHasFixedParts = false;

        do
        {
            int nMaxSteps = 0;

            for (int lParticleIdx = 0; lParticleIdx < MAX_PARTICLE_PROPS_NR; ++lParticleIdx)
            {
                auto& rParticleProperty = m_aParticleProperty[lParticleIdx];

                if (rParticleProperty.fixed)
                {
                    bHasFixedParts = true;
                    m_pParticles->m_pParticles[lParticleIdx].mass = 10000.0f;

                    ZVector3 vParticlePos;
                    m_pParticles->GetParticlePos(lParticleIdx, vParticlePos);

                    ZVector3 vOffset;
                    vsub(vOffset, rParticleProperty.fixed_pos, vParticlePos);

                    const float fDist = vlen(vOffset);
                    const float fLimit = fNailGun >= 0.1f ? 50.0f : 20.0f;

                    if (fDist > fLimit)
                    {
                        vscalar(vOffset, fLimit / fDist);

                        const int nSteps = static_cast<int>(std::ceil(fDist / fLimit));
                        if (nMaxSteps < nSteps)
                            nMaxSteps = nSteps;
                    }

                    vadd(vParticlePos, vOffset);
                    m_pParticles->SetParticlePos(lParticleIdx, vParticlePos);
                }
                else
                {
                    m_pParticles->m_pParticles[lParticleIdx].mass = 1.0f;
                }
            }

            if (nMaxSteps && std::fabsf(fTimeDt - fDt) < 0.000001f)
            {
                lSteps = nMaxSteps;
                fTimeDt /= nMaxSteps;
            }

            sRagdollColi = {};
            fRagdollSpeed = m_pParticles->MoveRagdoll(sRagdollColi, fTimeDt, bHasFixedParts);

            --lSteps;
        }
        while (lSteps > 0);

        m_fLastDeltaTime = fTimeDt;
        m_ColiInfo.bCollision = false;
        m_ColiInfo.bWaterImpact = !m_ColiInfo.bInWater && sRagdollColi.bInWater && !m_ColiInfo.bWaterImpact;
        m_ColiInfo.bInWater = sRagdollColi.bInWater;
        m_ColiInfo.nCollides = 0;
        m_ColiInfo.rWaterBox = sRagdollColi.bInWater ? sRagdollColi.rWaterBox : 0;

        for (int lPartColiIdx = 0; lPartColiIdx < 6; ++lPartColiIdx)
        {
            auto& rPartColi = m_ColiInfo.sPartColi[lPartColiIdx];
            const auto& rSrcColi = sRagdollColi.sPartColi[lPartColiIdx];

            const bool bImpact = !rPartColi.bCollision && rSrcColi.bCollision && !rPartColi.bImpact;

            rPartColi.bCollision = rSrcColi.bCollision;
            rPartColi.bImpact = bImpact;
            rPartColi.rGeom = rSrcColi.rGeom;
            rPartColi.vel = rSrcColi.vel;

            if (rPartColi.bCollision)
            {
                ++m_ColiInfo.nCollides;
                m_ColiInfo.bCollision = true;

                switch (lPartColiIdx)
                {
                    case 0: m_pParticles->GetVelocity(5, rPartColi.vel); break;
                    case 1: m_pParticles->GetVelocity(0, rPartColi.vel); break;
                    case 2: m_pParticles->GetVelocity(11, rPartColi.vel); break;
                    case 3: m_pParticles->GetVelocity(10, rPartColi.vel); break;
                    case 4: m_pParticles->GetVelocity(15, rPartColi.vel); break;
                    case 5: m_pParticles->GetVelocity(14, rPartColi.vel); break;
                }
            }
        }

        if (lOldWaterBox != m_ColiInfo.rWaterBox)
        {
            ZWaterBox* pWaterBoxToLeave = ref_cast<ZWaterBox>(lOldWaterBox);
            ZWaterBox* pWaterBoxToEnter = ref_cast<ZWaterBox>(m_ColiInfo.rWaterBox);

            const ZREF rGeom = m_pLnkObj->GetRef();
            if (pWaterBoxToLeave)
                pWaterBoxToLeave->Remove(rGeom);

            if (pWaterBoxToEnter)
                pWaterBoxToEnter->Add(rGeom);
        }

        if (m_ColiInfo.bCollision || m_ColiInfo.bInWater)
        {
            if (m_ColiInfo.bWaterImpact)
            {
                m_nTimeOut = pRagdollContainer->GetTimeOutValue();
                m_nNoMove = 0;
            }

            if (m_nTimeOut > 0)
                --m_nTimeOut;

            m_fWeightedSpeed = m_fWeightedSpeed * 0.9f + fRagdollSpeed;

            if (m_fWeightedSpeed >= m_fThreshold * 50.0f)
                m_nNoMove = 0;
            else
                ++m_nNoMove;

            m_fThreshold = m_fThreshold * 1.002f;

            if (m_fThreshold > 300.0f)
                m_fThreshold = 300.0f;
        }

        ZVector3 vOutPos;
        m_pParticles->GetParticlePos(5, vOutPos);

        if ((m_bDragdoll && (m_nNoMove > 250 || !m_nTimeOut)) || vOutPos.y < -10000.0f)
        {
            m_bActive = false;
            return false;
        }

        CalcGroundOffset(mMat, vPos);
        return true;
    }

    void CRagdoll2::CalcGroundOffset(ZMat3x3& mMat, ZVector3& vPos)
    {
        ZVector3 vDir;

        GetLocalPelvis(vDir, vPos);

        const ZVector3 vWorldUp = { 0.0f, 0.0f, 1.0f };
        const float fInvDt = static_cast<int>(g_pSysInterface->FrameTime - m_fStartTime) * TIMETYPE::kInvTPS;

        if (fInvDt < 1.0f)
        {
            const float fLerp = 1.0f - fInvDt;

            vDir.x = -mMat.data[0] * fLerp + mMat.data[0];
            vDir.y = -mMat.data[1] * fLerp + mMat.data[1];
            vDir.z = (1.0f - mMat.data[2]) * fLerp + mMat.data[2];

            ZVector3 vGroundOffset;
            vmuls(vGroundOffset, m_vGroundToPelvis, fLerp);
            vsub(vPos, vGroundOffset);
        }

        ZVector3 vPulledDir;
        vangpul(vPulledDir, vWorldUp, vDir, g_pSysInterface->DeltaFrameTime * 0.34906587f);
        createmat(mMat.data, vPulledDir, nullptr);

        float fDist = vnorm(vPos);
        const float fMaxSpeed = g_pSysInterface->DeltaFrameTime * 2000.0f;

        if (fDist > fMaxSpeed)
        {
            fDist = fMaxSpeed;
        }

        vscalar(vPos, fDist);
    }

    void CRagdoll2::HandleCalcMatsMsg(ZBone* pBone, bool bConvertToQuat, int nNumBones)
    {
        bool aHandled[256] = {};
        const auto* pParticles = m_pParticles->m_pParticles;

        // Pelvis
        ZVector3 vSpineMid, vPelvisMid, vDir, vRight, vUp;
        vmmul(vSpineMid, pParticles[5].x, pParticles[6].x);
        vscalar(vSpineMid, 0.5f);
        vmmul(vPelvisMid, pParticles[8].x, pParticles[9].x);
        vscalar(vPelvisMid, 0.5f);

        vsub(vDir, vSpineMid, vPelvisMid);
        vnorm(vDir);
        vsub(vRight, pParticles[8].x, pParticles[9].x);
        vnorm(vRight);
        vcross(vUp, vRight, vDir);
        vnorm(vUp);
        vcross(vRight, vDir, vUp);

        ZMat3x3 mPelvis;
        mPelvis.data[0] = vDir.x;
        mPelvis.data[1] = vDir.y;
        mPelvis.data[2] = vDir.z;
        mPelvis.data[3] = vRight.x;
        mPelvis.data[4] = vRight.y;
        mPelvis.data[5] = vRight.z;
        mPelvis.data[6] = vUp.x;
        mPelvis.data[7] = vUp.y;
        mPelvis.data[8] = vUp.z;

        SetLinkMatPosByIx(m_nPelvisIx, mPelvis, vPelvisMid, pBone);
        aHandled[m_nPelvisIx] = true;

        // Spine
        ZVector3 vNeckDir, vSpineRight, vSpineUp;
        vsub(vNeckDir, pParticles[1].x, vSpineMid);
        vnorm(vNeckDir);
        vsub(vSpineRight, pParticles[5].x, pParticles[6].x);
        vnorm(vSpineRight);
        vcross(vSpineUp, vSpineRight, vNeckDir);
        vnorm(vSpineUp);
        vcross(vSpineRight, vNeckDir, vSpineUp);

        ZMat3x3 mSpine;
        mSpine.data[0] = vNeckDir.x;
        mSpine.data[1] = vNeckDir.y;
        mSpine.data[2] = vNeckDir.z;
        mSpine.data[3] = vSpineRight.x;
        mSpine.data[4] = vSpineRight.y;
        mSpine.data[5] = vSpineRight.z;
        mSpine.data[6] = vSpineUp.x;
        mSpine.data[7] = vSpineUp.y;
        mSpine.data[8] = vSpineUp.z;

        SetLinkMatPosByIx(m_nSpineIx, mSpine, vSpineMid, pBone);
        aHandled[m_nSpineIx] = true;

        ZMat3x3 mSpineCopy;
        mcpy(mSpineCopy.data, mSpine.data);

        // Left upper arm
        ZVector3 vLUpperArmDir, vLHandDir, vLUpperArmUp, vLUpperArmRight;
        vsub(vLUpperArmDir, pParticles[7].x, pParticles[3].x);
        vnorm(vLUpperArmDir);
        vsub(vLHandDir, pParticles[11].x, pParticles[7].x);
        vnorm(vLHandDir);

        float fDot = std::fabs(vLHandDir.x * vLUpperArmDir.x + vLHandDir.y * vLUpperArmDir.y + vLHandDir.z * vLUpperArmDir.z);
        vcross(vLUpperArmUp, vLUpperArmDir, vLHandDir);
        vscalar(vLUpperArmUp, 1.0f - fDot);
        vaddscalar(vLUpperArmUp, vLUpperArmUp, mSpineCopy.data, fDot);
        vcross(vLUpperArmRight, vLUpperArmUp, vLUpperArmDir);
        vnorm(vLUpperArmRight);
        vcross(vLUpperArmUp, vLUpperArmDir, vLUpperArmRight);

        ZMat3x3 mLUpperArm;
        mLUpperArm.data[0] = vLUpperArmDir.x;
        mLUpperArm.data[1] = vLUpperArmDir.y;
        mLUpperArm.data[2] = vLUpperArmDir.z;
        mLUpperArm.data[3] = vLUpperArmUp.x;
        mLUpperArm.data[4] = vLUpperArmUp.y;
        mLUpperArm.data[5] = vLUpperArmUp.z;
        mLUpperArm.data[6] = vLUpperArmRight.x;
        mLUpperArm.data[7] = vLUpperArmRight.y;
        mLUpperArm.data[8] = vLUpperArmRight.z;

        SetLinkMatPosByIx(m_nLUpperArmIx, mLUpperArm, pParticles[3].x, pBone);
        aHandled[m_nLUpperArmIx] = true;

        // Left lower arm
        ZVector3 vLLowerArmDir, vLLowerArmAxis;
        vsub(vLLowerArmDir, pParticles[11].x, pParticles[7].x);
        vnorm(vLLowerArmDir);
        vcross(vLLowerArmAxis, vLLowerArmDir, mLUpperArm.data);
        float fAxisLen = vnorm(vLLowerArmAxis);
        float fCosAngle = vLLowerArmDir.x * mLUpperArm.data[0] + vLLowerArmDir.y * mLUpperArm.data[1] + vLLowerArmDir.z * mLUpperArm.data[2];

        ZMat3x3 mLLowerArmRot;
        mrotaxis2(fCosAngle, fAxisLen, vLLowerArmAxis, mLLowerArmRot.data);

        ZMat3x3 mLLowerArm;
        mmmul(mLLowerArm.data, mLUpperArm.data, mLLowerArmRot.data);

        SetLinkMatPosByIx(m_nLLowerArmIx, mLLowerArm, pParticles[7].x, pBone);
        aHandled[m_nLLowerArmIx] = true;

        // Right upper arm
        ZVector3 vRUpperArmDir, vRHandDir, vRUpperArmUp, vRUpperArmRight;
        vsub(vRUpperArmDir, pParticles[4].x, pParticles[2].x);
        vnorm(vRUpperArmDir);
        vsub(vRHandDir, pParticles[10].x, pParticles[4].x);
        vnorm(vRHandDir);

        fDot = std::fabs(vRHandDir.x * vRUpperArmDir.x + vRHandDir.y * vRUpperArmDir.y + vRHandDir.z * vRUpperArmDir.z);
        vcross(vRUpperArmUp, vRUpperArmDir, vRHandDir);
        vscalar(vRUpperArmUp, 1.0f - fDot);
        vaddscalar(vRUpperArmUp, vRUpperArmUp, mSpineCopy.data, -fDot);
        vcross(vRUpperArmRight, vRUpperArmUp, vRUpperArmDir);
        vnorm(vRUpperArmRight);
        vcross(vRUpperArmUp, vRUpperArmDir, vRUpperArmRight);

        ZMat3x3 mRUpperArm;
        mRUpperArm.data[0] = vRUpperArmDir.x;
        mRUpperArm.data[1] = vRUpperArmDir.y;
        mRUpperArm.data[2] = vRUpperArmDir.z;
        mRUpperArm.data[3] = vRUpperArmUp.x;
        mRUpperArm.data[4] = vRUpperArmUp.y;
        mRUpperArm.data[5] = vRUpperArmUp.z;
        mRUpperArm.data[6] = vRUpperArmRight.x;
        mRUpperArm.data[7] = vRUpperArmRight.y;
        mRUpperArm.data[8] = vRUpperArmRight.z;

        SetLinkMatPosByIx(m_nRUpperArmIx, mRUpperArm, pParticles[2].x, pBone);
        aHandled[m_nRUpperArmIx] = true;

        // Right lower arm
        ZVector3 vRLowerArmDir, vRLowerArmAxis;
        vsub(vRLowerArmDir, pParticles[10].x, pParticles[4].x);
        vnorm(vRLowerArmDir);
        vcross(vRLowerArmAxis, vRLowerArmDir, mRUpperArm.data);
        fAxisLen = vnorm(vRLowerArmAxis);
        fCosAngle = vRLowerArmDir.x * mRUpperArm.data[0] + vRLowerArmDir.y * mRUpperArm.data[1] + vRLowerArmDir.z * mRUpperArm.data[2];

        ZMat3x3 mRLowerArmRot;
        mrotaxis2(fCosAngle, fAxisLen, vRLowerArmAxis, mRLowerArmRot.data);

        ZMat3x3 mRLowerArm;
        mmmul(mRLowerArm.data, mRUpperArm.data, mRLowerArmRot.data);

        SetLinkMatPosByIx(m_nRLowerArmIx, mRLowerArm, pParticles[4].x, pBone);
        aHandled[m_nRLowerArmIx] = true;

        // Left thigh
        vscalar(mPelvis.data, -1.0f);
        vscalar(&mPelvis.data[3], -1.0f);
        ZVector3 vLThighDir = { mPelvis.data[0], mPelvis.data[1], mPelvis.data[2] };
        vnorm(vLThighDir);

        ZVector3 vLCalfDir, vLThighAxis;
        vsub(vLCalfDir, pParticles[13].x, pParticles[9].x);
        vnorm(vLCalfDir);
        vcross(vLThighAxis, vLThighDir, vLCalfDir);
        vnorm(vLThighAxis);

        fCosAngle = vLCalfDir.x * vLThighDir.x + vLCalfDir.y * vLThighDir.y + vLCalfDir.z * vLThighDir.z;
        float fSinAngle = 1.0f - fCosAngle * fCosAngle;
        fSinAngle = fSinAngle >= 0.0f ? std::sqrt(fSinAngle) : 0.0f;

        ZMat3x3 mLThighRot;
        mrotaxis2(fCosAngle, fSinAngle, vLThighAxis, mLThighRot.data);

        ZMat3x3 mLThigh;
        mmtmul(mLThigh.data, mPelvis.data, mLThighRot.data);

        SetLinkMatPosByIx(m_nLThighIx, mLThigh, pParticles[9].x, pBone);
        aHandled[m_nLThighIx] = true;

        // Left calf
        ZVector3 vLFootDir;
        vsub(vLFootDir, pParticles[15].x, pParticles[13].x);
        vnorm(vLFootDir);

        float fScale = (vLFootDir.x * mLThigh.data[0] + vLFootDir.y * mLThigh.data[1] + vLFootDir.z * mLThigh.data[2]) * fCosAngle;
        ZVector3 vLCalfUp;
        vscalar(vLCalfUp, &mLThigh.data[6], fScale);
        vaddscalar(vLCalfUp, vLCalfUp, mLThigh.data, 1.0f - fScale);

        ZVector3 vLCalfRight;
        vcross(vLCalfRight, vLFootDir, vLCalfUp);
        vnorm(vLCalfRight);
        vcross(vLCalfUp, vLCalfRight, vLFootDir);

        ZMat3x3 mLCalf;
        mLCalf.data[0] = vLFootDir.x;
        mLCalf.data[1] = vLFootDir.y;
        mLCalf.data[2] = vLFootDir.z;
        mLCalf.data[3] = vLCalfUp.x;
        mLCalf.data[4] = vLCalfUp.y;
        mLCalf.data[5] = vLCalfUp.z;
        mLCalf.data[6] = vLCalfRight.x;
        mLCalf.data[7] = vLCalfRight.y;
        mLCalf.data[8] = vLCalfRight.z;

        SetLinkMatPosByIx(m_nLCalfIx, mLCalf, pParticles[13].x, pBone);
        aHandled[m_nLCalfIx] = true;

        // Right thigh
        ZVector3 vRThighDir = { mPelvis.data[0], mPelvis.data[1], mPelvis.data[2] };
        vnorm(vRThighDir);

        ZVector3 vRCalfDir, vRThighAxis;
        vsub(vRCalfDir, pParticles[12].x, pParticles[8].x);
        vnorm(vRCalfDir);
        vcross(vRThighAxis, vRThighDir, vRCalfDir);
        vnorm(vRThighAxis);

        fCosAngle = vRCalfDir.x * vRThighDir.x + vRCalfDir.y * vRThighDir.y + vRCalfDir.z * vRThighDir.z;
        fSinAngle = 1.0f - fCosAngle * fCosAngle;
        fSinAngle = fSinAngle >= 0.0f ? std::sqrt(fSinAngle) : 0.0f;

        ZMat3x3 mRThighRot;
        mrotaxis2(fCosAngle, fSinAngle, vRThighAxis, mRThighRot.data);

        ZMat3x3 mRThigh;
        mmtmul(mRThigh.data, mPelvis.data, mRThighRot.data);

        SetLinkMatPosByIx(m_nRThighIx, mRThigh, pParticles[8].x, pBone);
        aHandled[m_nRThighIx] = true;

        // Right calf
        ZVector3 vRFootDir;
        vsub(vRFootDir, pParticles[14].x, pParticles[12].x);
        vnorm(vRFootDir);

        fScale = (vRFootDir.x * mRThigh.data[0] + vRFootDir.y * mRThigh.data[1] + vRFootDir.z * mRThigh.data[2]);
        fScale = fScale * fScale;
        ZVector3 vRCalfUp;
        vscalar(vRCalfUp, &mRThigh.data[6], fScale);
        vaddscalar(vRCalfUp, vRCalfUp, mRThigh.data, 1.0f - fScale);

        ZVector3 vRCalfRight;
        vcross(vRCalfRight, vRFootDir, vRCalfUp);
        vnorm(vRCalfRight);
        vcross(vRCalfUp, vRCalfRight, vRFootDir);

        ZMat3x3 mRCalf;
        mRCalf.data[0] = vRFootDir.x;
        mRCalf.data[1] = vRFootDir.y;
        mRCalf.data[2] = vRFootDir.z;
        mRCalf.data[3] = vRCalfUp.x;
        mRCalf.data[4] = vRCalfUp.y;
        mRCalf.data[5] = vRCalfUp.z;
        mRCalf.data[6] = vRCalfRight.x;
        mRCalf.data[7] = vRCalfRight.y;
        mRCalf.data[8] = vRCalfRight.z;

        SetLinkMatPosByIx(m_nRCalfIx, mRCalf, pParticles[12].x, pBone);
        aHandled[m_nRCalfIx] = true;

        // Fore twists
        if (m_nLForeTwistIx < nNumBones)
        {
            CopyBone(m_nLForeTwistIx, m_nLLowerArmIx, pBone);
            aHandled[m_nLForeTwistIx] = true;
        }

        if (m_nRForeTwistIx < nNumBones)
        {
            CopyBone(m_nRForeTwistIx, m_nRLowerArmIx, pBone);
            aHandled[m_nRForeTwistIx] = true;
        }

        if (bConvertToQuat)
        {
            const auto* pBoneDefs = ZPrimControlBase::Instance()->GetBoneDefinitions(LnkObj()->Prim());
            const auto* pAngelPose = m_pLnkObj->m_Model->m_AngelPose;

            mreset(pBone->_Mat.data);
            pBone->_Pos.x = 0.0f;
            pBone->_Pos.y = 0.0f;
            pBone->_Pos.z = 0.0f;

            int nMaxBones = nNumBones <= 30 ? nNumBones : 30;
            for (int i = 1; i < nMaxBones; ++i)
            {
                if (!aHandled[i])
                {
                    LinkMats(i, pBone);
                    aHandled[i] = true;
                }
            }

            for (int i = nNumBones - 1; i > 0; --i)
            {
                if (aHandled[i])
                {
                    uint32_t lPrev = pBoneDefs[i].lPrevBoneNr;
                    if (!aHandled[lPrev] && lPrev != 0)
                    {
                        ZASSERT(false);
                    }

                    ZMat3x3 mTmp;
                    tmat(mTmp.data, pBone[lPrev]._Mat.data);
                    mmmul(pBone[i]._Mat.data, mTmp.data);
                    mattoquat(&pBone[i]._Mat.data[0], &pBone[i]._Mat.data[0]);
                }
                else
                {
                    const float* pSrc = reinterpret_cast<const float*>(&pAngelPose[i]);
                    pBone[i]._Mat.data[0] = pSrc[0];
                    pBone[i]._Mat.data[1] = pSrc[1];
                    pBone[i]._Mat.data[2] = pSrc[2];
                    pBone[i]._Mat.data[3] = pSrc[3];
                }

                if (i > 1)
                {
                    const float* pSrc = reinterpret_cast<const float*>(&pAngelPose[i]);
                    pBone[i]._Pos.x = pSrc[4];
                    pBone[i]._Pos.y = pSrc[5];
                    pBone[i]._Pos.z = pSrc[6];
                }
            }
        }
        else
        {
            for (int i = 1; i < nNumBones; ++i)
            {
                if (!aHandled[i])
                {
                    LinkMats(i, pBone);
                }
            }
        }
    }

    void CRagdoll2::SetLinkMatPosByIx(int lIndex, const ZMat3x3& mMat, const ZVector3& vPos, ZBone* pBones)
    {
        const auto* pConvBones = ZPrimControlBase::Instance()->GetConvBones(LnkObj()->Prim());
        auto* pBone = &pBones[lIndex];

        pBone->_Pos = vPos;
        mcpy(pBone->_Mat.data, reinterpret_cast<const float*>(pConvBones) + 9 * lIndex);
        mmmul(pBone->_Mat.data, mMat.data);
        m_pLnkObj->GetLocalMatPos(pBone->_Mat, pBone->_Pos);
    }

    void CRagdoll2::LinkMats(int lBoneIndex, ZBone* pBones)
    {
        if (lBoneIndex != -1)
        {
            const auto lPrevBoneIndex = ZPrimControlBase::Instance()->GetBoneDefinitions(LnkObj()->Prim())[lBoneIndex].lPrevBoneNr;

            auto* pBone = &pBones[lBoneIndex];

            if (lPrevBoneIndex != -1)
            {
                const auto* pPrevBone = &pBone[lPrevBoneIndex];
                const auto* pJesusBone = &m_pJesusLocalBones[lBoneIndex];

                mmmul(pBone->_Mat, pJesusBone->_Mat, pPrevBone->_Mat);
                vmmul(pBone->_Pos, pJesusBone->_Pos, pPrevBone->_Mat);
                vadd(pBone->_Pos, pPrevBone->_Pos);
            }
        }
    }

    const ZLNKOBJ* CRagdoll2::LnkObj() const
    {
        return m_pLnkObj;
    }

    void CRagdoll2::CopyBone(int lDstBoneIdx, int lSrcBoneIdx, ZBone* pBones)
    {
        if (lDstBoneIdx != -1 && lSrcBoneIdx != -1)
        {
            const auto* pSrcBone = &pBones[lSrcBoneIdx];
            auto* pDstBone = &pBones[lDstBoneIdx];

            *pDstBone = *pSrcBone;
        }
    }

    void CRagdoll2::EnableTimeOut(bool bEnable)
    {
        const auto lTimeOut = g_pRenderDll->GetRagdollContainer()->GetTimeOutValue();

        if (bEnable && lTimeOut != 0)
        {
            if (!m_bDragdoll)
            {
                m_nTimeOut = lTimeOut;
                m_fThreshold = 50.0f;
                m_fWeightedSpeed = 2500.0f;
                m_nNoMove = 0;
            }

            m_bDragdoll = true;
        }
        else
        {
            m_bDragdoll = false;
        }
    }

    void CRagdoll2::Activate(const ZBone* pBones, bool bActivate)
    {
        if (bActivate)
        {
            ComputeParticlePositionsSub(pBones, true);
        }

        m_pParticles->ZeroOut(5);
        m_pParticles->FindFaces();

        if (!bActivate)
        {
            m_pParticles->HandleCollision(m_ColiInfo);
            m_pParticles->ResetVelocities();
        }

        m_pParticles->InitOkX();

        ZVector3 vPelvisDir;
        GetLocalPelvis(vPelvisDir, m_vGroundToPelvis);

        auto* pContainer = g_pRenderDll->GetRagdollContainer();
        m_nTimeOut = pContainer->GetTimeOutValue();
        m_bDragdoll = m_nTimeOut > 0;
        m_fThreshold = 50.0f;
        m_fWeightedSpeed = 2500.0f;
        m_nNoMove = 0;
        m_bActive = true;
        m_pLnkObj->GetWorldPosition(m_vOldPos);
        m_lNumBones = 0;
    }

    void CRagdoll2::Deactivate()
    {
        m_bActive = false;
    }

    void CRagdoll2::SetDamping(float fDamping)
    {
        m_pParticles->m_fDamping = std::clamp(fDamping, 0.0f, 1.0f);
    }

    void CRagdoll2::GetLocalPelvis(ZVector3& vDir, ZVector3& vPos)
    {
        ZVector3 pt8, pt9, pt14, pt15;

        // Get particle positions
        m_pParticles->GetParticlePos(8, pt8);
        m_pParticles->GetParticlePos(9, pt9);
        m_pParticles->GetParticlePos(14, pt14);
        m_pParticles->GetParticlePos(15, pt15);

        // Find mid points
        ZVector3 vDiffPt8Pt9 = pt9 - pt8;
        ZVector3 vHipMid     = (pt8 + pt9) * 0.5f;   // Бывший vPos до LERP
        ZVector3 vChestMid   = (pt14 + pt15) * 0.5f; // Бывший outPos

        // Calculate orientation
        ZVector3 vTorsoVec = vChestMid - vHipMid;
        ZVector3 vCross;
        vcross(vCross, vDiffPt8Pt9, vTorsoVec);

        // Make small lerp
        vPos = vHipMid + (vChestMid - vHipMid) * 0.1f;


        // Find base dir vector
        vsub(vDir, vPos, vChestMid); // vDir = vPos - vChestMid
        vDir.y = 0.0f;               // Drop height

        if (vCross.y < 0.0f)
        {
            vneg(vDir);
        }

        // Compute fade factor
        float fFactor = 1.0f;
        float fDist   = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);

        if (fDist > 20.0f)
        {
            if (fDist >= 40.0f)
                fFactor = 0.0f;
            else
                fFactor = 1.0f - (fDist - 20.0f) * 0.05f;
        }

        // Convert to local space
        m_pLnkObj->GetLocalPoint(vPos);
        m_pLnkObj->GetRootPoint(vDir);

        // Apply coefs to direction
        vDir.x = vDir.x * (1.0f - fFactor);
        vDir.y = vDir.y * (1.0f - fFactor);
        vDir.z = (1.0f - vDir.z) * fFactor + vDir.z;
    }

    void CRagdoll2::ComputeParticlePositionsSub(const ZBone* pBones, bool bCalcVelocity)
    {
        struct ZHelper
        {
            static void SetStuff(ConstrainedParticleSystem* pSys, uint16_t lParticleIndex, const ZVector3& vPos, bool bCalcVelocity)
            {
                if (bCalcVelocity)
                {
                    ZVector3 vOldPos;
                    pSys->GetParticleOldPos(lParticleIndex, vOldPos);

                    ZVector3 vVel;
                    vsub(vVel, vPos, vOldPos);

                    float fDeltaTime = g_pSysInterface->DeltaFrameTime;
                    if (fDeltaTime <= 0.0f)
                    {
                        fDeltaTime = 1.0f;
                    }

                    vscalar(vVel, 0.02f / fDeltaTime);
                    pSys->SetParticleVel(lParticleIndex, vVel);
                }
                else
                {
                    static const ZVector3 zero = { 0.0f, 0.0f, 0.0f };

                    pSys->SetParticlePos(lParticleIndex, vPos);
                    pSys->SetParticleOldPos(lParticleIndex, vPos);
                    pSys->SetParticleVel(lParticleIndex, zero);
                }
            }
        };

        ZMat3x3 mMat;
        ZVector3 vPos, vCen, vSize;

        if (GetLinkMatPos_global(m_nHeadIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            vCen.Reset();
            TransformRootVector(vCen, mMat);
            vadd(vPos, vCen);

            ZHelper::SetStuff(m_pParticles, 0, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nNeckIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            vCen.Reset();
            TransformRootVector(vCen, mMat);
            vadd(vPos, vCen);

            ZHelper::SetStuff(m_pParticles, 1, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nRUpperArmIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 2, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nLUpperArmIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 3, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nRLowerArmIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 4, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nLLowerArmIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 7, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nRHandIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 10, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nLHandIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 11, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nSpineIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            if (vSize.y < 10.0f)
            {
                vSize.y = 10.0f;
            }

            vCen.x = 0.0f;
            vCen.y = -(vSize.y * 0.8f);
            vCen.z = 0.0f;

            TransformRootVector(vCen, mMat);
            vadd(vPos, vCen);

            ZHelper::SetStuff(m_pParticles, 5, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nSpineIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            if (vSize.y < 10.0f)
            {
                vSize.y = 10.0f;
            }

            vCen.x = 0.0f;
            vCen.y = vSize.y * 0.8f;
            vCen.z = 0.0f;

            TransformRootVector(vCen, mMat);
            vadd(vPos, vCen);

            ZHelper::SetStuff(m_pParticles, 6, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nRThighIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 8, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nLThighIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 9, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nRCalfIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 12, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nLCalfIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 13, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nRFootIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 14, vPos, bCalcVelocity);
        }

        if (GetLinkMatPos_global(m_nLFootIx, mMat, vPos, &vCen, &vSize, pBones))
        {
            ZHelper::SetStuff(m_pParticles, 15, vPos, bCalcVelocity);
        }
    }

    bool CRagdoll2::GetLinkMatPos_global(uint16_t lBoneIndex, ZMat3x3& mMat, ZVector3& vPos, ZVector3* vCenter, ZVector3* vSize, const ZBone* pBones)
    {
        const auto* pBoneDef = &m_pLnkObj->GetBoneDefinitions()[lBoneIndex];

        if (vCenter)
        {
            *vCenter = pBoneDef->Center;
        }

        if (vSize)
        {
            *vSize = pBoneDef->Size;
        }

        const auto* pBone = &pBones[lBoneIndex];
        mMat = pBone->_Mat;
        vPos = pBone->_Pos;

        m_pLnkObj->GetRootMatPos(mMat, vPos);
        return true;
    }
}
