#include <Glacier/Physics/ZCollisionBox.h>
#include <Glacier/Physics/ZCollisionBase.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <cmath>
#include <cstring>


namespace Glacier
{
    // ZCollisionBox

    ZCollisionBox::ZCollisionBox()
    {
        m_bPushBoxEnabled = false;
        m_fFrameTime = TIMETYPE(0);
        m_fDeltaFrameTime = 0.0f;
        m_pMemBuffer = nullptr;
        m_lMemBufferSize = 0;
        m_bLocked = false;
    }

    void ZCollisionBox::SetMemBuffer(char* pBuffer, uint32_t uiSize)
    {
        m_pMemBuffer = pBuffer;
        m_lMemBufferSize = uiSize;
    }

    void ZCollisionBox::Lock()
    {
        ZASSERT(!m_bLocked);
        m_bLocked = true;
        m_bPushBoxEnabled = false;
    }

    void ZCollisionBox::Unlock()
    {
        ZASSERT(m_bLocked);
        m_bLocked = false;
    }

    bool ZCollisionBox::IsLocked() const
    {
        return m_bLocked;
    }

    void ZCollisionBox::SetPushBox(const float* pfMat, const float* pfPos, const float* pfDimensions)
    {
        m_bPushBoxEnabled = true;
        vcpy(m_vPushBoxDimensions.Get(), pfDimensions);
        GetLocalPoint(m_vPushBoxPosition.Get(), pfPos);
        GetLocalVect(&m_mPushBoxMatrix.data[6], pfMat + 6);
        GetLocalVect(&m_mPushBoxMatrix.data[3], pfMat + 3);
        GetLocalVect(&m_mPushBoxMatrix.data[0], pfMat + 0);
    }

    void ZCollisionBox::SetBox(const float* pfMat, const float* pfPos, const float* pfDimensions)
    {
        ZASSERT(vlen(pfPos) < 1e6f);
        mcpy(m_mBoxMatrix.data, pfMat);
        vcpy(m_vBoxPosition.Get(), pfPos);
        vcpy(m_vBoxDimensions.Get(), pfDimensions);
    }

    void ZCollisionBox::SetGravity(const float* pfGravity)
    {
        // TODO: Finish me
        // vcpy(m_vGravity.Get(), pfGravity);
    }

    void ZCollisionBox::OverrideFrameTime(TIMETYPE ttFrameTime, float fDeltaTime)
    {
        m_fFrameTime = ttFrameTime;
        m_fDeltaFrameTime = fDeltaTime;
    }

    float ZCollisionBox::DeltaTime() const
    {
        if (m_fDeltaFrameTime == 0.0f)
            return g_pSysInterface->DeltaFrameTime;
        return m_fDeltaFrameTime;
    }

    void ZCollisionBox::GetStrips(uint32_t uiFlags)
    {
        ZCollisionBase::GetCollisionInterface()->GetStripsInsideBox(
            &m_lNrStrips,
            m_pMemBuffer,
            m_lMemBufferSize,
            m_mBoxMatrix,
            m_vBoxPosition,
            m_vBoxDimensions,
            uiFlags,
            true,
            true,
            false,
            GT_StdObjs);
    }

    uint32_t ZCollisionBox::StripCount() const
    {
        return m_lNrStrips;
    }

    bool ZCollisionBox::PushSphereOut(float* pfSpherePos, float* pfSphereRadius) const
    {
        if (!m_bPushBoxEnabled)
            return false;

        GetLocalPoint(pfSpherePos);
        const bool bResult = PushLocalSphereOut(pfSpherePos, pfSphereRadius);
        GetRootPoint(pfSpherePos);
        return bResult;
    }

