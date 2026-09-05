#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
	struct ZBlockArrayBase
	{
		// vtbl
		virtual ~ZBlockArrayBase();

		virtual void Construct(void*) = 0;
		virtual void Destruct(void*) = 0;
		virtual void Copy(void*, const void*) = 0;

		// methods
		ZBlockArrayBase(uint32_t item_size, uint32_t block_size_exhibitor);
		void SetupWithoutInit(uint32_t );
		void Setup(uint32_t);
		void Resize(uint32_t size);
		void Reset(uint32_t);
		uint32_t GetSize() const;
		void* GetItem(uint32_t item_idx);
		void Cleanup();
		void ConstructItems(const uint32_t from, const uint32_t to);
		void DestructItems(const uint32_t from, const uint32_t to);
		uint32_t CalculatePrimaryArraySize(uint32_t new_size) const;

		// members
		const uint32_t m_ItemSize;
		const uint32_t m_BlockSizeExhibitor;
		const uint32_t m_SubIndexMask;
		uint32_t m_PrimaryArrayCapacity;
		char** m_PrimaryArray;
		uint32_t m_Size;
	};
	RE_VERIFY_SIZE(ZBlockArrayBase, 0x1C);
}