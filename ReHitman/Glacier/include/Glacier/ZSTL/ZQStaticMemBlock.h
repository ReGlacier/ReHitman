#pragma once

#include <Glacier/ZSTL/ZQStaticMemBlock.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <cstdint>

namespace Glacier
{
	struct ZOffsetAlloc;
	struct ZQStaticMemBlock
	{
		virtual ~ZQStaticMemBlock();

		ZOffsetAlloc* m_pOffsetAlloc;
		char* m_pStart;
		uint32_t m_lDynamicStart;
		uint32_t m_lBufferSize;
		REFTAB* m_pStack;
	};
	RE_VERIFY_SIZE(ZQStaticMemBlock, 0x18);
}