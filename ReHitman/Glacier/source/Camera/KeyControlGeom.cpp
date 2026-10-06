#include <Glacier/Camera/KeyControlGeom.h>

#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/System/CConfiguration.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    namespace
    {
        Action::ZHandle a_TiltCamera { "TiltCamera" };
        Action::ZHandle a_TurnCamera { "TurnCamera" };
        Action::ZHandle a_MoveX { "MoveX" };
        Action::ZHandle a_MoveY { "MoveY" };
        Action::ZHandle a_Mode1 { "Mode1" };
        Action::ZHandle a_Mode2 { "Mode2" };
        Action::ZHandle a_Mode3 { "Mode3" };
        Action::ZHandle a_Mode4 { "Mode4" };
        Action::ZHandle a_Mode1MoveX { "Mode1MoveX" };
        Action::ZHandle a_Mode1MoveY { "Mode1MoveY" };
        Action::ZHandle a_Mode1MoveZ { "Mode1MoveZ" };
        Action::ZHandle a_Mode1MouseL { "Mode1MouseL" };
        Action::ZHandle a_Mode1MouseM { "Mode1MouseM" };
        Action::ZHandle a_Mode1MouseR { "Mode1MouseR" };
        Action::ZHandle a_Turbo { "Turbo" };

        float g_fSmoothVelX = 0.0f; // PC flt_9ACF58
        float g_fSmoothVelY = 0.0f; // PC flt_9ACF5C
        float g_fSmoothVelZ = 0.0f; // PC flt_9ACF60
    }

    ZDebugInt g_iFreeCamMode { "FreeCamMode", "Controlmode of freecam", 0, -1000000, 1000000, 1, nullptr }; // PC 009ACD14

    void MouseConVect1(float* vDir, const float* pOldMouse, const float* pNewMouse, float fSensX, float fSensY)
    {
        // Horizontal (XZ) component of the view direction
        float h[3] = { vDir[0], 0.0f, vDir[2] };
        const float fLen = vnorm(h);

        // Current vertical angle, so the pitch can be clamped to avoid flipping
        const float fCosPitch = h[0] * vDir[0] + h[1] * vDir[1] + h[2] * vDir[2];
        const float fSinPitch = vDir[1];
        float fAngle = GetAngle(fCosPitch, fSinPitch);
        GetAngle(h[2], h[0]);

        float rot[3] = { 0.0f, -fLen, 0.0f };
        vrot(vDir, rot);

        float fMin, fMax;
        if (fSinPitch >= 0.0f)
        {
            fMin = -1.5533431f - fAngle;
            fMax = 1.5533431f - fAngle;
        }
        else
        {
            fMin = 4.7298422f - fAngle;
            fMax = 7.8365288f - fAngle;
        }

        // Vertical mouse delta -> pitch
        float fPitch = (pOldMouse[1] - pNewMouse[1]) * fSensY;
        if (CConfiguration::m_bInvertVertical[0])
        {
            fPitch = -fPitch;
        }
        if (fPitch < fMin)
        {
            fPitch = fMin;
        }
        if (fMax < fPitch)
        {
            fPitch = fMax;
        }

        rot[0] = -fPitch;
        rot[1] = 0.0f;
        rot[2] = 0.0f;
        vrot(vDir, rot);

        rot[0] = 0.0f;
        rot[1] = fLen;
        rot[2] = 0.0f;
        vrot(vDir, rot);

        // Horizontal mouse delta -> yaw
        rot[0] = 0.0f;
        rot[1] = 0.0f;
        rot[2] = (pNewMouse[0] - pOldMouse[0]) * fSensX;
        vrot(vDir, rot);
    }

    void KeyControlGeom(ZBaseGeom* pGeom, float fMultiplier)
    {
        if (!pGeom)
        {
            return;
        }

        float fTimeDelta = g_pSysInterface->m_fActualTimeDelta;
        const float fTimeMultiplier = g_pSysInterface->GetTimeMultiplier();
        if (fTimeMultiplier > 0.0f)
        {
            fTimeDelta = fTimeDelta / fTimeMultiplier;
        }

        ZMat3x3 to;
        ZVector3 vPos;

        // ------------------------------------------------------------------
        // Mode 0: free flight
        // ------------------------------------------------------------------
        if (g_iFreeCamMode.m_iValue == 0)
        {
            pGeom->GetMatPos(to, vPos);

            // _position = rotation request built from the MoveX/MoveY keys and mode keys
            ZVector3 vRot;
            vRot.x = a_MoveY.Analog() * a_Mode1.Analog();
            vRot.y = 0.0f - a_MoveX.Analog() * a_Mode3.Analog();
            vRot.z = a_Mode1.Analog() * a_MoveX.Analog();

            // a1 = translation request (strafe/forward) built from the same keys
            ZVector3 vMove;
            float fTurbo = a_Turbo.Analog() * 6.0f + 1.0f;
            vMove.x = a_MoveX.Analog() * (a_Mode2.Analog() * fTurbo);
            fTurbo = a_Turbo.Analog() * 6.0f + 1.0f;
            vMove.x += a_MoveX.Analog() * (a_Mode4.Analog() * fTurbo);
            fTurbo = a_Turbo.Analog() * 6.0f + 1.0f;
            vMove.y = a_MoveY.Analog() * (a_Mode2.Analog() * fTurbo);
            fTurbo = a_Turbo.Analog() * 6.0f + 1.0f;
            vMove.z = 0.0f - a_MoveY.Analog() * (a_Mode3.Analog() * fTurbo);
            fTurbo = a_Turbo.Analog() * 6.0f + 1.0f;
            vMove.z -= a_MoveY.Analog() * (a_Mode4.Analog() * fTurbo);

            vscalar(&vRot.x, fTimeDelta * 200.0f);
            vscalar(&vMove.x, fTimeDelta * fMultiplier);

            a_TurnCamera.Analog();
            a_TiltCamera.Analog();

            if (vMove.x != 0.0f || vMove.y != 0.0f || vMove.z != 0.0f || !vzero(&vRot.x))
            {
                vscalar(&vMove.x, 4.0f);
                TransformRootVector(vMove, to);
                vPos += vMove;

                // Mouse look: current mouse position is fed relative to an origin
                float vZero[3] = { 0.0f, 0.0f, 0.0f };
                float vMouse[3] = { -vRot.y, vRot.x, 0.0f };
                ZVector3 vDir(0.0f, 0.0f, 1.0f);
                MouseConVect1(&vDir.x, vZero, vMouse, 0.01f, 0.01f);
                TransformRootVector(vDir, to);

                // Roll the up vector by the remaining rotation request
                ZVector3 vUp(0.0f, 1.0f, 0.0f);
                float rot[3] = { 0.0f, 0.0f, vRot.z * 0.01f };
                vrot(&vUp.x, rot);
                TransformRootVector(vUp, to);

                createmat(&to.data[0], &vDir.x, &vUp.x);
            }

            pGeom->SetMatPos(&to.data[0], &vPos.x);
            return;
        }

        // ------------------------------------------------------------------
        // Modes 1/2: translate along the camera axes (mode 2 smooths the motion)
        // ------------------------------------------------------------------
        if (g_iFreeCamMode.m_iValue == 1 || g_iFreeCamMode.m_iValue == 2)
        {
            pGeom->GetMatPos(to, vPos);

            ZVector3 vMove;
            vMove.x = a_Mode1MoveX.Analog();
            vMove.y = a_Mode1MoveY.Analog();
            vMove.z = a_Mode1MoveZ.Analog();

            if (g_iFreeCamMode.m_iValue == 2 || !vzero(&vMove.x))
            {
                const float fSpeed = fTimeDelta * fMultiplier + fTimeDelta * fMultiplier;
                vscalar(&vMove.x, fSpeed);

                ZVector3 vWorld = vMove;
                TransformRootVector(vWorld, to);
                vWorld.y = vMove.y; // keep vertical in world space

                if (g_iFreeCamMode.m_iValue == 2)
                {
                    const float fDt = fTimeDelta;
                    float fOldX = g_fSmoothVelX - (g_fSmoothVelX * fDt + g_fSmoothVelX * fDt);
                    float fOldY = g_fSmoothVelY - (g_fSmoothVelY * fDt + g_fSmoothVelY * fDt);
                    float fOldZ = g_fSmoothVelZ - (g_fSmoothVelZ * fDt + g_fSmoothVelZ * fDt);

                    g_fSmoothVelX = (vWorld.x - fOldX) * (fDt * 4.0f) + fOldX;
                    g_fSmoothVelY = (vMove.y - fOldY) * (fDt * 4.0f) + fOldY;
                    g_fSmoothVelZ = (vWorld.z - fOldZ) * (fDt * 4.0f) + fOldZ;

                    if (g_fSmoothVelX != 0.0f || g_fSmoothVelY != 0.0f || g_fSmoothVelZ != 0.0f)
                    {
                        vPos.x += g_fSmoothVelX;
                        vPos.y += g_fSmoothVelY;
                        vPos.z += g_fSmoothVelZ;
                        pGeom->SetPos(&vPos.x);
                    }
                }
                else
                {
                    vPos += vWorld;
                    pGeom->SetPos(&vPos.x);
                }
            }

            return;
        }

        // ------------------------------------------------------------------
        // Mode 3: translate along X/Z (stays on the current height plane)
        // ------------------------------------------------------------------
        if (g_iFreeCamMode.m_iValue == 3)
        {
            pGeom->GetMatPos(to, vPos);

            ZVector3 vMove;
            vMove.x = a_Mode1MoveX.Analog();
            vMove.y = 0.0f;
            vMove.z = a_Mode1MoveZ.Analog();

            if (vzero(&vMove.x))
            {
                return;
            }

            const float fSpeed = fTimeDelta * fMultiplier * fMultiplier * 0.0099999998f;
            vscalar(&vMove.x, fSpeed);

            ZVector3 vWorld = vMove;
            TransformRootVector(vWorld, to);

            if (!a_Mode1MouseM.Digital())
            {
                vWorld.y = vMove.y;
            }

            vPos += vWorld;
            pGeom->SetPos(&vPos.x);
            return;
        }

        // ------------------------------------------------------------------
        // Mode 4: translate + vertical from the mouse buttons
        // ------------------------------------------------------------------
        if (g_iFreeCamMode.m_iValue == 4)
        {
            pGeom->GetMatPos(to, vPos);

            ZVector3 vMove;
            vMove.x = a_Mode1MoveX.Analog();
            vMove.y = (a_Mode1MouseR.Digital() ? 1.0f : 0.0f) - (a_Mode1MouseL.Digital() ? 1.0f : 0.0f);
            vMove.y *= 0.5f;
            vMove.z = a_Mode1MoveZ.Analog();

            if (vzero(&vMove.x))
            {
                return;
            }

            const float fSpeed = fTimeDelta * fMultiplier * fMultiplier * 0.0099999998f;
            vscalar(&vMove.x, fSpeed);

            ZVector3 vWorld = vMove;
            TransformRootVector(vWorld, to);

            if (!a_Mode1MouseM.Digital())
            {
                vWorld.y = vMove.y; // vertical is kept in world space unless grabbed by the middle button
            }

            vPos += vWorld;
            pGeom->SetPos(&vPos.x);
        }
    }
}