    bool ZCollisionBox::PushLocalSphereOut(float* pfSpherePos, const float* pfSphereRadius) const
    {
        if (!m_bPushBoxEnabled)
            return false;

        float vLocalPos[3];
        float vMaxDist[3];

        vsub(vLocalPos, pfSpherePos, m_vPushBoxPosition.Get());
        vmtmul(vLocalPos, m_mPushBoxMatrix.data);
        vadd(vMaxDist, pfSphereRadius, m_vPushBoxDimensions.Get());

        if (std::fabs(vLocalPos[0]) >= vMaxDist[0])
            return false;
        if (std::fabs(vLocalPos[1]) >= vMaxDist[1])
            return false;
        if (std::fabs(vLocalPos[2]) >= vMaxDist[2])
            return false;

        float fPenX = vMaxDist[0] - vLocalPos[0];
        if (vLocalPos[0] < 0.0f)
            fPenX = -vMaxDist[0] - vLocalPos[0];

        float fPenZ = vMaxDist[2] - vLocalPos[2];
        if (vLocalPos[2] < 0.0f)
            fPenZ = -vMaxDist[2] - vLocalPos[2];

        if (std::fabs(fPenX) >= std::fabs(fPenZ))
        {
            fPenZ += (fPenZ >= 0.0f) ? 0.01f : -0.01f;
            vLocalPos[2] += fPenZ;
        }
        else
        {
            fPenX += (fPenX >= 0.0f) ? 0.01f : -0.01f;
            vLocalPos[0] += fPenX;
        }

        vmmul(pfSpherePos, vLocalPos, m_mPushBoxMatrix.data);
        vadd(pfSpherePos, m_vPushBoxPosition.Get());
        return true;
    }

    bool ZCollisionBox::CheckBoxCollision(const float* pfMat, const float* pfPos, const float* pfSize) const
    {
        float vLocalMat[9];
        float vLocalPos[3];
        float vLocalSize[3];

        mmtmul(vLocalMat, pfMat, m_mBoxMatrix.data);
        GetLocalPoint(vLocalPos, pfPos);
        vcpy(vLocalSize, pfSize);
        return CheckSphereCollisionLocal(vLocalMat, vLocalPos, vLocalSize);
    }

    bool ZCollisionBox::CheckSphereCollision(const float* pfMat, const float* pfPos, const float* pfRadius) const
    {
        float vLocalMat[9];
        float vLocalPos[3];
        float vLocalRadius[3];

        mmtmul(vLocalMat, pfMat, m_mBoxMatrix.data);
        GetLocalPoint(vLocalPos, pfPos);
        vcpy(vLocalRadius, pfRadius);
        return CheckSphereCollisionLocal(vLocalMat, vLocalPos, vLocalRadius);
    }

    bool ZCollisionBox::CheckSphereCollisionLocal(const float* pfInvMat, const float* pfPos, const float* pfRadius) const
    {
        float sphereMatrix[9]{};
        if (pfRadius[0] != 0.0f)
            vscalar(sphereMatrix + 6, pfInvMat + 6, 1.0f / pfRadius[0]);
        if (pfRadius[1] != 0.0f)
            vscalar(sphereMatrix + 3, pfInvMat + 3, 1.0f / pfRadius[1]);
        if (pfRadius[2] != 0.0f)
            vscalar(sphereMatrix, pfInvMat, 1.0f / pfRadius[2]);

        const auto* strip = reinterpret_cast<const ZRawStrip*>(m_pMemBuffer);
        for (uint32_t i = 0; i < m_lNrStrips; ++i)
        {
            uint32_t length = strip->m_lLength;
            const float* vertices = reinterpret_cast<const float*>(strip + 1);
            if ((length & 0x80000000u) != 0)
            {
                length &= 0x7FFFFFFFu;
                for (uint32_t j = 0; j < length; ++j, vertices += 9)
                {
                    if (ZCommonAlgorithms::PolySphColl(pfPos, sphereMatrix, vertices, vertices + 3, vertices + 6))
                        return true;
                }
            }
            else
            {
                for (uint32_t j = 0; j + 2 < length; ++j, vertices += 3)
                {
                    if (ZCommonAlgorithms::PolySphColl(pfPos, sphereMatrix, vertices, vertices + 3, vertices + 6))
                        return true;
                }
                vertices += 6;
            }
            strip = reinterpret_cast<const ZRawStrip*>(vertices);
        }
        return false;
    }

    bool ZCollisionBox::CalcLineCollision(SExtendedImpactInfo* pImpactInfo, const float* pfFrom, const float* pfDir, bool bBothSides) const
    {
        ZVector3 from;
        ZVector3 direction;
        GetLocalPoint(from, pfFrom);
        GetLocalVect(direction, pfDir);
        return CalcLineCollisionLocal(pImpactInfo, from, direction, bBothSides);
    }

