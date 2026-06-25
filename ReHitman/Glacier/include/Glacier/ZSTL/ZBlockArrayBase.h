#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
	struct ZBlockArrayBase
	{
		// vtbl
		virtual ~ZBlockArrayBase();

		virtual void Construct(void*);
		virtual void Destruct(void*);
		virtual void Copy(void*, const void*);

		// members
		const unsigned int m_ItemSize;
		const unsigned int m_BlockSizeExhibitor;
		const unsigned int m_SubIndexMask;
		unsigned int m_PrimaryArrayCapacity;
		char** m_PrimaryArray;
		unsigned int m_Size;
	};
	RE_VERIFY_SIZE(ZBlockArrayBase, 0x1C);
}