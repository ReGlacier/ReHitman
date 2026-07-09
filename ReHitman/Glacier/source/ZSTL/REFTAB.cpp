#include <Glacier/ZSTL/REFTAB.h>

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
    REFTAB* REFTAB::MakeReftab(int pPoolSize, int pUserData)
    {
        void* pRawMem = nullptr;

#       ifdef USE_STL_ALLOCATOR
        pRawMem = std::malloc(sizeof(REFTAB));
#       else
        pRawMem = ZSysMem::m_pInstance->New(EAllocType::DEFAULT_MEM, sizeof(REFTAB));
#       endif

        if (!pRawMem)
            return nullptr;
        
        return ::new (pRawMem) REFTAB(pPoolSize, pUserData);
    }

    void REFTAB::DeleteReftab(REFTAB* pRefTab)
    {
        if (!pRefTab) 
            return;

        pRefTab->~REFTAB();

#       ifdef USE_STL_ALLOCATOR
        std::free(pRefTab);
#       else
        ZSysMem::m_pInstance->Delete(pRefTab);
#       endif
    }

    REFTAB::REFTAB(int pPoolSize, int pUserData)
    {
        if (pPoolSize < 1)
            pPoolSize = 1;

        m_lRefsPrBlk = pPoolSize;
        EleSize = pUserData + 1;
        EleCount = 0;
        BlkSize = (pUserData + 1) * pPoolSize;
        TabFirstPtr = nullptr;
        TabBlockPtr = nullptr;
    }

    REFTAB::~REFTAB()
    {
        auto* pCurrent = TabFirstPtr;

        while (pCurrent)
        {
            auto* pToClear = pCurrent;
            pCurrent = pCurrent->_Next;
            DeleteBlock(pToClear);
        }

        EleCount = 0;
        TabFirstPtr = nullptr;
        TabBlockPtr = nullptr;
    }

    uint32_t* REFTAB::Add(uint32_t rRef)
    {
        if (!TabFirstPtr || TabBlockPtr->_Cou == BlkSize)
        {
            TabBlk* pNewBlk = NewBlock();
            ZASSERT(pNewBlk != nullptr);

            pNewBlk->_Next = nullptr;

            if (TabFirstPtr)
            {
                TabBlockPtr->_Next = pNewBlk;
                pNewBlk->_Prev = TabBlockPtr;
            }
            else
            {
                TabFirstPtr = pNewBlk;
                pNewBlk->_Prev = nullptr;
            }

            TabBlockPtr = pNewBlk;
            TabBlockPtr->_Cou = 0;
        }

        uint32_t* pBlockDataStart = reinterpret_cast<uint32_t*>(TabBlockPtr + 1);
        uint32_t* pTargetSlot = pBlockDataStart + TabBlockPtr->_Cou;

        *pTargetSlot = rRef;

        TabBlockPtr->_Cou += EleSize;
        ++EleCount;

        return pTargetSlot + 1;
    }

    uint32_t* REFTAB::AddUnique(uint32_t rRef)
    {
        uint32_t* pFound = Find(rRef);
        if (pFound)
            return pFound + 1;
        
        return Add(rRef);
    }

    void REFTAB::Clear()
    {
        auto* pCurrent = TabFirstPtr;

        while (pCurrent)
        {
            auto* pToClear = pCurrent;
            pCurrent = pCurrent->_Next;
            DeleteBlock(pToClear);
        }

        EleCount = 0;
        TabFirstPtr = nullptr;
        TabBlockPtr = nullptr;
    }

    void REFTAB::ClearThis()
    {
        // In PC build just same func
        Clear();
    }

    int REFTAB::Count()
    {
        return EleCount;
    }

    uint32_t REFTAB::Size()
    {
        return EleSize;
    }

    uint32_t REFTAB::GetEleSize()
    {
        return EleSize;
    }

    uint32_t REFTAB::PoolSize()
    {
        return m_lRefsPrBlk;
    }

    void REFTAB::DelRefPtr(uint32_t* pRefPtr)
    {
        RefRun it;
        RunInitNxtRef(&it);
        
        uint32_t* pCurrent = reinterpret_cast<uint32_t*>(RunNxtRefPtr(&it));
        
        while (pCurrent)
        {
            if (pCurrent == pRefPtr)
            {
                RunDelRef(&it);
                return; 
            }
            
            pCurrent = reinterpret_cast<uint32_t*>(RunNxtRefPtr(&it));
        }
    }

    bool REFTAB::Exists(uint32_t* pRef)
    {
        RefRun it;
        RunInitNxtRef(&it);

        while (auto* pCurrent = reinterpret_cast<uint32_t*>(RunNxtRefPtr(&it)))
        {
            if (pCurrent == pRef)
            {
                return true;
            }
        }

        return false;
    }

    bool REFTAB::Exists(uint32_t rRef) const
    {
        RefRun it;
        RunInitNxtRef(&it);

        while (it)
        {
            uint32_t rCurrent = RunNxtRef(&it);
            
            if (rCurrent == rRef)
            {
                return true;
            }
        }

        return false;
    }

    uint32_t* REFTAB::Find(uint32_t rRef) const
    {
        RefRun it;
        RunInitNxtRef(&it);

        while (auto* pCurrent = RunNxtRefPtr(&it))
        {
            if (*pCurrent == rRef)
            {
                return const_cast<uint32_t*>(pCurrent);
            }
        }

        return nullptr;
    }

    uint32_t REFTAB::GetRefNr(int lRefNo) const
    {
        if (lRefNo >= EleCount)
        {
            return 0; 
        }

        const int refsPerBlock = m_lRefsPrBlk & 0x7FFFFFFF;
        TabBlk* pCurrentBlk = TabFirstPtr;

        while (lRefNo >= refsPerBlock)
        {
            pCurrentBlk = pCurrentBlk->_Next;
            lRefNo -= refsPerBlock;
        }

        uint32_t* pBlockDataStart = reinterpret_cast<uint32_t*>(pCurrentBlk + 1);
        uint32_t* pTargetSlot = pBlockDataStart + (lRefNo * EleSize);

        return *pTargetSlot;
    }

    uint32_t* REFTAB::GetRefPtrNr(int iIndex)
    {
        if (iIndex >= this->EleCount)
        {
            return nullptr;
        }

        const int refsPerBlock = m_lRefsPrBlk & 0x7FFFFFFF;
        TabBlk* pCurrentBlk = TabFirstPtr;

        while (iIndex >= refsPerBlock)
        {
            pCurrentBlk = pCurrentBlk->_Next;
            iIndex -= refsPerBlock;
        }

        uint8_t* pBlockDataStart = reinterpret_cast<uint8_t*>(pCurrentBlk + 1);        
        uint32_t* pTargetSlot = reinterpret_cast<uint32_t*>(pBlockDataStart + (iIndex * EleSize));

        return pTargetSlot;
    }

    uint32_t REFTAB::GetIndex(uint32_t rInputRef) const
    {
        RefRun it;
        RunInitNxtRef(&it);

        uint32_t iIndex = 0;

        while (it)
        {
            uint32_t rCurrent = RunNxtRef(&it);

            if (rCurrent == rInputRef)
            {
                return iIndex;
            }

            ++iIndex;
        }

        // not found
        return 0xFFFFFFFFu;
    }

    void REFTAB::Remove(uint32_t rRef)
    {
        RemoveIfExists(rRef);
    }

    bool REFTAB::RemoveIfExists(uint32_t rRef)
    {
        RefRun it;
        RunInitNxtRef(&it);

        while (it)
        {
            uint32_t rCurrent = RunNxtRef(&it);

            if (rCurrent == rRef)
            {
                RunDelRef(&it);
                return true;
            }
        }

        return false;
    }

    void REFTAB::RunDelRef(RefRun* pRefRun)
    {
        ZASSERT(pRefRun != nullptr);
        ZASSERT(TabBlockPtr != nullptr);

        --EleCount;
    
        TabBlockPtr->_Cou -= EleSize;

        if (pRefRun->_RunDir > 0)
        {
            pRefRun->_RunCou -= EleSize;
            if (pRefRun->_RunCou < 0)
            {
                pRefRun->_RunCou = BlkSize - EleSize;
                pRefRun->_RunPtr = pRefRun->_RunPtr->_Prev;
            }
        }

        if (EleCount > 0)
        {
            uint32_t* pLastElementData = reinterpret_cast<uint32_t*>(TabBlockPtr + 1) + TabBlockPtr->_Cou;            
            uint32_t* pTargetDeleteData = reinterpret_cast<uint32_t*>(pRefRun->_RunPtr + 1) + pRefRun->_RunCou;

            if (pLastElementData == pTargetDeleteData)
            {
                memset(pLastElementData, 0x55, EleSize);
            }
            else
            {
                memcpy(pTargetDeleteData, pLastElementData, EleSize);
            }
        }

        TabBlk* pEmptyBlk = TabBlockPtr;
        if (pEmptyBlk->_Cou == 0)
        {
            TabBlk* pPrevBlk = pEmptyBlk->_Prev;
            TabBlockPtr = pPrevBlk;

            if (pPrevBlk == nullptr)
            {
                TabFirstPtr = nullptr;
                pRefRun->_RunPtr = nullptr;
                
                ZASSERT(EleCount == 0); // Must be empty here
            }
            else
            {
                if (pEmptyBlk == pRefRun->_RunPtr)
                {
                    pRefRun->_RunPtr = pPrevBlk;
                    pRefRun->_RunCou = BlkSize;
                }
                
                TabBlockPtr->_Next = nullptr;
            }

            DeleteBlock(pEmptyBlk);
        }
    }

    void REFTAB::RunInitNxtRef(RefRun* pRefRun) const
    {
        ZASSERT(pRefRun != nullptr);

        pRefRun->_RunCou = 0;
        pRefRun->_RunDir = 1;
        pRefRun->_RunPtr = TabFirstPtr;
    }
    
    void REFTAB::RunInitNxtRef(RefRun* pRefRun)
    {
        ZASSERT(pRefRun != nullptr);

        pRefRun->_RunCou = 0;
        pRefRun->_RunDir = 1;
        pRefRun->_RunPtr = TabFirstPtr;

        m_lRefsPrBlk &= ~0x80000000;
    }

    void REFTAB::RunInitPrevRef(RefRun* pRefRun) const
    {
        ZASSERT(pRefRun != nullptr);

        pRefRun->_RunDir = -1;
        pRefRun->_RunPtr = TabBlockPtr;
        pRefRun->_RunCou = TabBlockPtr ? TabBlockPtr->_Cou : 0;
    }

    void REFTAB::RunInitPrevRef(RefRun* pRefRun)
    {
        ZASSERT(pRefRun != nullptr);

        pRefRun->_RunDir = -1;
        pRefRun->_RunPtr = TabBlockPtr;
        pRefRun->_RunCou = TabBlockPtr ? TabBlockPtr->_Cou : 0;

        m_lRefsPrBlk &= ~0x80000000;
    }

    uint32_t REFTAB::RunNxtRef(RefRun* pRefRun) const
    {
        auto pRef = RunNxtRefPtr(pRefRun);
        if (pRef)
            return *pRef;
        
        return 0u;
    }

    uint32_t REFTAB::RunNxtRef(RefRun* pRefRun)
    {
        auto pRef = RunNxtRefPtr(pRefRun);
        if (pRef)
            return *pRef;
        
        return 0u;
    }

    const uint32_t* REFTAB::RunNxtRefPtr(RefRun* pRefRun) const
    {
        ZASSERT(pRefRun != nullptr);

        if (pRefRun->_RunCou == BlkSize)
        {
            pRefRun->_RunCou = 0;
            pRefRun->_RunPtr = pRefRun->_RunPtr->_Next;
        }

        TabBlk* pCurrentBlk = pRefRun->_RunPtr;
        if (!pCurrentBlk)
        {
            return nullptr;
        }

        int currentCou = pRefRun->_RunCou;

        if (currentCou != TabBlockPtr->_Cou || pCurrentBlk->_Next != nullptr)
        {
            pRefRun->_RunCou = currentCou + EleSize;

            auto* pResult = reinterpret_cast<const uint32_t*>(pCurrentBlk);
            return pResult + currentCou + 4;
        }
        
        pRefRun->_RunPtr = nullptr;
        return nullptr;
    }

    uint32_t* REFTAB::RunNxtRefPtr(RefRun* pRefRun)
    {
        // Same to const impl
        return const_cast<uint32_t*>(const_cast<const REFTAB*>(this)->RunNxtRefPtr(pRefRun));
    }

    uint32_t REFTAB::RunPrevRef(RefRun* pRefRun) const
    {
        auto* pResult = RunPrevRefPtr(pRefRun);
        if (pResult)
            return *pResult;

        return 0;
    }

    uint32_t REFTAB::RunPrevRef(RefRun* pRefRun)
    {
        auto* pResult = RunPrevRefPtr(pRefRun);
        if (pResult)
            return *pResult;

        return 0;
    }

    const uint32_t* REFTAB::RunPrevRefPtr(RefRun* pRefRun) const
    {
        ZASSERT(pRefRun != nullptr);

        if (!pRefRun->_RunPtr)
        {
            return nullptr;
        }

        pRefRun->_RunCou -= EleSize;

        if (pRefRun->_RunCou < 0)
        {
            TabBlk* pPrevBlk = pRefRun->_RunPtr->_Prev;
            pRefRun->_RunPtr = pPrevBlk;

            if (!pPrevBlk)
            {
                return nullptr;
            }

            pRefRun->_RunCou = BlkSize - EleSize;
        }

        return RunToRefPtr(pRefRun);
    }

    uint32_t* REFTAB::RunPrevRefPtr(RefRun* pRefRun)
    {
        return const_cast<uint32_t*>(const_cast<const REFTAB*>(this)->RunPrevRefPtr(pRefRun));
    }
    
    uint32_t REFTAB::operator[](int lIndex) const
    {
        ZASSERT(lIndex >= 0);

        return GetRefNr(lIndex);
    }

    const uint32_t* REFTAB::RunToRefPtr(RefRun* pRefRun) const
    {
        ZASSERT(pRefRun != nullptr);

        if (!pRefRun->_RunPtr)
        {
            return nullptr;
        }
        
        const uint32_t* pBlockDataStart = reinterpret_cast<const uint32_t*>(pRefRun->_RunPtr + 1);
        return pBlockDataStart + pRefRun->_RunCou;
    }

    void REFTAB::DeleteBlock(TabBlk* pBlk)
    {
        ZASSERT(pBlk != nullptr);
        
#       ifdef USE_STL_ALLOCATOR
        std::free((void*)pBlk);
#       else
        ZSysMem::m_pInstance->Delete((void*)pBlk);
#       endif
    }

    TabBlk* REFTAB::NewBlock(void)
    {
        const size_t iTotalSize = (4 * BlkSize) + sizeof(TabBlk);

#       ifdef USE_STL_ALLOCATOR
        return reinterpret_cast<TabBlk*>(std::malloc(iTotalSize));
#       else
        return reinterpret_cast<TabBlk*>(ZSysMem::m_pInstance->New(EAllocType::DEFAULT_MEM, iTotalSize));
#       endif
    }
}