    bool ZCollisionBox::CalcLineCollisionLocal(SExtendedImpactInfo* pImpactInfo, const float* pfFrom, const float* pfDir, bool bBothSides) const
    {
        bool hit = false;
        const auto* strip = reinterpret_cast<const ZRawStrip*>(m_pMemBuffer);
        for (uint32_t i = 0; i < m_lNrStrips; ++i)
        {
            uint32_t length = strip->m_lLength;
            const float* vertices = reinterpret_cast<const float*>(strip + 1);
            const uint32_t triangleCount = (length & 0x80000000u) ? (length & 0x7FFFFFFFu) : (length > 2 ? length - 2 : 0);
            for (uint32_t triangle = 0; triangle < triangleCount; ++triangle)
            {
                const float* a = vertices;
                const float* b = vertices + 3;
                const float* c = vertices + 6;
                ZVector3 edge1 = ZVector3(b) - ZVector3(a);
                ZVector3 edge2 = ZVector3(c) - ZVector3(a);
                ZVector3 normal;
                vcross(normal, edge1, edge2);
                const float denominator = vdot(normal, pfDir);
                if ((bBothSides || denominator < 0.0f) && std::fabs(denominator) > 0.000001f)
                {
                    const float t = vdot(normal, ZVector3(a) - ZVector3(pfFrom)) / denominator;
                    if (t >= 0.0f && t <= pImpactInfo->fPercent)
                    {
                        ZVector3 point = ZVector3(pfFrom) + ZVector3(pfDir) * t;
                        ZVector3 c0, c1, c2;
                        vcross(c0, ZVector3(b) - ZVector3(a), point - ZVector3(a));
                        vcross(c1, ZVector3(c) - ZVector3(b), point - ZVector3(b));
                        vcross(c2, ZVector3(a) - ZVector3(c), point - ZVector3(c));
                        if (vdot(c0, normal) >= 0.0f && vdot(c1, normal) >= 0.0f && vdot(c2, normal) >= 0.0f)
                        {
                            pImpactInfo->fPercent = t;
                            pImpactInfo->vPosition = point;
                            pImpactInfo->vP1 = a;
                            pImpactInfo->vP2 = b;
                            pImpactInfo->vP3 = c;
                            pImpactInfo->lTriangleNr = strip->m_lTriangleStartNr + triangle;
                            pImpactInfo->m_HitCache = strip->m_HitCache;
                            pImpactInfo->pBaseGeom = strip->m_pBaseGeom;
                            hit = true;
                        }
                    }
                }
                vertices += (length & 0x80000000u) ? 9 : 3;
            }
            if ((length & 0x80000000u) == 0)
                vertices += 6;
            strip = reinterpret_cast<const ZRawStrip*>(vertices);
        }
        return hit;
    }

    void ZCollisionBox::AddActiveImpact(ZSphereImpact* pSphereImpact, ZRawStrip* pStrip, uint32_t uiImpactType, uint32_t uiTriangleNr) const
    {
        ZActiveImpactPrim* pImpact = &pSphereImpact->m_ActiveImpacts[0];
        if (pSphereImpact->m_lNrActiveImpacts != 3)
        {
            pImpact = &pSphereImpact->m_ActiveImpacts[pSphereImpact->m_lNrActiveImpacts++];
        }

        std::memset(pImpact, 0, sizeof(ZActiveImpactPrim));
        pImpact->m_pBaseGeom = pStrip->m_pBaseGeom;
        pImpact->m_lType = uiImpactType;
        pImpact->m_StripId = pStrip->m_HitCache;
        pImpact->m_lImpactTriangleNr = pStrip->m_lTriangleStartNr + uiTriangleNr;
        pImpact->m_bInvalid = false;

        const float* pfVerts = reinterpret_cast<const float*>(pStrip + 1);
        if (pStrip->m_lLength & 0x80000000)
            pfVerts += uiTriangleNr * 9; // triangle fan
        else
            pfVerts += uiTriangleNr * 3; // triangle strip

        vcpy(pImpact->m_vCorner1.Get(), pfVerts + 0);
        vcpy(pImpact->m_vCorner2.Get(), pfVerts + 3);
        vcpy(pImpact->m_vCorner3.Get(), pfVerts + 6);

        switch (uiImpactType)
        {
        case 0x01:
            ZASSERT(!vcmp(pImpact->m_vCorner1.Get(), pImpact->m_vCorner2.Get()));
            break;
        case 0x02:
            vcpy(pImpact->m_vCorner1.Get(), pImpact->m_vCorner2.Get());
            vcpy(pImpact->m_vCorner2.Get(), pImpact->m_vCorner3.Get());
            ZASSERT(!vcmp(pImpact->m_vCorner1.Get(), pImpact->m_vCorner2.Get()));
            break;
        case 0x04:
            vcpy(pImpact->m_vCorner2.Get(), pImpact->m_vCorner1.Get());
            vcpy(pImpact->m_vCorner1.Get(), pImpact->m_vCorner3.Get());
            ZASSERT(!vcmp(pImpact->m_vCorner1.Get(), pImpact->m_vCorner2.Get()));
            break;
        case 0x10:
            vcpy(pImpact->m_vCorner1.Get(), pImpact->m_vCorner2.Get());
            break;
        case 0x20:
            vcpy(pImpact->m_vCorner1.Get(), pImpact->m_vCorner3.Get());
            break;
        case 0x40:
            ZASSERT(!vcmp(pImpact->m_vCorner1.Get(), pImpact->m_vCorner2.Get()));
            ZASSERT(!vcmp(pImpact->m_vCorner1.Get(), pImpact->m_vCorner3.Get()));
            ZASSERT(!vcmp(pImpact->m_vCorner2.Get(), pImpact->m_vCorner3.Get()));
            break;
        default:
            break;
        }
    }

