#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
	class ZROOM;

	struct ZBaseGeomRoomList
	{
		ZROOM* m_pRooms[6];
		uint8_t m_cNrRooms;
	};
	RE_VERIFY_SIZE(ZBaseGeomRoomList, 0x1C);
}