#include "Glacier/Physics/Fysix/Fysix.h"
#include "Glacier/ZSTL/ZMath.h"
#include <Glacier/Physics/Fysix/ConstrainedParticleSystem.h>
#include <Glacier/Physics/Fysix/ZWaterBoxManager.h>
#include <Glacier/Physics/Fysix/ZWaterBox.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/Physics/SFastBoxColiTri.h>
#include <Glacier/Physics/SCapsuleColiInfo.h>
#include <Glacier/Physics/ZFastBoxColi.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <xmmintrin.h>
#include <numbers>
#include <cmath>


namespace Glacier
{
    namespace
    {
        void MakeBaseTransformation(ZVector3 (&vBasis)[3], float& fOutProjScaleX, float& fOutProjScaleY, float fLeftAngleDeg, float fRightAngleDeg, float fBottonAngleDeg, float fTopAngleDeg)
        {
            constexpr float DEG_TO_RAD_HALF = std::numbers::pi_v<float> / 360.0f; //0.0087266462f;

            const float fCenterYawRad   = (fLeftAngleDeg + fRightAngleDeg) * DEG_TO_RAD_HALF;
            const float fCenterPitchRad = (fBottonAngleDeg + fTopAngleDeg) * DEG_TO_RAD_HALF;
            const float fHalfFovXRad    = (fRightAngleDeg - fLeftAngleDeg) * DEG_TO_RAD_HALF;
            const float fHalfFovYRad    = (fTopAngleDeg - fBottonAngleDeg) * DEG_TO_RAD_HALF;

            ZVector3& vForward = vBasis[0];
            ZVector3& vUp      = vBasis[1];
            ZVector3& vRight   = vBasis[2];

            // Calc forward vector
            vForward.x = std::sinf(fCenterYawRad);
            vForward.z = std::cosf(fCenterYawRad);
            vForward.y = std::tanf(fCenterPitchRad) * vForward.z;
            vnorm(vForward);

            // Calc right vector
            vRight.x = vForward.z;
            vRight.y = 0.0f;
            vRight.z = -vForward.x;
            vnorm(vRight);

            // Calc up vector
            vcross(vUp, vForward, vRight);
            vnorm(vUp);

            // Calc scale projection xy
            fOutProjScaleX = 1.f / std::tanf(fHalfFovXRad);
            fOutProjScaleY = 1.f / std::tanf(fHalfFovYRad);
        }

        inline void MakeRandomVector(ZVector3& v)
        {
            v = {
                g_pSysInterface->FRand(nullptr, 0),
                g_pSysInterface->FRand(nullptr, 0),
                g_pSysInterface->FRand(nullptr, 0),
            };
        }

        void AdjustPart2(ZVector3& vPos1, ZVector3& vPos2, float fTargetDist, float fMass1, float fMass2)
        {
            ZVector3 vDelta;
            vsub(vDelta, vPos2, vPos1);

            float fDist = vlen(vDelta);

            // Find solution without division by zero
            while (fDist < 0.000001f)
            {
                vDelta.x = g_pSysInterface->FRand(nullptr, 0);
                vDelta.y = g_pSysInterface->FRand(nullptr, 0);
                vDelta.z = g_pSysInterface->FRand(nullptr, 0);
                fDist = vlen(vDelta);
            }

            // Normalize dir vector
            ZVector3 vDir;
            vscalar(vDir, vDelta, 1.0f / fDist);

            const float fInvTotalMass = 1.0f / (fMass1 + fMass2);
            const float fDeltaDist    = fDist - fTargetDist; // Deform error compute

            const float fShift1 =  (fInvTotalMass * fMass2) * fDeltaDist * 0.6f;
            const float fShift2 = -(fInvTotalMass * fMass1) * fDeltaDist * 0.6f;

            vaddscalar(vPos1, vPos1, vDir, fShift1);
            vaddscalar(vPos2, vPos2, vDir, fShift2);
        }

        void AdjustSpecialConstraint(ZVector3& vPos1, ZVector3& vPos2, float fTargetDist, float fMass1, float fMass2)
        {
            ZVector3 vDelta;
            vsub(vDelta, vPos2, vPos1);

            float fDist = vlen(vDelta);

            if ((fTargetDist <= 0.0f || fDist <= fTargetDist) && (fTargetDist >= 0.0f || fDist >= -fTargetDist))
            {
                if (fTargetDist < 0.0f)
                {
                    fTargetDist = -fTargetDist;
                }

                // Find solution without division by zero
                while (fDist < 0.000001f)
                {
                    vDelta.x = g_pSysInterface->FRand(nullptr, 0);
                    vDelta.y = g_pSysInterface->FRand(nullptr, 0);
                    vDelta.z = g_pSysInterface->FRand(nullptr, 0);
                    fDist = vlen(vDelta);
                }

                const float fFactor = ((fDist - fTargetDist) * 0.85f) / ((fMass1 + fMass2) * fDist);
                const float fShift1 =  fFactor * fMass2 * 0.6f;
                const float fShift2 = -fFactor * fMass1 * 0.6f;

                // Move towards vDelta
                vaddscalar(vPos1, vPos1, vDelta, fShift1);
                vaddscalar(vPos2, vPos2, vDelta, fShift2);
            }
        }

        void ZeroOutPoints2(ZVector3& vPos1, ZVector3& vPos2, float fMass1, float fMass2, const ZVector3& vNormal)
        {
            constexpr float CORRECTION_COFF = 0.75f; // 75%

            ZVector3 vDelta;
            vsub(vDelta, vPos2, vPos1);

            const float fDot = vdot(vDelta, vNormal);

            if (fDot <= 0.0f)
            {
                const float fMassSum = fMass1 + fMass2;
                const float fShift1Base = fDot * fMass2 / fMassSum;

                const float fShift1 = fShift1Base * CORRECTION_COFF;
                const float fShift2 = (fShift1Base - fDot) * CORRECTION_COFF;

                vaddscalar(vPos1, vPos1, vNormal, fShift1);
                vaddscalar(vPos2, vPos2, vNormal, fShift2);
            }
        }

        void AdjustPlane2(int iIdx1, int iIdx2, const ZVector3& vNormal, Particle* pParticles)
        {
            ZeroOutPoints2(
                pParticles[iIdx1].x,
                pParticles[iIdx2].x,
                pParticles[iIdx1].mass,
                pParticles[iIdx2].mass,
                vNormal
            );
        }

