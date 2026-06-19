#pragma once

#include <cstdint>


namespace Glacier
{
	template <size_t N, typename T>
	struct ZStackArray
	{
		uint32_t m_lNrEntries;
		T m_Array[N];
	};
}