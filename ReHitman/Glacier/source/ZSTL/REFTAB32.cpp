#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    REFTAB32::REFTAB32()
        : REFTAB(32, 0)
    {
        BlkSize = 32;
        EleCount = 0;
        EleSize = 1;
        m_lRefsPrBlk = 32;
        TabFirstPtr = nullptr;
        TabBlockPtr = nullptr;
        std::memset(m_FirstTab, 0, sizeof(m_FirstTab));
    }

    REFTAB32::~REFTAB32()
    {
        TabBlk* pCurrent = TabFirstPtr;

        while (pCurrent)
        {
            TabBlk* pToClear = pCurrent;
            pCurrent = pCurrent->_Next;
            
            DeleteBlock(pToClear);
        }
        
        EleCount = 0;
        TabFirstPtr = nullptr;
        TabBlockPtr = nullptr;
    }

    void REFTAB32::DeleteBlock(TabBlk* pBlk)
    {
        if (pBlk == reinterpret_cast<TabBlk*>(m_FirstTab))
        {
            return;
        }

        ZUniMemory::Free((void*)pBlk);
    }

    TabBlk* REFTAB32::NewBlock(void)
    {
        if (!TabFirstPtr)
        {
            return reinterpret_cast<TabBlk*>(m_FirstTab);
        }

        const size_t iRequiredSize = sizeof(TabBlk) + (BlkSize * sizeof(uint32_t));
        TabBlk* pResult = (TabBlk*)ZUniMemory::Allocate(iRequiredSize);
        if (pResult)
        {
            // From Glacier code
            std::memset(pResult, 0x22, iRequiredSize);
        }

        return pResult;
    }
        
}