    void ZCollisionBox::CalcImpactNormal(ZActiveImpactPrim* pActiveImpact, const float* pfSpherePos, const float* pfSphereRadius) const
    {
        // TODO: Finish me
    }

    void ZCollisionBox::CalcOutFromImpact(float* pfSpeed, const ZSphereImpact* pSphereImpact, uint32_t uiImpactNr) const
    {
        const ZActiveImpactPrim* pImpact = &pSphereImpact->m_ActiveImpacts[uiImpactNr];
        const float fDot = vdot(pfSpeed, pImpact->m_vNormal.Get());
        if (fDot < 0.00012207031f)
        {
            vaddscalar(pfSpeed, pfSpeed, pImpact->m_vNormal.Get(), 0.00012207031f - fDot);
            ZASSERT(vdot(pfSpeed, pImpact->m_vNormal.Get()) > 0.0f);
        }
    }

    void ZCollisionBox::CalcOutFromImpact(float* pfSpeed, const ZSphereImpact* pSphereImpact, uint32_t uiImpactNr1, uint32_t uiImpactNr2) const
    {
        const ZActiveImpactPrim* pImpact1 = &pSphereImpact->m_ActiveImpacts[uiImpactNr1];
        const ZActiveImpactPrim* pImpact2 = &pSphereImpact->m_ActiveImpacts[uiImpactNr2];

        float vCross[3];
        vcross(vCross, pImpact1->m_vNormal.Get(), pImpact2->m_vNormal.Get());
        vnorm(vCross);

        const float fDot = vdot(pfSpeed, vCross);
        vscalar(pfSpeed, vCross, fDot);

        float vSum[3];
        vadd(vSum, pImpact1->m_vNormal.Get(), pImpact2->m_vNormal.Get());
        vnorm(vSum);
        vaddscalar(pfSpeed, pfSpeed, vSum, 0.00012207031f);
    }

    bool ZCollisionBox::CheckSpeedAllowed(const float* pfSpeed, const ZSphereImpact* pSphereImpact, uint32_t uiImpactNr) const
    {
        return vdot(pfSpeed, pSphereImpact->m_ActiveImpacts[uiImpactNr].m_vNormal.Get()) >= 0.00012207031f;
    }

    void ZCollisionBox::CalcAllowedOutSpeed(float* pfSpeed, ZSphereImpact* pSphereImpact, uint32_t uiImpactNr1, uint32_t uiImpactNr2) const
    {
        float vSpeed[3];

        vcpy(vSpeed, pfSpeed);
        CalcOutFromImpact(vSpeed, pSphereImpact, uiImpactNr1);
        if (CheckSpeedAllowed(vSpeed, pSphereImpact, uiImpactNr2))
        {
            pSphereImpact->m_ActiveImpacts[uiImpactNr2].m_bNoEnter = true;
            vcpy(pfSpeed, vSpeed);
            return;
        }

        vcpy(vSpeed, pfSpeed);
        CalcOutFromImpact(vSpeed, pSphereImpact, uiImpactNr2);
        if (CheckSpeedAllowed(vSpeed, pSphereImpact, uiImpactNr1))
        {
            pSphereImpact->m_ActiveImpacts[uiImpactNr1].m_bNoEnter = true;
            vcpy(pfSpeed, vSpeed);
            return;
        }

        CalcOutFromImpact(pfSpeed, pSphereImpact, uiImpactNr1, uiImpactNr2);
    }

