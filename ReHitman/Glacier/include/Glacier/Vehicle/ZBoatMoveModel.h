#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Vehicle/BaseMoveModel.h>
#include <Glacier/Vehicle/ZBoatMoveModelIF.h>


namespace Glacier
{
    class ZBoatMoveModel : public BaseMoveModel, public ZBoatMoveModelIF
    {
    public:
        // vtbl
        // data
        ZVector3 m_vPos;
        ZMat3x3 m_mMat;
        float m_fAngle;
        bool m_bReversing;
        RE_ADD_PADDING(3);
        float m_fSteerAngle;
        int m_iFrontFindGroundCounte;
        int m_iBackFindGroundCounter;
        float m_fPreUpdateTime;
        ZVector3 m_vHorDir;
        ZVector3 m_vOldHorDir;
        float m_fTurnAngle;
        float m_fTurnAngleSpd;
        float m_fGravityAccelParam;
        float m_fGravityTurnSpdParam;
        float m_fTimeStep;
    };
    RE_VERIFY_SIZE(ZBoatMoveModel, 0x8C);
    RE_VERIFY_OFFSET(ZBoatMoveModel, m_vPos, 0x18);
}