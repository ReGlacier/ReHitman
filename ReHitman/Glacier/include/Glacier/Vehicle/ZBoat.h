#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Vehicle/ZSailing.h>
#include <Glacier/Vehicle/PathMover.h>
#include <Glacier/Vehicle/ZBoatMoveModelIF.h>
#include <Glacier/Vehicle/ZBoatMoveModel.h>


namespace Glacier
{
    class ZBoat : public ZSailing, public ZBoatMoveModelIF
    {
    public:
        // vtbl
        // data        
        ZBoatMoveModel m_BoatMoveModel;
        PathMover m_PathMover;
        ZVector3 m_vInitialPos;
        ZMat3x3 m_mInitialMat;
        RE_ADD_PADDING(8); // Idk, need fix later
    };
    RE_VERIFY_SIZE(ZBoat, 0x600);
}