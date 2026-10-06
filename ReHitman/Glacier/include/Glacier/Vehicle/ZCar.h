#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Vehicle/ZDriving.h>
#include <Glacier/Vehicle/PathMover.h>
#include <Glacier/Vehicle/OrgCarMoveModel.h>
#include <Glacier/Vehicle/ZCarMoveModelIF.h>

namespace Glacier
{
    class ZCar : public ZDriving, public ZCarMoveModelIF
    {
    public:
        // vtbl (no changes)
        // data
        OrgCarMoveModel m_CarMoveModel;
        PathMover m_PathMover;
        ZVector3 m_vInitialPos;
        ZMat3x3 m_mInitialMat;
    };
    RE_VERIFY_SIZE(ZCar, 0x6FC); // Verified
    RE_VERIFY_OFFSET(ZCar, m_CarMoveModel, 0x208);
    RE_VERIFY_OFFSET(ZCar, m_PathMover, 0x36c);
    RE_VERIFY_OFFSET(ZCar, m_vInitialPos, 0x6cc);
    RE_VERIFY_OFFSET(ZCar, m_mInitialMat, 0x6d8);
}