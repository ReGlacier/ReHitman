#include <Glacier/ZSTL/REFTAB32.h>

#if defined(REHITMAN_STANDALONE) || defined(REHITMAN_TESTS) // Only for separated testing without game instance
#   define USE_STL_ALLOCATOR
#else
#   include <Glacier/ZSysMem.h>
#   define USE_GLACIER_ALLOCATOR
#endif

// Override for testing env
#ifdef REHITMAN_TESTS
#   include <stdexcept>
#   include <string>
#   undef ZASSERT
#   define ZASSERT(expr) \
        if (!(expr)) { \
            throw std::runtime_error("ZASSERT failed: " #expr " at " + std::to_string(__LINE__)); \
        }
#endif


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

#       ifdef USE_STL_ALLOCATOR
        std::free((void*)pBlk);
#       else
        // В оригинале используется оператор delete для блоков
        ZSysMem::m_pInstance->Delete((void*)pBlk);
#       endif
    }

    TabBlk* REFTAB32::NewBlock(void)
    {
        if (!TabFirstPtr)
        {
            return reinterpret_cast<TabBlk*>(m_FirstTab);
        }

        const size_t iRequiredSize = sizeof(TabBlk) + (BlkSize * sizeof(uint32_t));
        TabBlk* pResult = nullptr;

#       ifdef USE_STL_ALLOCATOR
        pResult = reinterpret_cast<TabBlk*>(std::malloc(iRequiredSize));
#       else
        pResult = reinterpret_cast<TabBlk*>(ZSysMem::m_pInstance->New(EAllocType::DEFAULT_MEM, iRequiredSize));
#       endif
        if (pResult)
        {
            // From Glacier code
            std::memset(pResult, 0x22, iRequiredSize);
        }

        return pResult;
    }
        
}