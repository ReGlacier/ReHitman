#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct ZEnumEntry
	{
		ZEnumEntry* m_Prev;
		int m_Value;
		const char* m_Name;
	};
	RE_VERIFY_SIZE(ZEnumEntry, 0xC);

	struct ZEnumInfo
	{
		ZEnumEntry* m_Last;
		const char* m_Name;
		uint32_t m_Size;
	};
	RE_VERIFY_SIZE(ZEnumInfo, 0xC);
}