    void ZCollisionBox::CalcSpeedAndActiveImpacts(float* pfSpeed, const float* pfSpherePos, const float* pfSphereRadius, ZSphereImpact* pSphereImpact) const
    {
        for (uint32_t i = 0; i < pSphereImpact->m_lNrActiveImpacts; ++i)
        {
            CalcImpactNormal(&pSphereImpact->m_ActiveImpacts[i], pfSpherePos, pfSphereRadius);
        }

        uint32_t uiWriteIdx = 0;
        for (uint32_t i = 0; i < pSphereImpact->m_lNrActiveImpacts; ++i)
        {
            if (pSphereImpact->m_ActiveImpacts[i].m_bInvalid)
            {
                --pSphereImpact->m_lNrActiveImpacts;
            }
            else
            {
                if (uiWriteIdx != i)
                {
                    std::memcpy(&pSphereImpact->m_ActiveImpacts[uiWriteIdx], &pSphereImpact->m_ActiveImpacts[i], sizeof(ZActiveImpactPrim));
                }
                ++uiWriteIdx;
            }
        }
    }

    bool ZCollisionBox::PushSphereOutOfMesh(ZSphereImpact* pSphereImpact, float* pfSpherePos, const float* pfSphereRadius, SPushWork* pPushWork) const
    {
        // TODO: Finish this place after ZCollisionBase::GetPrimFromStripUniqueId will be reversed
        return false;
    }

    bool ZCollisionBox::PushSphereOutOfStrips(ZSphereImpact* pSphereImpact, float* pfSpherePos, const float* pfSphereRadius, SPushWork* pPushWork) const
    {
        // TODO: Finish this place after ZCollisionBase::GetPrimFromStripUniqueId will be reversed
        return false;
    }

    uint32_t ZCollisionBox::PushSphereOutOfTriangle(ZSphereImpact* pSphereImpact, float (*pfTriangle)[3], float* pfSpherePos, float fSphereRadius, SPushWork* pPushWork, uint32_t uiMaterialId, float fStairAngle) const
    {
        // TODO: Finish this place after ZCollisionBase::GetPrimFromStripUniqueId will be reversed
        return 0;
    }

    bool ZCollisionBox::MovingSphereCollision(ZSphereImpact* pSphereImpact, const float* pfOldPos, float* pfNewPos, const float* pfSphereRadius, float* pfSpeed) const
    {
        // TODO: Finish this place after ZCollisionBase::GetPrimFromStripUniqueId and ZSysInterface::m_fTimeMultiplier will be reversed
        return pSphereImpact->m_bGroundContact;
    }

    ZBaseGeom* ZCollisionBox::GetDynamicContactGeom(ZSphereImpact* pSphereImpact, const ZBaseGeom* pLastGeom) const
    {
        ZBaseGeom* pResult = const_cast<ZBaseGeom*>(pLastGeom);
        for (uint32_t i = 0; i < pSphereImpact->m_lNrActiveImpacts; ++i)
        {
            pResult = pSphereImpact->m_ActiveImpacts[i].m_pBaseGeom;
            if (pResult == pLastGeom)
                return pResult;
        }
        return pResult;
    }

    bool ZCollisionBox::Solve3x3System(float* pfMat, float* pfVec, float* pfOut) const
    {
        const float fDet = pfMat[0] * pfMat[4] * pfMat[8]
                         + pfMat[1] * pfMat[5] * pfMat[6]
                         + pfMat[2] * pfMat[3] * pfMat[7]
                         - pfMat[2] * pfMat[4] * pfMat[6]
                         - pfMat[1] * pfMat[3] * pfMat[8]
                         - pfMat[0] * pfMat[5] * pfMat[7];

        if (std::fabs(fDet) < 0.000001f)
            return false;

        const float fInvDet = 1.0f / fDet;

        pfOut[0] = (pfVec[0] * pfMat[4] * pfMat[8]
                  + pfVec[1] * pfMat[5] * pfMat[6]
                  + pfVec[2] * pfMat[3] * pfMat[7]
                  - pfVec[2] * pfMat[4] * pfMat[6]
                  - pfVec[1] * pfMat[3] * pfMat[8]
                  - pfVec[0] * pfMat[5] * pfMat[7]) * fInvDet;

        pfOut[1] = (pfMat[0] * pfVec[1] * pfMat[8]
                  + pfMat[1] * pfVec[2] * pfMat[6]
                  + pfMat[2] * pfVec[0] * pfMat[7]
                  - pfMat[2] * pfVec[1] * pfMat[6]
                  - pfMat[1] * pfVec[0] * pfMat[8]
                  - pfMat[0] * pfVec[2] * pfMat[7]) * fInvDet;

        pfOut[2] = (pfMat[0] * pfMat[4] * pfVec[2]
                  + pfMat[1] * pfMat[5] * pfVec[0]
                  + pfMat[2] * pfMat[3] * pfVec[1]
                  - pfMat[2] * pfMat[4] * pfVec[0]
                  - pfMat[1] * pfMat[3] * pfVec[2]
                  - pfMat[0] * pfMat[5] * pfVec[1]) * fInvDet;

        return true;
    }

