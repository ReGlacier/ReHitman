#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct ZEnumEntry
	{
		// members
		ZEnumEntry* m_Prev{nullptr};
		int m_Value{0};
		const char* m_Name{nullptr};
	};
	RE_VERIFY_SIZE(ZEnumEntry, 0xC);

	struct ZEnumInfo
	{
		// methods
		uint32_t GetSize() const { return m_Size; }

		// members
		ZEnumEntry* m_Last;
		const char* m_Name;
		uint32_t m_Size;
	};
	RE_VERIFY_SIZE(ZEnumInfo, 0xC);
}