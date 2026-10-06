#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZPoolAllocRefTab::ZPoolAllocRefTab(ZPoolAllocator* pAllocator, int pPoolSize, int pUserData)
        : REFTAB(pPoolSize, pUserData)
        , m_pAllocator(pAllocator)
    {
    }

    ZPoolAllocRefTab::ZPoolAllocRefTab(ZPoolAllocator* pAllocator)
        : REFTAB(32, 0)
        , m_pAllocator(pAllocator)
    {
    }

    ZPoolAllocRefTab::~ZPoolAllocRefTab()
    {
    }

    void ZPoolAllocRefTab::DeleteBlock(TabBlk* pBlk)
    {
        ZASSERT(m_pAllocator != nullptr);

        m_pAllocator->Free((void*)pBlk);
    }

    TabBlk* ZPoolAllocRefTab::NewBlock(void)
    {
        ZASSERT(m_pAllocator != nullptr);

        const auto iAllocSize = 4 * (BlkSize + 4);
        void* pBlk = m_pAllocator->Alloc(iAllocSize);
        ZASSERT(pBlk != nullptr);

        std::memset(pBlk, 0x22, iAllocSize);
        return (TabBlk*)pBlk;
    }
}