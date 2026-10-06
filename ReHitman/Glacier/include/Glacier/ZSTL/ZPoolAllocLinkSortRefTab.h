#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/LINKSORTREFTAB.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h> // ZPoolAllocator


namespace Glacier
{
    class ZPoolAllocLinkSortRefTab : public LINKSORTREFTAB
    {
    public:
        // vtbl
        ~ZPoolAllocLinkSortRefTab() override;

        void DeleteBlock(TabBlk* pBlk) override;
        TabBlk* NewBlock(void) override;
        void RemoveFreeStack() override;
        void CreateFreeStack() override;
        
        // methods
        ZPoolAllocLinkSortRefTab(ZPoolAllocator* pAllocator, int RefsPrBlk, int Exsize);

        // members
        ZPoolAllocator* m_pAllocator { nullptr };
    };
}
