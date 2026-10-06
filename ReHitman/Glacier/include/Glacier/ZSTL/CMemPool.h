#pragma once

#include <cstdint>
#include <Glacier/ReGlacier.h>

namespace Glacier
{
	struct CMemPool
	{
		// methods
		CMemPool() = default;
		~CMemPool();

		void Init(int iBlockSize, int iEntries, int iAlign);
		void Init(int iBlockSize, int iEntries, void *pPool, int iAlign);
		void* Alloc();
		void* Alloc(int* iBlock);
		void DeAlloc(void* pxBlock);
		void DeInit();
		void* GetPoolPtr() { return m_pxPool; }
		int GetNumEntries() const { return m_iEntries; }
		void* GetPtrFromBlockNum(int iBlock) const;
		bool IsFull() const;

		// members
		void* m_pxPool{nullptr};
		uint16_t* m_pAllocTable{nullptr};
		int m_iEntries{0};
		int m_iBlockSize{0};
		int m_iNumAlloc{0};
		bool m_bOwnPool{false};
		bool m_padding[3]{false,false,false};
	};
	RE_VERIFY_SIZE(CMemPool, 0x18);
}