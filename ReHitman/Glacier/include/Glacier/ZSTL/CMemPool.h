#pragma once

#include <cstdint>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
	struct CMemPool
	{
		void* m_pxPool;
		uint16_t* m_pAllocTable;
		int m_iEntries;
		int m_iBlockSize;
		int m_iNumAlloc;
		bool m_bOwnPool;
		bool m_padding[3];
	};
	RE_VERIFY_SIZE(CMemPool, 0x18);
}