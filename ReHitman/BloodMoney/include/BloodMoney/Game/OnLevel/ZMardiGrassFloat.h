#pragma once

#include <Glacier/PF4/ZDynamicObstacle.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>

namespace Hitman::BloodMoney
{
	struct ZMardiGrassFloat : public Glacier::ZGROUP
	{
        float m_fWidth;
        float m_fLength;
        uint32_t m_lNumCorners;
        Glacier::ZMatrix m_MovementCorners[9];
        Glacier::ZStackArray<16, ZGEOM*> m_Corners;
        Glacier::ZREF m_rObstacleBase;
        ZMardiGrassFloat* m_pFloatGroup;
        Glacier::ZMat3x3 m_mTrackerPosition;
        Glacier::ZVector3 m_vTrackerPosition;
        uint32_t m_lNextCorner;
        Glacier::ZMat3x3 m_mPosition;
        Glacier::ZVector3 m_vPosition;
        Glacier::ZMatrix m_PrevPosition;
        Glacier::ZGEOM* m_pCenterGeom;
        Glacier::PF4::ZDynamicObstacle* m_pObstacle;
	};
	RE_VERIFY_SIZE(ZMardiGrassFloat, 0x2F0); // Not finished yet
}