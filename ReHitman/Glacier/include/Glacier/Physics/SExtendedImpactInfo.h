#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/STempStripsUniqueId.h>
#include <cstdint>


namespace Glacier
{
    struct SLineImpact
	{		
		float fPercent;
		ZVector3 vPosition;
		ZBaseGeom *pBaseGeom;
	};
    RE_VERIFY_SIZE(SLineImpact, 0x14);

	struct SExtendedImpactInfo : SLineImpact
	{
        ZVector3 vP1;
        ZVector3 vP2;
        ZVector3 vP3;
        int lTriangleNr;
        uint32_t m_BoneId;
        STempStripsUniqueId m_HitCache;
        uint32_t m_iColiMaterialDescId;
        ZBaseGeom *pLinkObjGround;
        uint32_t m_iColiBits;
    };
    RE_VERIFY_SIZE(SExtendedImpactInfo, 0x58);
}