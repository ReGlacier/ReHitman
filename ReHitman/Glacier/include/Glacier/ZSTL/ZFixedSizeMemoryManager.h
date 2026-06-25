#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
	struct ZFixedSizeMemoryManagerBase
	{
		struct ZFreeBlock
		{
			struct ZFreeBlock* m_Next;
		};

		struct Iterator
		{
			const ZFixedSizeMemoryManagerBase* m_MemoryManager;
			uint32_t m_Index;
		};

		bool m_FreeListValid;
		RE_ADD_PADDING(3);
		const uint32_t m_BlockSize;
		const uint32_t m_NumberOfBlocks;
		char* m_Buffer;
		int16_t *m_ID;
		ZFreeBlock* m_FirstFreeBlock;
		ZFreeBlock* m_LastFreeBlock;
	};
	RE_VERIFY_SIZE(ZFixedSizeMemoryManagerBase, 0x1C);

	template <typename T>
	struct ZFixedSizeMemoryManager : public ZFixedSizeMemoryManagerBase {};
}