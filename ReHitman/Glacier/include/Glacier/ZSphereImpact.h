#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/STempStripsUniqueId.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    struct ZActiveImpactPrim
    {
        uint32_t m_lType;
        uint32_t m_lImpactTriangleNr;
        ZVector3 m_vCorner1;
        ZVector3 m_vCorner2;
        ZVector3 m_vCorner3;
        ZVector3 m_vNormal;
        ZBaseGeom* m_pBaseGeom;
        RE_ADD_PADDING(4);
        STempStripsUniqueId m_StripId;
        bool m_bInvalid;
        bool m_bNoEnter;
        RE_ADD_PADDING(6);
    };
    RE_VERIFY_SIZE(ZActiveImpactPrim, 0x50);

	struct ZSphereImpact
	{
		unsigned int m_lNrActiveImpacts;
		unsigned int m_lNrFramesOffGround;
		STempStripsUniqueId m_StripId;
		ZActiveImpactPrim m_ActiveImpacts[4];
		int m_iGroundMaterial;
		bool m_bLocal;
		bool m_bGroundContact;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZSphereImpact, 0x158);
}