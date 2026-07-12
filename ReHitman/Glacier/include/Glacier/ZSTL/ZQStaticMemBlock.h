#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <cstdint>

namespace Glacier
{
	struct ZOffsetAlloc;

	struct ZQStaticMemBlock
	{
		// vtbl
		virtual ~ZQStaticMemBlock();

		// methods
		ZQStaticMemBlock(uint32_t lSize, uint32_t* pFreeList);
		void* GetPtr(uint32_t);
		uint32_t GetSize() const;
		void* GetBufferPtr();

		// members
		ZOffsetAlloc* m_pOffsetAlloc;
		char* m_pStart;
		uint32_t m_lDynamicStart;
		uint32_t m_lBufferSize;
		REFTAB* m_pStack;
	};
	RE_VERIFY_SIZE(ZQStaticMemBlock, 0x18);
}