        void AdjustCone4(int iIdx0, int iIdx1, int iIdx2, int iIdx3, const ZMat3x3& mTransform, float fScaleX, float fScaleY, Particle* pParticles)
        {
            constexpr float ADJUST_COFF = 0.75f;

            Particle& p0 = pParticles[iIdx0];
            Particle& p1 = pParticles[iIdx1];
            Particle& p2 = pParticles[iIdx2];
            Particle& p3 = pParticles[iIdx3];

            // Compute diff for target particle
            ZVector3 vDiff;
            vsub(vDiff, p3.x, p0.x);

            // Convert local space to cone space
            ZVector3 vLocal;
            vmtmul(vLocal, vDiff, mTransform);

            ZVector3 vScaled;
            vScaled.x = vLocal.x * fScaleX;
            vScaled.y = vLocal.y * fScaleY;
            vScaled.z = 0.0f;

            // Is particle outside of cone?
            if (vLocal.z * vLocal.z < (vScaled.x * vScaled.x + vScaled.y * vScaled.y) || vLocal.z <= 0.0f)
            {
                ZVector3 vConeDir;
                vnorm(vConeDir, vScaled);
                vConeDir.z = 1.0f;
                vConeDir.x /= fScaleX;
                vConeDir.y /= fScaleY;

                // Convert bounds point to world space
                ZVector3 vConeTargetRel;
                vmmul(vConeTargetRel, vConeDir, mTransform);

                ZVector3 vConeTarget;
                vadd(vConeTarget, vConeTargetRel, p0.x);

                ZVector3 vP3Rel;
                vsub(vP3Rel, p3.x, p0.x);

                const float fLenSq = vdot(vConeTargetRel, vConeTargetRel);

                if (fLenSq >= 0.00000001f)
                {
                    // Projection p3 to cone ray
                    const float fProjFactor = vdot(vP3Rel, vConeTargetRel) / fLenSq;

                    ZVector3 vProjected;
                    vaddscalar(vProjected, p0.x, vConeTargetRel, fProjFactor);

                    // Find correction error
                    ZVector3 vErr;
                    vsub(vErr, p3.x, vProjected);

                    ZVector3 vErrDir;
                    const float fErrLen = vnorm(vErrDir, vErr);

                    // Need solve system 3x3 for find barycentric coords
                    ZVector3 vRel1, vRel2, vCross12;
                    vsub(vRel1, p1.x, p0.x);
                    vsub(vRel2, p2.x, p0.x);
                    vcross(vCross12, vRel1, vRel2);

                    ZVector3 vProjRel;
                    vsub(vProjRel, vProjected, p0.x);

                    ZMat3x3 mBase
                    {
                        vRel1.x, vRel1.y, vRel1.z,
                        vRel2.x, vRel2.y, vRel2.z,
                        vCross12.x, vCross12.y, vCross12.z
                    };

                    ZVector3 vWeights;
                    if (ZCommonAlgorithms::Solve3x3System(mBase.data, vProjRel, vWeights))
                    {
                        const float w0 = 1.0f - (vWeights.x + vWeights.y + vWeights.z);
                        const float w1 = vWeights.x;
                        const float w2 = vWeights.y;
                        const float w3 = vWeights.z;

                        ZMat3x3 mSolve;
                        vsub(mSolve.ZAxis(), p0.x, vCross12);
                        vsub(mSolve.YAxis(), p1.x, vCross12);
                        vsub(mSolve.XAxis(), p2.x, vCross12);

                        ZVector3 vImpulse;
                        if (ZCommonAlgorithms::Solve3x3System(mSolve.data, vErrDir, vImpulse))
                        {
                            ZVector3 vJ0, vJ1, vJ2;

                            // Compute impulse by particle mass
                            const float fInvMass0 = 1.0f / p0.mass;
                            const float fInvMass1 = 1.0f / p1.mass;
                            const float fInvMass2 = 1.0f / p2.mass;
                            const float fInvMass3 = 1.0f / p3.mass;

                            vmuls(vJ0, vErrDir, w0 * fInvMass0);
                            vaddscalar(vJ0, vJ0, mSolve.ZAxis(), w3 * fInvMass0);

                            vmuls(vJ1, vErrDir, w1 * fInvMass1);
                            vaddscalar(vJ1, vJ1, mSolve.YAxis(), w3 * fInvMass1);

                            vmuls(vJ2, vErrDir, w2 * fInvMass2);
                            vaddscalar(vJ2, vJ2, mSolve.XAxis(), w3 * fInvMass2);

                            ZVector3 vSum;
                            vmuls(vSum, vJ0, w0);
                            vaddscalar(vSum, vSum, vJ1, w1);
                            vaddscalar(vSum, vSum, vJ2, w2);
                            vaddscalar(vSum, vSum, vCross12, w3);

                            const float fDenom = vdot(vErrDir, vSum) + fInvMass3;

                            if (fabsf(fDenom) >= 0.00012207031f)
                            {
                                const float fFactor = (fErrLen * ADJUST_COFF) / fDenom;

                                // Apply offsets
                                vaddscalar(p0.x, p0.x, vJ0, fFactor);
                                vaddscalar(p1.x, p1.x, vJ1, fFactor);
                                vaddscalar(p2.x, p2.x, vJ2, fFactor);
                                vaddscalar(p3.x, p3.x, vErrDir, -(fFactor * fInvMass3));
                            }
                        }
                    }
                }
            }
        }

