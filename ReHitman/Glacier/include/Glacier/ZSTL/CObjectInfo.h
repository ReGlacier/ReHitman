#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
	struct CObjectInfo
	{
		// members
		uint32_t iID;
		uint16_t iMinX;
		uint16_t iMinY;
		uint16_t iMinZ;
		uint16_t iMaxX;
		uint16_t iMaxY;
		uint16_t iMaxZ;

		// methods
		inline bool IsOverlapAABB(int iChkMinX, int iChkMinY, int iChkMinZ, int iChkMaxX, int iChkMaxY, int iChkMaxZ) const
		{
			return  iChkMinX < iMaxX && iMinX < iChkMaxX && 
					iChkMinY < iMaxY && iMinY < iChkMaxY && 
					iChkMinZ < iMaxZ && iMinZ < iChkMaxZ;
		}
	};
	RE_VERIFY_SIZE(CObjectInfo, 0x10);
}