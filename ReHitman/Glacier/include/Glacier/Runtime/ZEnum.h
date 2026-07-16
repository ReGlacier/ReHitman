#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct ZEnumEntry
	{
		// methods

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
		
		const char* FindItem(uint32_t iValue) const
		{
			if (!m_Last) return nullptr;

			auto* pCurrent = m_Last;

			while (pCurrent->m_Value != iValue)
			{
				pCurrent = pCurrent->m_Prev;
				if (!pCurrent)
					return nullptr;
			}

			return pCurrent->m_Name;
		}

		const char* FindItem(void** pData) const
		{
			switch (m_Size)
			{
				case 1:
				{
					return FindItem(*reinterpret_cast<uint8_t*>(*pData));
				}
				case 2:
				{
					return FindItem(*reinterpret_cast<uint16_t*>(*pData));
				}
				case 4:
				{
					return FindItem(*reinterpret_cast<uint32_t*>(*pData));
				}
				default:
				{
					ZASSERT(false && "Illegal enum size");
					return nullptr;
				}
			}
		}

		// members
		ZEnumEntry* m_Last;
		const char* m_Name;
		uint32_t m_Size;
	};
	RE_VERIFY_SIZE(ZEnumInfo, 0xC);
}