        void AdjustCone5(int iIdx0, int iIdx1, int iIdx2, int iIdx3, const ZMat3x3& mTransform, float fScale, float fScaleY, Particle* pParticles)
        {
            constexpr float ADJUST_COFF = 0.75f;

            Particle& p0 = pParticles[iIdx0];
            Particle& p1 = pParticles[iIdx1];
            Particle& p2 = pParticles[iIdx2];
            Particle& p3 = pParticles[iIdx3];

            // Compute diff in local space
            ZVector3 vDiff;
            vsub(vDiff, p3.x, p0.x);

            ZVector3 vLocal;
            vmtmul(vLocal, vDiff, mTransform);

            const float fScaledX = vLocal.x * fScale;

            // Is particle outside cone?
            if ((fabsf(fScaledX) >= vLocal.z || vLocal.z <= 0.0f) && vLocal.z >= 0.0f)
            {
                ZVector3 vTargetLocal;
                vTargetLocal.y = vLocal.y;

                if (fScaledX < 0.0f)
                {
                    const float fDist = (-fScaledX - vLocal.z) / (fScale * fScale + 1.0f);
                    if (fDist > 0.0f)
                    {
                        vTargetLocal.x = vLocal.x + fScale * fDist;
                        vTargetLocal.z = vLocal.z + fDist;
                    }
                    else
                    {
                        vTargetLocal.x = (vLocal.x >= 0.0f) ? 0.000001f : -0.000001f;
                        vTargetLocal.z = fScale * 0.000001f;
                    }
                }
                else
                {
                    const float fDist = (fScaledX - vLocal.z) / (fScale * fScale + 1.0f);
                    if (fDist > 0.0f)
                    {
                        vTargetLocal.x = vLocal.x - fScale * fDist;
                        vTargetLocal.z = vLocal.z + fDist;
                    }
                    else
                    {
                        vTargetLocal.x = 0.000001f;
                        vTargetLocal.z = fScale * 0.000001f;
                    }
                }

                // Convert target point back to world space
                ZVector3 vConeTarget;
                vmmul(vConeTarget, vTargetLocal, mTransform);
                vadd(vConeTarget, p0.x);

                // Compute error vector
                ZVector3 vErr;
                vsub(vErr, p3.x, vConeTarget);

                if (vdot(vErr, vErr) >= 1e-11f)
                {
                    ZVector3 vErrDir;
                    const float fErrLen = vnorm(vErrDir, vErr);

                    // Solve system 3x3 to find trangle weights
                    ZVector3 vRel1, vRel2, vCross12;
                    vsub(vRel1, p1.x, p0.x);
                    vsub(vRel2, p2.x, p0.x);
                    vcross(vCross12, vRel1, vRel2);

                    ZVector3 vTargetRel;
                    vsub(vTargetRel, vConeTarget, p0.x);

                    ZMat3x3 mBase
                    {
                        vRel1.x, vRel1.y, vRel1.z,
                        vRel2.x, vRel2.y, vRel2.z,
                        vCross12.x, vCross12.y, vCross12.z
                    };

                    ZVector3 vWeights;
                    if (ZCommonAlgorithms::Solve3x3System(mBase.data, vTargetRel, vWeights))
                    {
                        const float w0 = 1.0f - (vWeights.x + vWeights.y + vWeights.z);
                        const float w1 = vWeights.x;
                        const float w2 = vWeights.y;
                        const float w3 = vWeights.z;

                        ZMat3x3 mSolve;
                        vsub(mSolve.ZAxis(), p0.x, vCross12);
                        vsub(mSolve.YAxis(), p1.x, vCross12);
                        vsub(mSolve.XAxis(), p2.x, vCross12);

                        ZVector3 vImpulse;
                        if (ZCommonAlgorithms::Solve3x3System(mSolve.data, vErrDir, vImpulse))
                        {
                            if (p0.mass != 0.0f && p1.mass != 0.0f && p2.mass != 0.0f)
                            {
                                const float fInvMass0 = 1.0f / p0.mass;
                                const float fInvMass1 = 1.0f / p1.mass;
                                const float fInvMass2 = 1.0f / p2.mass;

                                ZVector3 vJ0, vJ1, vJ2;
                                vmuls(vJ0, vErrDir, w0 * fInvMass0);
                                vaddscalar(vJ0, vJ0, mSolve.ZAxis(), w3 * fInvMass0);

                                vmuls(vJ1, vErrDir, w1 * fInvMass1);
                                vaddscalar(vJ1, vJ1, mSolve.YAxis(), w3 * fInvMass1);

                                vmuls(vJ2, vErrDir, w2 * fInvMass2);
                                vaddscalar(vJ2, vJ2, mSolve.XAxis(), w3 * fInvMass2);

                                ZVector3 vSum;
                                vmuls(vSum, vJ0, w0);
                                vaddscalar(vSum, vSum, vJ1, w1);
                                vaddscalar(vSum, vSum, vJ2, w2);
                                vaddscalar(vSum, vSum, vCross12, w3);

                                if (p3.mass != 0.0f)
                                {
                                    const float fInvMass3 = 1.0f / p3.mass;
                                    const float fDenom = vdot(vErrDir, vSum) + fInvMass3;

                                    if (fabsf(fDenom) >= 0.000001f)
                                    {
                                        const float fFactor = (fErrLen * ADJUST_COFF) / fDenom;

                                        vaddscalar(p0.x, p0.x, vJ0, fFactor);
                                        vaddscalar(p1.x, p1.x, vJ1, fFactor);
                                        vaddscalar(p2.x, p2.x, vJ2, fFactor);
                                        vaddscalar(p3.x, p3.x, vErrDir, -(fFactor * fInvMass3));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void AdjustCone6(int iIdx0, int iIdx1, int iIdx2, int iIdx3, const ZMat3x3& mTransform, float fScaleX, float fScaleY, Particle* pParticles)
        {
            Particle& p0 = pParticles[iIdx0];
            Particle& p1 = pParticles[iIdx1];
            Particle& p2 = pParticles[iIdx2];
            Particle& p3 = pParticles[iIdx3];

            ZVector3 vDiff;
            vsub(vDiff, p3.x, p0.x);

            ZVector3 vLocal;
            vmtmul(vLocal, vDiff, mTransform);

            const float fScaledY = vLocal.y * fScaleY;

            // Check out of cone
            if ((fabsf(fScaledY) >= vLocal.z || vLocal.z <= 0.0f) && vLocal.z >= 0.0f)
            {
                ZVector3 vTargetLocal;
                vTargetLocal.x = vLocal.x; // Fox Y-axis

                if (fScaledY < 0.0f)
                {
                    const float fDist = (-fScaledY - vLocal.z) / (fScaleY * fScaleY + 1.0f);
                    if (fDist > 0.0f)
                    {
                        vTargetLocal.y = vLocal.y + fScaleY * fDist;
                        vTargetLocal.z = vLocal.z + fDist;
                    }
                    else
                    {
                        vTargetLocal.y = (vLocal.y < 0.0f) ? -0.000001f : 0.000001f;
                        vTargetLocal.z = fScaleY * 0.000001f;
                    }
                }
                else
                {
                    const float fDist = (fScaledY - vLocal.z) / (fScaleY * fScaleY + 1.0f);
                    if (fDist > 0.0f)
                    {
                        vTargetLocal.y = vLocal.y - fScaleY * fDist;
                        vTargetLocal.z = vLocal.z + fDist;
                    }
                    else
                    {
                        vTargetLocal.y = (vLocal.y < 0.0f) ? -0.000001f : 0.000001f;
                        vTargetLocal.z = fScaleY * 0.000001f;
                    }
                }

                // Convert cone space to world space
                ZVector3 vConeTarget;
                vmmul(vConeTarget, vTargetLocal, mTransform);
                vadd(vConeTarget, p0.x);

                // Find error correction
                ZVector3 vErr;
                vsub(vErr, p3.x, vConeTarget);

                if (vdot(vErr, vErr) >= 1e-11f)
                {
                    ZVector3 vErrDir;
                    const float fErrLen = vnorm(vErrDir, vErr);

                    // Solve system 3x3 to find weights
                    ZVector3 vRel1, vRel2, vCross12;
                    vsub(vRel1, p1.x, p0.x);
                    vsub(vRel2, p2.x, p0.x);
                    vcross(vCross12, vRel1, vRel2);

                    ZVector3 vTargetRel;
                    vsub(vTargetRel, vConeTarget, p0.x);

                    ZMat3x3 mBase {
                        vRel1.x, vRel1.y, vRel1.z,
                        vRel2.x, vRel2.y, vRel2.z,
                        vCross12.x, vCross12.y, vCross12.z
                    };

                    ZVector3 vWeights;
                    if (ZCommonAlgorithms::Solve3x3System(mBase, vTargetRel, vWeights))
                    {
                        const float w1 = vWeights.x;
                        const float w2 = vWeights.y;
                        const float w3 = vWeights.z;
                        const float w0 = 1.0f - (w1 + w2 + w3);

                        ZMat3x3 mSolve;
                        vsub(mSolve.ZAxis(), p0.x, vCross12);
                        vsub(mSolve.YAxis(), p1.x, vCross12);
                        vsub(mSolve.XAxis(), p2.x, vCross12);

                        ZVector3 vImpulse;
                        if (ZCommonAlgorithms::Solve3x3System(mSolve, vErrDir, vImpulse))
                        {
                            if (p0.mass != 0.0f && p1.mass != 0.0f && p2.mass != 0.0f)
                            {
                                const float fInvMass0 = 1.0f / p0.mass;
                                const float fInvMass1 = 1.0f / p1.mass;
                                const float fInvMass2 = 1.0f / p2.mass;

                                ZVector3 vJ0, vJ1, vJ2;
                                vmuls(vJ0, vErrDir, w0 * fInvMass0);
                                vaddscalar(vJ0, vJ0, mSolve.ZAxis(), w3 * fInvMass0);

                                vmuls(vJ1, vErrDir, w1 * fInvMass1);
                                vaddscalar(vJ1, vJ1, mSolve.YAxis(), w3 * fInvMass1);

                                vmuls(vJ2, vErrDir, w2 * fInvMass2);
                                vaddscalar(vJ2, vJ2, mSolve.XAxis(), w3 * fInvMass2);

                                ZVector3 vSum;
                                vmuls(vSum, vJ0, w0);
                                vaddscalar(vSum, vSum, vJ1, w1);
                                vaddscalar(vSum, vSum, vJ2, w2);
                                vaddscalar(vSum, vSum, vCross12, w3);

                                if (p3.mass != 0.0f)
                                {
                                    const float fInvMass3 = 1.0f / p3.mass;
                                    const float fDenom = vdot(vErrDir, vSum) + fInvMass3;

                                    if (fabsf(fDenom) >= 0.000001f)
                                    {
                                        const float fFactor = (fErrLen * 0.75f) / fDenom;

                                        vaddscalar(p0.x, p0.x, vJ0, fFactor);
                                        vaddscalar(p1.x, p1.x, vJ1, fFactor);
                                        vaddscalar(p2.x, p2.x, vJ2, fFactor);
                                        vaddscalar(p3.x, p3.x, vErrDir, -(fFactor * fInvMass3));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    static ZDebugInt g_lGravity("gravity", "Gravity force in particle dynamics", 1500, -1000000, 1000000, 1, nullptr); // PC at 0x007FE568

    // Constants
    //
    static constexpr int BASIS_SIZE = 8;
    static constexpr int PARTICLES_NR = 16;
    static constexpr int CONSTRAINTS_NR = 24;

    // Types
    //
    using BaseTransformBasisVector = ZVector3[BASIS_SIZE][3];
    using BasisProjVector = float[BASIS_SIZE];

    // Globals
    //
    STATIC_GLOBAL_CLASS_INSTANCE(bool, bTransformsInited);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(bool, bTransformsInited, 0x009A33CC, false);

    STATIC_GLOBAL_CLASS_INSTANCE(BaseTransformBasisVector, g_aBaseTransformBasis);
    STATIC_GLOBAL_CLASS_INSTANCE(BasisProjVector, g_aBaseProjScaleX);
    STATIC_GLOBAL_CLASS_INSTANCE(BasisProjVector, g_aBaseProjScaleY);

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(BasisProjVector, g_aBaseProjScaleX, 0x009A3288, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(BasisProjVector, g_aBaseProjScaleY, 0x009A3268, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(BaseTransformBasisVector, g_aBaseTransformBasis, 0x009A32A8, {});

    STATIC_GLOBAL_ARRAY(int, 15, g_lSausageA, 0x00781B14, { 5, 5, 5, 6, 6, 8, 9, 12, 13, 2, 3, 4, 7, 2, 3 });
    STATIC_GLOBAL_ARRAY(int, 15, g_lSausageB, 0x00781B50, { 6, 8, 2, 9, 3, 12, 13, 14, 15, 4, 7, 10, 11, 1, 1 });
    STATIC_GLOBAL_ARRAY(float, 15, g_lSausageRadius, 0x00781B8C, { 13.0f, 13.0f, 10.0f, 13.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 5.0f, 5.0f, 4.0f, 4.0f, 10.0f, 10.0f });

    Particle::Particle()
    {
        MakeRandomVector(x);
        ok_x = x;
        oldx = x;
        v = { 0.f };
        mass = 1.0f;
    }

    void Particle::Init(const ZVector3& vPos, const ZVector3& vVel, float fMass)
    {
        x = vPos;
        oldx = vPos;
        v = vVel;
        mass = fMass;
    }

    ParticleConstraint::ParticleConstraint()
    {
        m_pPar1 = nullptr;
        m_pPar2 = nullptr;
        m_fDist = 0.0f;
    }

    void ParticleConstraint::Init(Particle* pPar1, Particle* pPar2)
    {
        m_pPar1 = pPar1;
        m_pPar2 = pPar2;
        ComputeDistance();
    }

    void ParticleConstraint::ComputeDistance()
    {
        if (m_pPar1 && m_pPar2)
        {
            ZVector3 vDiff;
            vsub(vDiff, m_pPar1->x, m_pPar2->x);
            m_fDist = vlen(vDiff);
        }
    }

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

    ConstrainedParticleSystem::ConstrainedParticleSystem()
    {
        DefaultConstruct();
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

    void ConstrainedParticleSystem::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        LoadSaveParticles(stream, bSaving);
        LoadSaveConstraints(stream, bSaving);
        stream.Exchange("m_fDamping", m_fDamping);
    }

    void ConstrainedParticleSystem::LoadSaveParticles(ISerializerStream& stream, bool bSaving)
    {
        stream.Exchange("m_iNumParticles", m_iNumParticles);

        for (int i = 0; i < m_iNumParticles; ++i)
        {
            stream.ExchangeArray("x", m_pParticles[i].x, 3);
            stream.ExchangeArray("ok_x", m_pParticles[i].ok_x, 3);
            stream.ExchangeArray("oldx", m_pParticles[i].oldx, 3);
            stream.ExchangeArray("v", m_pParticles[i].v, 3);
        }
    }

    void ConstrainedParticleSystem::LoadSaveConstraints(ISerializerStream& stream, bool bSaving)
    {
        stream.Exchange("m_iNumConstraints", m_iNumConstraints);

        ParticleConstraint* con = m_pConstraints;
        for (int i = 0; i < m_iNumConstraints; ++i, ++con)
        {
            int32_t ix1 = static_cast<int32_t>(con->m_pPar1 - m_pParticles);
            int32_t ix2 = static_cast<int32_t>(con->m_pPar2 - m_pParticles);

            stream.Exchange("ix1", ix1);
            stream.Exchange("ix2", ix2);

            con->m_pPar1 = &m_pParticles[ix1];
            con->m_pPar2 = &m_pParticles[ix2];

            stream.Exchange("m_fDist", con->m_fDist);
        }

        stream.Exchange("m_iNumSpecialConstraints", m_iNumSpecialConstraints);

        ParticleConstraint* specialCon = m_pSpecialConstraints;
        for (int i = 0; i < m_iNumSpecialConstraints; ++i, ++specialCon)
        {
            int32_t ix1 = static_cast<int32_t>(specialCon->m_pPar1 - m_pParticles);
            int32_t ix2 = static_cast<int32_t>(specialCon->m_pPar2 - m_pParticles);

            stream.Exchange("ix1", ix1);
            stream.Exchange("ix2", ix2);

            specialCon->m_pPar1 = &m_pParticles[ix1];
            specialCon->m_pPar2 = &m_pParticles[ix2];

            stream.Exchange("m_fDist", specialCon->m_fDist);
        }
    }

    void ConstrainedParticleSystem::SetNumConstraints(int n)
    {
        ZASSERT(n <= m_iNumConstraints);

        m_iNumConstraints = n;
    }

    void ConstrainedParticleSystem::SetParticleOldPos(int i, const ZVector3& oldpos)
    {
        m_pParticles[i].oldx = oldpos;
    }

    void ConstrainedParticleSystem::GetParticleOldPos(int lIndex, ZVector3& vPos)
    {
        vPos = m_pParticles[lIndex].oldx;
    }

    void ConstrainedParticleSystem::SetParticlePos(int lIndex, const ZVector3& vPos)
    {
        m_pParticles[lIndex].x = vPos;
    }

    void ConstrainedParticleSystem::GetParticleOKPos(int lIndex, ZVector3& vPos)
    {
        vPos = m_pParticles[lIndex].ok_x;
    }

    void ConstrainedParticleSystem::SetParticleOKPos(int lIndex, const ZVector3& vPos)
    {
        m_pParticles[lIndex].ok_x = vPos;
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

    void ConstrainedParticleSystem::BlowDirBomb(const ZVector3& pos, float fForce, const ZVector3& dir)
    {
        if (fForce < 0.001f)
            return;

        ZVector3 vDir(dir);

        const float fDirLen = vlen(vDir);
        if (fDirLen < 0.000001f)
        {
            BlowBomb(pos, fForce);
            return;
        }

        vscalar(vDir, 1.0f / fDirLen);

        float fTotalWeight = 0.0f;
        for (int i = 0; i < m_iNumParticles; ++i)
        {
            const Particle& par = m_pParticles[i];
            fTotalWeight += 1.0f / (vdist(par.x, pos) + 1.0f);
        }

        for (int i = 0; i < m_iNumParticles; ++i)
        {
            Particle& par = m_pParticles[i];

            const float fDist = vdist(par.x, pos);
            vaddscalar(par.v, par.v, vDir, fForce * (fDist / fTotalWeight));
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

    void ConstrainedParticleSystem::DefaultConstruct()
    {
        m_fPrevTimeStep = -1.0f;
        m_fDamping = 0.0099999998f;
        m_bReallyInWater = false;
        m_bReallyInWaterOld = false;
        m_bInWater = false;

        ConstrainedParticleSystem::m_pWaterBoxManager = ZWaterBoxManager::m_pInstance;

        m_pFastBox = nullptr;
        m_pParticles = nullptr;
        m_pConstraints = nullptr;
        m_pSpecialConstraints = nullptr;
        m_bFollow = true;
        m_iNumParticles = PARTICLES_NR;
        m_iNumConstraints = 24;
        m_iNumSpecialConstraints = 18;

        m_pParticles = ZUniMemory::NewArray<Particle>(m_iNumParticles);
        m_pConstraints = ZUniMemory::NewArray<ParticleConstraint>(m_iNumConstraints);
        m_pSpecialConstraints = ZUniMemory::NewArray<ParticleConstraint>(m_iNumSpecialConstraints);

        // Initialize base transforms
        if (!bTransformsInited)
        {
            bTransformsInited = true;

            struct STransformAnglePreset
            {
                float fLeft { 0.0f };
                float fRight { 0.0f };
                float fBottom { 0.0f };
                float fTop { 0.0f };
            };

            static constexpr STransformAnglePreset aPresets[BASIS_SIZE] = {
                { -15.0f,  65.0f, -30.0f, 10.0f },
                { -15.0f,  65.0f, -10.0f, 30.0f },
                {-165.0f,  -5.0f, -20.0f,  2.0f },
                {-165.0f,  -5.0f,  -2.0f, 20.0f },
                { -90.0f,  90.0f, -80.0f, 40.0f },
                { -90.0f,  90.0f, -40.0f, 80.0f },
                {   0.0f, 100.0f, -90.0f, 10.0f },
                {   0.0f, 100.0f, -90.0f, 10.0f },
            };

            for (int i = 0; i < BASIS_SIZE; ++i)
            {
                const auto& sPreset = aPresets[i];
                MakeBaseTransformation(g_aBaseTransformBasis[i], g_aBaseProjScaleX[i], g_aBaseProjScaleY[i], sPreset.fLeft, sPreset.fRight, sPreset.fBottom, sPreset.fTop);
            }
        }

        // Fill particles
        {
            struct SParticleInitData
            {
                ZVector3 pos;
                float    mass;
            };

            static const SParticleInitData aParticlesData[PARTICLES_NR] = {
                { {   0.0f, 260.0f,   0.0f }, 1.0f },
                { {   0.0f, 230.0f,   0.0f }, 1.0f },
                { { -50.0f, 210.0f,   0.0f }, 2.0f },
                { {  50.0f, 210.0f,   0.0f }, 2.0f },
                { { -50.0f, 150.0f,   0.0f }, 1.0f },
                { { -20.0f, 150.0f,   0.0f }, 2.0f },
                { {  20.0f, 150.0f,   0.0f }, 2.0f },
                { {  50.0f, 150.0f,   0.0f }, 1.0f },
                { { -30.0f, 120.0f,   0.0f }, 2.0f },
                { {  30.0f, 120.0f,   0.0f }, 2.0f },
                { { -50.0f,  90.0f,   0.0f }, 1.0f },
                { {  50.0f,  90.0f,   0.0f }, 1.0f },
                { { -30.0f,  60.0f,   0.0f }, 1.0f },
                { {  30.0f,  60.0f,   0.0f }, 1.0f },
                { { -30.0f,   0.0f,   0.0f }, 1.0f },
                { {  30.0f,   0.0f,   0.0f }, 1.0f }
            };

            static const ZVector3 v0 { 0.0f };

            for (size_t i = 0; i < PARTICLES_NR; ++i)
            {
                m_pParticles[i].Init(aParticlesData[i].pos, v0, aParticlesData[i].mass);
            }

            SetNumParticles(PARTICLES_NR, true);
        }

        // Fill constraints
        {
            static constexpr Fysix::SConstraintIndex aConstraints[CONSTRAINTS_NR] =
            {
                { 0,  1 }, { 1,  2 }, { 1,  3 }, { 2,  5 },
                { 3,  6 }, { 1,  5 }, { 1,  6 }, { 2,  6 },
                { 3,  5 }, { 5,  6 }, { 2,  4 }, { 3,  7 },
                { 4, 10 }, { 7, 11 }, { 5,  8 }, { 6,  9 },
                { 5,  9 }, { 6,  8 }, { 8,  9 }, { 8, 12 },
                { 9, 13 }, { 12, 14 }, { 13, 15 }, { 2,  3 }
            };

            for (size_t i = 0; i < CONSTRAINTS_NR; ++i)
            {
                const auto& sPair = aConstraints[i];
                m_pConstraints[i].Init(&m_pParticles[sPair.ix1], &m_pParticles[sPair.ix2]);
            }

            SetNumConstraints(CONSTRAINTS_NR);
        }

        InitOkX();

        // Reset lnkobj
        m_pLnkObj = nullptr;
    }

    void ConstrainedParticleSystem::ZeroOut(int iIterations)
    {
        if (iIterations <= 0)
            return;

        for (int it = 0; it < iIterations; ++it)
        {
            // Ragdoll:
            if (m_bFollow)
            {
                ZMat3x3 mTransform, mBasis;

                // Spine
                ZVector3 vMid89 = (m_pParticles[8].x + m_pParticles[9].x) * 0.5f;
                ZVector3 vSpineDir = vMid89 - m_pParticles[5].x - m_pParticles[6].x;
                vnorm(vSpineDir);

                // Pelvis
                ZVector3 vPelvisDir = m_pParticles[9].x - m_pParticles[8].x;
                vnorm(vPelvisDir);

                ZVector3 vSpineCross;
                vcross(vSpineCross, vPelvisDir, vSpineDir);
                vnorm(vSpineCross);

                // Fill bassis matrix
                mBasis.SetRow(0, vSpineDir);
                mBasis.SetRow(1, vPelvisDir);
                mBasis.SetRow(2, vSpineCross);

                // Left thigh / Leg 1
                mmmul(mTransform.data, g_aBaseTransformBasis[0][0], mBasis.data);
                AdjustPlane2(8, 12, mTransform.data, m_pParticles);
                AdjustCone4(8, 5, 9, 12, mTransform, g_aBaseProjScaleX[0], g_aBaseProjScaleY[0], m_pParticles);

                // Right thigh / Leg 2
                mmmul(mTransform.data, g_aBaseTransformBasis[1][0], mBasis.data);
                AdjustPlane2(9, 13, mTransform.data, m_pParticles);
                AdjustCone4(9, 6, 8, 13, mTransform, g_aBaseProjScaleX[1], g_aBaseProjScaleY[1], m_pParticles);

                // --- Left shin ---
                ZMat3x3 mLimb;
                ZVector3 vLegL = m_pParticles[12].x - m_pParticles[8].x;
                vnorm(vLegL);
                mLimb.SetRow(0, vLegL);

                ZVector3 vLegLCross;
                vcross(vLegLCross, vPelvisDir, vLegL);
                vnorm(vLegLCross);
                mLimb.SetRow(2, vLegLCross);

                ZVector3 vLegLUp;
                vcross(vLegLUp, vLegL, vLegLCross);
                mLimb.SetRow(1, vLegLUp);

                mmmul(mTransform.data, g_aBaseTransformBasis[2][0], mLimb.data);
                AdjustCone5(12, 8, 9, 14, mTransform, g_aBaseProjScaleX[2], g_aBaseProjScaleY[2], m_pParticles);
                AdjustCone6(12, 8, 9, 14, mTransform, g_aBaseProjScaleX[2], g_aBaseProjScaleY[2], m_pParticles);

                // Right leg
                ZVector3 vLegR = m_pParticles[13].x - m_pParticles[9].x;
                vnorm(vLegR);
                mLimb.SetRow(0, vLegR);

                ZVector3 vLegRCross;
                vcross(vLegRCross, vPelvisDir, vLegR);
                vnorm(vLegRCross);
                mLimb.SetRow(2, vLegRCross);

                ZVector3 vLegRUp;
                vcross(vLegRUp, vLegR, vLegRCross);
                mLimb.SetRow(1, vLegRUp);

                mmmul(mTransform.data, g_aBaseTransformBasis[3][0], mLimb.data);
                AdjustCone5(13, 9, 8, 15, mTransform, g_aBaseProjScaleX[3], g_aBaseProjScaleY[3], m_pParticles);
                AdjustCone6(13, 9, 8, 15, mTransform, g_aBaseProjScaleX[3], g_aBaseProjScaleY[3], m_pParticles);

                // Hand / chest
                ZVector3 vShoulderDir = m_pParticles[3].x - m_pParticles[2].x;
                vnorm(vShoulderDir);
                mLimb.SetRow(0, vShoulderDir);

                ZVector3 vChestMid = (m_pParticles[2].x + m_pParticles[3].x) * 0.5f;
                ZVector3 vChestUp = vChestMid - m_pParticles[5].x - m_pParticles[6].x;
                vnorm(vChestUp);

                ZVector3 vChestCross;
                vcross(vChestCross, vChestUp, vShoulderDir);
                mLimb.SetRow(2, vChestCross);

                ZVector3 vChestForward;
                vcross(vChestForward, vShoulderDir, vChestCross);
                mLimb.SetRow(1, vChestForward);

                mmmul(mTransform.data, g_aBaseTransformBasis[4][0], mLimb.data);
                AdjustPlane2(3, 7, mTransform.data, m_pParticles);
                AdjustCone4(3, 6, 1, 7, mTransform, g_aBaseProjScaleX[4], g_aBaseProjScaleY[4], m_pParticles);

                // Left arm
                ZVector3 vArmL = m_pParticles[7].x - m_pParticles[3].x;
                vnorm(vArmL);
                mLimb.SetRow(0, vArmL);

                ZVector3 vArmLCross;
                vcross(vArmLCross, vPelvisDir, vArmL);
                vnorm(vArmLCross);
                mLimb.SetRow(2, vArmLCross);

                ZVector3 vArmLUp;
                vcross(vArmLUp, vArmL, vArmLCross);
                mLimb.SetRow(1, vArmLUp);

                mmmul(mTransform.data, g_aBaseTransformBasis[5][0], mLimb.data);
                AdjustCone6(7, 3, 6, 11, mTransform, g_aBaseProjScaleX[5], g_aBaseProjScaleY[5], m_pParticles);

                // Correct volume/rot for l-hand
                for (int subIter = 0; subIter < 4; ++subIter)
                {
                    EnforceCrossDotConstraint(3, 7, 6, 11);
                    EnforceCrossDotConstraint(3, 6, 1, 11);
                }

                // Symmetric correction for R-arm
                ZMat3x3 mLimbReflect;
                vmuls((float*)&mLimbReflect.data[0], (const float*)&mLimb.data[0], -1.0f);
                vmuls((float*)&mLimbReflect.data[3], (const float*)&mLimb.data[3], -1.0f);
                mLimbReflect.data[6] = mLimb.data[6];
                mLimbReflect.data[7] = mLimb.data[7];
                mLimbReflect.data[8] = mLimb.data[8];

                mmmul(mTransform.data, g_aBaseTransformBasis[6][0], mLimbReflect.data);
                AdjustPlane2(2, 4, mTransform.data, m_pParticles);
                AdjustCone4(2, 5, 1, 4, mTransform, g_aBaseProjScaleX[6], g_aBaseProjScaleY[6], m_pParticles);

                // --- Right forearm ---
                ZVector3 vArmR = m_pParticles[4].x - m_pParticles[2].x;
                vnorm(vArmR);
                mLimb.SetRow(0, vArmR);

                ZVector3 vArmRCross;
                vcross(vArmRCross, vPelvisDir, vArmR);
                vnorm(vArmRCross);
                mLimb.SetRow(2, vArmRCross);

                ZVector3 vArmRUp;
                vcross(vArmRUp, vArmR, vArmRCross);
                mLimb.SetRow(1, vArmRUp);

                mmmul(mTransform.data, g_aBaseTransformBasis[7][0], mLimb.data);
                AdjustCone6(4, 2, 5, 10, mTransform, g_aBaseProjScaleX[7], g_aBaseProjScaleY[7], m_pParticles);

                // Correct volume/rot for r-hand
                for (int subIter = 0; subIter < 4; ++subIter)
                {
                    EnforceCrossDotConstraint(2, 5, 4, 10);
                    EnforceCrossDotConstraint(2, 1, 5, 10);
                }

                // --- Prevent knees and feet intersection ---
                const ZVector3& vPelvisAxis = *(const ZVector3*)&mBasis.data[3]; // Y-axis of pelvis

                // Knees (12 and 13)
                ZVector3 vKneeDiff = m_pParticles[13].x - m_pParticles[12].x;
                float fKneeDist = vdot(vPelvisAxis, vKneeDiff);
                if (fKneeDist < 10.0f)
                {
                    float fHalfDist = fKneeDist * 0.5f;
                    float fPushL = (fHalfDist - 5.0f) * 0.1f;
                    float fPushR = (5.0f - fHalfDist) * 0.1f;

                    vaddscalar(m_pParticles[12].x, m_pParticles[12].x, vPelvisAxis, fPushL);
                    vaddscalar(m_pParticles[13].x, m_pParticles[13].x, vPelvisAxis, fPushR);
                }

                // Feet (14 and 15)
                ZVector3 vFeetDiff = m_pParticles[15].x - m_pParticles[14].x;
                float fFeetDist = vdot(vPelvisAxis, vFeetDiff);
                if (fFeetDist < 10.0f)
                {
                    float fHalfDist = fFeetDist * 0.5f;
                    float fPushL = (fHalfDist - 5.0f) * 0.1f;
                    float fPushR = (5.0f - fHalfDist) * 0.1f;

                    vaddscalar(m_pParticles[14].x, m_pParticles[14].x, vPelvisAxis, fPushL);
                    vaddscalar(m_pParticles[15].x, m_pParticles[15].x, vPelvisAxis, fPushR);
                }
            }

            // 2. Standard distance constraint relaxation (springs/rods)
            for (int subIter = 0; subIter < 4; ++subIter)
            {
                // Special constraints
                for (int i = 0; i < m_iNumSpecialConstraints; ++i)
                {
                    ParticleConstraint& sc = m_pSpecialConstraints[i];
                    AdjustSpecialConstraint(
                        sc.m_pPar1->x,
                        sc.m_pPar2->x,
                        sc.m_fDist,
                        sc.m_pPar1->mass,
                        sc.m_pPar2->mass
                    );
                }

                // Base distance constraints
                for (int j = 0; j < m_iNumConstraints; ++j)
                {
                    ParticleConstraint& c = m_pConstraints[j];
                    AdjustPart2(
                        c.m_pPar1->x,
                        c.m_pPar2->x,
                        c.m_fDist,
                        c.m_pPar1->mass,
                        c.m_pPar2->mass
                    );
                }
            }
        }
    }

    void ConstrainedParticleSystem::FindFaces()
    {
        ZVector3 vMin { 10000000.0f, 10000000.0f, 10000000.0f };
        ZVector3 vMax { -10000000.0f, -10000000.0f, -10000000.0f };

        for (int i = 0; i < m_iNumParticles; ++i)
        {
            Particle* p = &m_pParticles[i];
            vmin(vMin, p->ok_x);
            vmax(vMax, p->ok_x);
            vmin(vMin, p->x);
            vmax(vMax, p->x);
        }

        vMin.x -= 15.0f;
        vMin.y -= 15.0f;
        vMin.z -= 15.0f;
        vMax.x += 15.0f;
        vMax.y += 15.0f;
        vMax.z += 15.0f;

        ZVector3 vCen;
        vadd(vCen, vMin, vMax);
        vscalar(vCen, 0.5f);

        ZVector3 vSize;
        vsub(vSize, vMax, vMin);
        vscalar(vSize, 0.5f);

        if (m_pFastBox)
        {
            ZMat3x3 mIdentity;
            mIdentity.Reset();

            float fElevDtY = 0.0f;
            if (m_pLnkObj && m_pLnkObj->IsInElevator())
            {
                fElevDtY = m_pLnkObj->GetElevatorDeltaY();
            }

            m_pFastBox->SetBox(vCen, mIdentity, vSize, std::fabs(fElevDtY) > 0.050000001f);
        }
        else
        {
            ZMat3x3 mIdentity;
            mIdentity.Reset();

            m_pFastBox = ZUniMemory::New<ZFastBoxColi>(10.0f, 32);
            m_pFastBox->m_bExtendedMode = true;
            m_pFastBox->SetBox(vCen, mIdentity, vSize, true);
        }

        if (m_pWaterBoxManager)
        {
            m_bReallyInWaterOld = m_bReallyInWater;
            m_bInWater = false;
            m_bReallyInWater = false;

            const uint32_t lWaterBoxes = m_pWaterBoxManager->Count();
            for (uint32_t i = 0; i < lWaterBoxes; ++i)
            {
                ZWaterBox* pWaterBox = (*m_pWaterBoxManager)[i];
                ZBaseGeom* pBaseGeom = pWaterBox->BaseGeom();

                ZASSERT(pBaseGeom->IsDerivedFrom<ZSTDOBJ>());

                ZVector3 vWaterCen;
                pBaseGeom->GetCen(vWaterCen);
                pBaseGeom->GetRootPoint(vWaterCen);

                ZVector3 vWaterSize;
                pBaseGeom->GetSize(vWaterSize);

                ZVector3 vWaterMin;
                vsub(vWaterMin, vWaterCen, vWaterSize);

                ZVector3 vWaterMax;
                vadd(vWaterMax, vWaterCen, vWaterSize);

                bool abOverlap[3] = {};
                for (int k = 0; k < 3; ++k)
                {
                    abOverlap[k] = vWaterMin[k] <= vMax[k] && vMin[k] <= vWaterMax[k];
                }

                if (abOverlap[0] && abOverlap[1] && abOverlap[2])
                {
                    m_bInWater = true;
                    return;
                }
            }
        }
    }

    void ConstrainedParticleSystem::ResetVelocities()
    {
        for (int i = 0; i < m_iNumParticles; ++i)
        {
            Particle* p = &m_pParticles[i];
            p->v = {};
            p->oldx = p->x;
        }
    }

    void ConstrainedParticleSystem::SetNumParticles(int32_t lNrParticles, bool a2)
    {
        if (m_iNumParticles < lNrParticles && a2)
        {
            ZERROR("ConstrainedParticleSystem::SetNumParticles(..): Number of particles too high");
        }

        m_iNumParticles = lNrParticles;
    }

    void ConstrainedParticleSystem::EnforceCrossDotConstraint(int iIdx0, int iIdx1, int iIdx2, int iIdx3)
    {
        Particle& p0 = m_pParticles[iIdx0];
        Particle& p1 = m_pParticles[iIdx1];
        Particle& p2 = m_pParticles[iIdx2];
        Particle& p3 = m_pParticles[iIdx3];

        ZVector3 v1, v2, v3;
        vsub(v1, p1.x, p0.x);
        vsub(v2, p2.x, p0.x);
        vsub(v3, p3.x, p0.x);

        // grads of weight changes
        ZVector3 vGrad3, vGrad1, vGrad2;
        vcross(vGrad3, v1, v2); // vnorm for face (0, 1, 2) -> grad(p3)
        vcross(vGrad1, v2, v3); // vnorm for face (0, 2, 3) -> grad(p1)
        vcross(vGrad2, v3, v1); // vnorm for face (0, 3, 1) -> grad(p2)

        // Find den (squared grad mag)
        ZVector3 vTemp1, vTemp2;
        vcross(vTemp1, v1, vGrad2);
        vcross(vTemp2, vGrad1, v2);

        float fDenom = 0.0f;
        fDenom += vdot(vTemp1, v3);    // |vGrad2|^2
        fDenom += vdot(vTemp2, v3);    // |vGrad1|^2
        fDenom += vdot(vGrad3, vGrad3);// |vGrad3|^2

        // Volume restrictions
        const float fVolumeErr = -vdot(vGrad3, v3);

        // volume is negative
        if (fVolumeErr < 0.0f)
        {
            if (fabsf(fDenom) > 0.00001f)
            {
                const float fFactor = (fVolumeErr * 0.35f) / fDenom;

                // Find accumulated impulse for p0 (for save center of mass by sum(vGrad) == 0)
                ZVector3 vGrad0Sum;
                vmuls(vGrad0Sum, vGrad1, fFactor);
                vaddscalar(vGrad0Sum, vGrad0Sum, vGrad2, fFactor);
                vaddscalar(vGrad0Sum, vGrad0Sum, vGrad3, fFactor);

                // Correct edges
                vaddscalar(p1.x, p1.x, vGrad1, fFactor);
                vaddscalar(p2.x, p2.x, vGrad2, fFactor);
                vaddscalar(p3.x, p3.x, vGrad3, fFactor);
                vsub(p0.x, p0.x, vGrad0Sum);
            }
        }
    }

    void ConstrainedParticleSystem::GetVelocity(int lIndex, ZVector3& vVelocity) const
    {
        vVelocity = m_pParticles[lIndex].v;
    }

    void ConstrainedParticleSystem::HandleCollision(SRagdollCollisionInfo& sCollisionInfo)
    {
        REFTAB* pFaceList = m_pFastBox->m_pFaceList;

        auto handleParticleCollision = [&](Particle* pParticle)
        {
            if (!pFaceList)
                return;

            RefRun run;
            pFaceList->RunInitNxtRef(&run);

            for (uint32_t* pRef = pFaceList->RunNxtRefPtr(&run);
                 pRef;
                 pRef = pFaceList->RunNxtRefPtr(&run))
            {
                const SFastBoxColiTri* pTri = reinterpret_cast<const SFastBoxColiTri*>(pRef);

                ZVector3 vDir;
                vsub(vDir, pParticle->x, pParticle->ok_x);

                ZVector3 vHit;
                float t;

                if (ZCommonAlgorithms::IntersectTriangleAndLine3(
                        vHit.Get(), pParticle->ok_x.Get(), vDir.Get(),
                        pTri->m_avVerts[0].Get(), pTri->m_mTri.Get(), &t, false))
                {
                    pParticle->x = vHit;

                    ZVector3 vNewCenter;
                    ZVector3 vNewVel;

                    ZCommonAlgorithms::ProjectSphereOutFromPlane(
                        vNewCenter.Get(), pParticle->x.Get(), vNewVel.Get(),
                        pParticle->v.Get(), pTri->m_vTriNorm.Get(), 15.0f);

                    pParticle->x = vNewCenter;
                    pParticle->v = vNewVel;
                }
            }
        };

        handleParticleCollision(&m_pParticles[5]);

        if (m_bFollow)
        {
            CollideLineBetweenParticles3(sCollisionInfo);
        }

        handleParticleCollision(&m_pParticles[5]);
    }

    void ConstrainedParticleSystem::CollideLineBetweenParticles3(SRagdollCollisionInfo& sCollisionInfo)
    {
        REFTAB* pFaceList = m_pFastBox->m_pFaceList;
        if (!pFaceList)
            return;

        RefRun run;
        pFaceList->RunInitNxtRef(&run);

        for (uint32_t* pRef = pFaceList->RunNxtRefPtr(&run);
             pRef;
             pRef = pFaceList->RunNxtRefPtr(&run))
        {
            const SFastBoxColiTri* pTri = reinterpret_cast<const SFastBoxColiTri*>(pRef);

            Particle* pPar1 = &m_pParticles[1];
            Particle* pPar5 = &m_pParticles[5];

            // Build an extended capsule so particle 1 lies at the 2/3 point of the segment.
            ZVector3 vPos = pPar1->x;
            ZVector3 vDiff;
            vsub(vDiff, vPos, pPar5->x);
            vaddscalar(vPos, vPos, vDiff, 0.5f);

            ZVector3 vVel = pPar1->v;
            vsub(vDiff, vVel, pPar5->v);
            vaddscalar(vVel, vVel, vDiff, 0.5f);

            for (int i = 0; i < 2; ++i)
            {
                SCapsuleColiInfo result;

                if (!ZCommonAlgorithms::CollideCapsuleAndTriangle(
                        reinterpret_cast<const float(&)[3]>(pPar5->x),
                        reinterpret_cast<const float(&)[3]>(vPos),
                        6.0f, pTri, result)
                    || result.fScaledDist * result.fScaledDist <= 0.000001f)
                {
                    break;
                }

                if (result.t0 == 1.0f)
                {
                    ZCommonAlgorithms::PullTriangleCyl2(pPar5->x.Get(), vPos.Get(), pPar5->v.Get(), vVel.Get(), result);

                    vaddscalar(vDiff, pPar5->x, vPos, 2.0f);
                    vscalar(pPar1->x, vDiff, 0.33333334f);
                    vaddscalar(vDiff, pPar5->v, vVel, 2.0f);
                    vscalar(pPar1->v, vDiff, 0.33333334f);

                    sCollisionInfo.sPartColi[1].bCollision = true;
                    sCollisionInfo.sPartColi[1].rGeom = pTri->m_rGeom;
                }
            }

            for (int k = 0; k < 15; ++k)
            {
                Particle* pA = &m_pParticles[g_lSausageA[k]];
                Particle* pB = &m_pParticles[g_lSausageB[k]];

                int j = 0;
                for (; j < 2; ++j)
                {
                    SCapsuleColiInfo result;

                    if (!ZCommonAlgorithms::CollideCapsuleAndTriangle(
                            reinterpret_cast<const float(&)[3]>(pA->x),
                            reinterpret_cast<const float(&)[3]>(pB->x),
                            g_lSausageRadius[k], pTri, result)
                        || result.fScaledDist * result.fScaledDist <= 0.000001f)
                    {
                        break;
                    }

                    ZCommonAlgorithms::PullTriangleCyl2(pA->x.Get(), pB->x.Get(), pA->v.Get(), pB->v.Get(), result);

                    if (result.fScaledDist > 6.0f)
                    {
                        pA->v = {};
                        pB->v = {};
                    }
                }

                if (j)
                {
                    int partIndex = -1;
                    switch (k)
                    {
                        case 0:  partIndex = 0; break;
                        case 7:  partIndex = 5; break;
                        case 8:  partIndex = 4; break;
                        case 11: partIndex = 3; break;
                        case 12: partIndex = 2; break;
                        default: break;
                    }

                    if (partIndex >= 0)
                    {
                        sCollisionInfo.sPartColi[partIndex].bCollision = true;
                        sCollisionInfo.sPartColi[partIndex].rGeom = pTri->m_rGeom;
                    }
                }
            }
        }
    }

    float ConstrainedParticleSystem::MoveRagdoll(SRagdollCollisionInfo& sCollisionInfo, float fTimeDt, bool bHasFixedParts)
    {
        m_fPrevTimeStep = fTimeDt;

        const int nSubSteps = bHasFixedParts ? 3 : 4;
        const float fSubSteps = static_cast<float>(nSubSteps);
        const float fTimeMul = fTimeDt * 50.0f;

        for (int i = 0; i < m_iNumParticles; ++i)
        {
            Particle& par = m_pParticles[i];
            vaddscalar(par.oldx, par.x, par.v, -fTimeMul / fSubSteps);
        }

        const float fDamping = powf(1.0f - m_fDamping, fTimeMul / fSubSteps);
        const float fGravity = static_cast<float>(g_lGravity.m_iValue);
        const float fGravityStep = (fTimeDt * fTimeDt + fTimeDt * fTimeDt) / (fSubSteps * fSubSteps) * fGravity * 0.66666698f;

        for (int iSubStep = 0; iSubStep < nSubSteps; ++iSubStep)
        {
            ZeroOut(3);

            if (m_bInWater)
            {
                m_bReallyInWater = false;

                for (int i = 0; i < m_iNumParticles; ++i)
                {
                    Particle& par = m_pParticles[i];
                    const ZVector3 vSavedX = par.x;

                    if (par.mass < 10000.0f)
                    {
                        ZWaterBox* pFoundBox = nullptr;
                        float fSurfaceY = 0.0f;

                        if (m_pWaterBoxManager)
                        {
                            const uint32_t lWaterBoxes = m_pWaterBoxManager->Count();
                            for (uint32_t k = 0; k < lWaterBoxes; ++k)
                            {
                                ZWaterBox* pWaterBox = (*m_pWaterBoxManager)[k];

                                ZVector3 vRoot;
                                pWaterBox->GetCen(vRoot);
                                pWaterBox->GetRootPoint(vRoot);

                                ZVector3 vSize;
                                pWaterBox->GetSize(vSize);

                                ZVector3 vMin;
                                vsub(vMin, vRoot, vSize);

                                ZVector3 vMax;
                                vadd(vMax, vRoot, vSize);

                                const int iWaveTicks = static_cast<int>((par.x.x + par.x.z) * 20.48f);
                                const float fWavePhase = static_cast<float>(g_pSysInterface->FrameTime.secs + iWaveTicks);
                                const float fWave = sinf(fWavePhase * TIMETYPE::kInvTPS) * 7.0f + 3.0f;
                                fSurfaceY = vMax.y - fWave;

                                if (par.x.x >= vMin.x && par.x.x <= vMax.x
                                    && par.x.y >= vMin.y && par.x.y <= fSurfaceY + 25.0f
                                    && par.x.z >= vMin.z && par.x.z <= vMax.z)
                                {
                                    pFoundBox = pWaterBox;
                                    break;
                                }
                            }
                        }

                        if (pFoundBox)
                        {
                            m_bReallyInWater = true;
                            sCollisionInfo.bInWater = true;
                            sCollisionInfo.rWaterBox = pFoundBox->GetRef();

                            ZVector3 vDelta;
                            vsub(vDelta, par.x, par.oldx);
                            vscalar(par.v, vDelta, 0.9f);
                            vadd(par.x, par.v);

                            float fSurfaceFactor = (par.x.y - fSurfaceY) / 25.0f;
                            if (fSurfaceFactor < -0.5f)
                                fSurfaceFactor = -0.5f;
                            else if (fSurfaceFactor > 1.0f)
                                fSurfaceFactor = 1.0f;

                            ZVector3 vCurrent(pFoundBox->m_vCurrent[0], pFoundBox->m_vCurrent[1], pFoundBox->m_vCurrent[2]);
                            if (vlen2(vCurrent) > 0.0f)
                            {
                                pFoundBox->GetRootVect(vCurrent);
                                vaddscalar(par.x, par.x, vCurrent, fTimeDt / fSubSteps);
                            }
                            else
                            {
                                par.x.y -= fSurfaceFactor * fGravityStep;
                            }
                        }
                        else
                        {
                            ZVector3 vDelta;
                            vsub(vDelta, par.x, par.oldx);
                            vscalar(par.v, vDelta, fDamping);
                            vadd(par.x, par.v);
                            par.x.y -= fGravityStep;
                        }
                    }

                    par.oldx = vSavedX;
                }
            }
            else
            {
                for (int i = 0; i < m_iNumParticles; ++i)
                {
                    Particle& par = m_pParticles[i];

                    ZVector3 vDelta;
                    vsub(vDelta, par.x, par.oldx);
                    par.oldx = par.x;

                    if (par.mass < 10000.0f)
                    {
                        vaddscalar(par.x, par.x, vDelta, fDamping);
                        par.x.y -= fGravityStep;
                    }
                }
            }

            for (int i = 0; i < m_iNumParticles; ++i)
            {
                Particle& par = m_pParticles[i];
                vsub(par.v, par.x, par.oldx);
            }

            FindFaces();
            HandleCollision(sCollisionInfo);

            for (int i = 0; i < m_iNumParticles; ++i)
            {
                Particle& par = m_pParticles[i];
                vsub(par.oldx, par.x, par.v);
                par.ok_x = par.x;
            }
        }

        ZVector3 vSum(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < m_iNumParticles; ++i)
        {
            Particle& par = m_pParticles[i];
            vsub(par.v, par.x, par.oldx);
            vscalar(par.v, fSubSteps / fTimeMul);
            vadd(vSum, par.v);
        }

        return vlen(vSum) / fTimeDt;
    }

    STATIC_CLASS_VAR_IMPL(ConstrainedParticleSystem, ZWaterBoxManager*, m_pWaterBoxManager, 0x009A33D0, nullptr);
}
