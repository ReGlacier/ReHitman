#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
	class ZROOM;

	struct ZBaseGeomRoomList
	{
		// methods
		void Remove(ZROOM* pRoom);
		void Init();
		ZROOM* GetRoomNr(uint32_t lRoomNr) const;
		ZROOM** GetRoomList();
		bool Exists(ZROOM* pRoom) const;
		uint8_t Count() const;
		void Clear();
		bool Add(ZROOM* pRoom);

		// members
		ZROOM* m_pRooms[6];
		uint8_t m_cNrRooms;
	};
	RE_VERIFY_SIZE(ZBaseGeomRoomList, 0x1C);
}
