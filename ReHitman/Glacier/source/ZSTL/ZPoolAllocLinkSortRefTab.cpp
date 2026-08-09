#include <Glacier/ZSTL/ZPoolAllocLinkSortRefTab.h>

#include <cstring>
#include <new>

namespace Glacier
{
    ZPoolAllocLinkSortRefTab::ZPoolAllocLinkSortRefTab(
        ZPoolAllocator* pAllocator,
        int RefsPrBlk,
        int Exsize)
        : LINKSORTREFTAB(RefsPrBlk, Exsize)
        , m_pAllocator(pAllocator)
    {
    }

    ZPoolAllocLinkSortRefTab::~ZPoolAllocLinkSortRefTab()
    {
        Clear();
    }

    void ZPoolAllocLinkSortRefTab::DeleteBlock(TabBlk* pBlk)
    {
        ZASSERT(m_pAllocator != nullptr);
        m_pAllocator->Free(pBlk);
    }

    TabBlk* ZPoolAllocLinkSortRefTab::NewBlock()
    {
        ZASSERT(m_pAllocator != nullptr);

        const unsigned int allocSize = 4u * static_cast<unsigned int>(BlkSize) + 16u;
        void* pBlock = m_pAllocator->Alloc(allocSize);
        ZASSERT(pBlock != nullptr);

        std::memset(pBlock, 0x22, allocSize);
        return static_cast<TabBlk*>(pBlock);
    }

    void ZPoolAllocLinkSortRefTab::RemoveFreeStack()
    {
        if (!FreeStack)
            return;

        FreeStack->Clear();
        ZASSERT(m_pAllocator != nullptr);
        static_cast<ZPoolAllocRefTab*>(FreeStack)->~ZPoolAllocRefTab();
        m_pAllocator->Free(FreeStack);
        FreeStack = nullptr;
    }

    void ZPoolAllocLinkSortRefTab::CreateFreeStack()
    {
        ZASSERT(FreeStack == nullptr);
        ZASSERT(m_pAllocator != nullptr);

        void* pMemory = m_pAllocator->Alloc(sizeof(ZPoolAllocRefTab));
        ZASSERT(pMemory != nullptr);

        FreeStack = ::new (pMemory) ZPoolAllocRefTab(m_pAllocator, 8, 0);
    }
}
