#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
	struct CObjectInfo
	{
		uint32_t iID;
		uint16_t iMinX;
		uint16_t iMinY;
		uint16_t iMinZ;
		uint16_t iMaxX;
		uint16_t iMaxY;
		uint16_t iMaxZ;
	};
	RE_VERIFY_SIZE(CObjectInfo, 0x10);
}