    void ZCollisionBox::RandomUnitVector(float* pfOut) const
    {
        float fLength;
        do
        {
            vset(pfOut,
                 g_pSysInterface->FRand(nullptr, 0) - 0.5f,
                 g_pSysInterface->FRand(nullptr, 0) - 0.5f,
                 g_pSysInterface->FRand(nullptr, 0) - 0.5f);
            fLength = vlen(pfOut);
        } while (fLength < 0.000001f);

        vscalar(pfOut, 1.0f / fLength);
    }

    uint32_t ZCollisionBox::DistPointLineVar2(const float* pfPoint, const float* pfLineA, const float* pfLineB, float* pfT, float* pfMinDist, float* pfDist, float* pfDir, uint32_t uiEdgeFlagA, uint32_t uiEdgeFlagB, uint32_t uiLineFlag) const
    {
        float vToPoint[3];
        float vLineDir[3];

        vsub(vToPoint, pfPoint, pfLineA);
        vsub(vLineDir, pfLineB, pfLineA);

        const float fLineLen2 = vdot(vLineDir, vLineDir);
        *pfT = vdot(vToPoint, vLineDir);

        uint32_t uiResult;
        if (*pfT <= fLineLen2)
        {
            *pfT = *pfT / fLineLen2;
            float vClosest[3];
            vaddscalar(vClosest, pfLineA, vLineDir, *pfT);
            vsub(pfDir, pfPoint, vClosest);
            uiResult = uiLineFlag;
        }
        else
        {
            *pfT = 1.0f;
            vsub(pfDir, pfPoint, pfLineB);
            uiResult = uiEdgeFlagB;
        }

        const float fDist2 = vlen2(pfDir);
        if (fDist2 > *pfMinDist * *pfMinDist)
            return 0;

        *pfDist = std::sqrt(fDist2);
        if (*pfDist < 0.000001f)
        {
            RandomUnitVector(pfDir);
        }
        else
        {
            vscalar(pfDir, 1.0f / *pfDist);
        }

        return uiResult;
    }

    void ZCollisionBox::GetRootPoint(float* pfPoint) const
    {
        vmmul(pfPoint, m_mBoxMatrix.data);
        vadd(pfPoint, m_vBoxPosition.Get());
    }

    void ZCollisionBox::GetLocalPoint(float* pfPoint) const
    {
        vsub(pfPoint, m_vBoxPosition.Get());
        vmtmul(pfPoint, m_mBoxMatrix.data);
    }

    void ZCollisionBox::GetRootPoint(float* pfOut, const float* pfPoint) const
    {
        vmmul(pfOut, pfPoint, m_mBoxMatrix.data);
        vadd(pfOut, m_vBoxPosition.Get());
    }

    void ZCollisionBox::GetLocalPoint(float* pfOut, const float* pfPoint) const
    {
        vsub(pfOut, pfPoint, m_vBoxPosition.Get());
        vmtmul(pfOut, m_mBoxMatrix.data);
    }

    void ZCollisionBox::GetRootVect(float* pfVect) const
    {
        vmmul(pfVect, m_mBoxMatrix.data);
    }

    void ZCollisionBox::GetRootVect(float* pfOut, const float* pfVect) const
    {
        vmmul(pfOut, pfVect, m_mBoxMatrix.data);
    }

    void ZCollisionBox::GetLocalVect(float* pfOut, const float* pfVect) const
    {
        vmtmul(pfOut, pfVect, m_mBoxMatrix.data);
    }
}
