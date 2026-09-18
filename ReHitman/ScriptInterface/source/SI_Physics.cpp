#include <SI/SI_Physics.h>
#include <Glacier/Physics/SRigidBodyVelocity.h>
#include <Glacier/Physics/SBodyGMR.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Data/ZEngineDataBase.h>


namespace Glacier
{
    bool Physics__Releasebone(ZREF rGeom, v3 vTarget, float fRadius)
    {
        auto* pGeom = ZGEOM::RefToPtr(rGeom);
        if (!pGeom)
            return false;

        const auto lMsgId = g_pEngineData->RegisterZMsg("Release", 0, __FILE__, __LINE__);
        SBodyGMR sGMR {};
        sGMR.radius = fRadius;
        sGMR.target = vTarget;
        sGMR.par = 0;
        sGMR.grabbing = false;
        sGMR.strained = 0.0f;
        sGMR.dir = {};

        pGeom->SendCommand(lMsgId, &sGMR, nullptr);
        return true;
    }

    bool Physics__Setvelocity(ZREF rGeom, float xRotation, float yRotation, float zRotation, v3 vPos)
    {
        auto* pGeom = ZGEOM::RefToPtr(rGeom);
        if (!pGeom)
            return false;

        SRigidBodyVelocity sRBV {};

        sRBV.m_mOldMat.Reset();
        sRBV.m_vPos = vPos;
        sRBV.m_vOldPos.Reset();
        sRBV.m_mMat.Reset();

        mrotaxis(sRBV.m_mMat, xRotation, 1.0, 0.0, 0.0);
        mrotaxis(sRBV.m_mMat, yRotation, 0.0, 1.0, 0.0);
        mrotaxis(sRBV.m_mMat, zRotation, 0.0, 0.0, 1.0);

        const auto lMsgId = g_pEngineData->RegisterZMsg("SetVelocity", 0, __FILE__, __LINE__);

        pGeom->SendCommand(lMsgId, &sRBV, nullptr);
        return true;
    }
}
