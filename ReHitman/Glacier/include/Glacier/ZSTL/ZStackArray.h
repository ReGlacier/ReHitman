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

	template <int N, typename T, typename TKey>
	struct ZStackArrayInsert : public Glacier::ZStackArray<N, T>
	{
		TKey m_SortValues[N];
	};
}