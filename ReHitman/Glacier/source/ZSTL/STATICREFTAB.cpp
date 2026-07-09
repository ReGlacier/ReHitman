#include <Glacier/ZSTL/STATICREFTAB.h>

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
    STATICREFTAB::STATICREFTAB(int pPoolSize, int pUserData) 
        : REFTAB(pPoolSize, pUserData)
    {
        if (pPoolSize < 1)
            pPoolSize = 1;

        BlkSize = pPoolSize * (pUserData + 2);
        EleCount = 0;
        EleSize = pUserData + 2;
        m_lRefsPrBlk = pPoolSize;
        TabFirstPtr = 0;
        TabBlockPtr = 0;
        
        m_pFreeStack = REFTAB::MakeReftab(m_lRefsPrBlk & 0x7FFFFFFF, 0);
    }

    STATICREFTAB::~STATICREFTAB()
    {
        if (m_pFreeStack)
        {
            REFTAB::DeleteReftab(m_pFreeStack);
            m_pFreeStack = nullptr;
        }
    }

    uint32_t* STATICREFTAB::Add(uint32_t rRef)
    {
        this->EleCount++;

        if (m_pFreeStack->Count() == 0)
        {
            TabBlk* pNewBlk = this->NewBlock();
            ZASSERT(pNewBlk != nullptr);

            pNewBlk->_Prev = nullptr;
            pNewBlk->_Next = this->TabFirstPtr;
            pNewBlk->_Cou = 0;

            if (TabFirstPtr)
            {
                TabFirstPtr->_Prev = pNewBlk;
            }

            TabFirstPtr = pNewBlk;

            int refsInBlock = this->m_lRefsPrBlk & 0x7FFFFFFF;
            
            uint32_t* pSlotIterator = reinterpret_cast<uint32_t*>(pNewBlk + 1);
            for (int i = 0; i < refsInBlock; ++i)
            {
                m_pFreeStack->Add(reinterpret_cast<uint32_t>(pSlotIterator));

                TabBlk** pParentBlkSlot = reinterpret_cast<TabBlk**>(&pSlotIterator[EleSize - 1]);
                *pParentBlkSlot = pNewBlk;

                pSlotIterator += EleSize;
            }
        }

        uint32_t* pTargetSlot = nullptr;

        if (m_pFreeStack)
        {
            RefRun it;
            m_pFreeStack->RunInitNxtRef(&it);
            uint32_t* pFreeAddr = m_pFreeStack->RunNxtRefPtr(&it);
            
            if (pFreeAddr)
            {
                pTargetSlot = reinterpret_cast<uint32_t*>(*pFreeAddr);
                m_pFreeStack->RunDelRef(&it);
            }
        }

        if (!pTargetSlot)
        {
            pTargetSlot = reinterpret_cast<uint32_t*>(rRef);
        }

        TabBlk* pParentBlk = *reinterpret_cast<TabBlk**>(&pTargetSlot[EleSize - 1]);
        pParentBlk->_Cou++;

        pTargetSlot[0] = rRef;

        return pTargetSlot + 1;
    }

    uint32_t* STATICREFTAB::AddUnique(uint32_t)
    {
        ZASSERT(false); // __debugtrap()
        return nullptr;
    }

    void STATICREFTAB::Clear()
    {
        if (m_pFreeStack)
        {
            m_pFreeStack->Clear();
            REFTAB::DeleteReftab(m_pFreeStack);
            m_pFreeStack = nullptr;
        }

        TabBlk* pCurrentBlk = this->TabFirstPtr;
        while (pCurrentBlk)
        {
            TabBlk* pToKill = pCurrentBlk;
            pCurrentBlk = pCurrentBlk->_Next;
            DeleteBlock(pToKill);
        }

        EleCount = 0;
        TabFirstPtr = nullptr;
        TabBlockPtr = nullptr;
    }

    void STATICREFTAB::ClearThis()
    {
        if (m_pFreeStack)
        {
            m_pFreeStack->Clear();
            REFTAB::DeleteReftab(m_pFreeStack);
            m_pFreeStack = nullptr;
        }
    }
    
    void STATICREFTAB::DelRefPtr(uint32_t* pRef)
    {
        EleCount--;

        uint32_t* pSlotStart = pRef - 1;
        TabBlk* pParentBlk = *reinterpret_cast<TabBlk**>(&pSlotStart[EleSize - 1]);

        pParentBlk->_Cou--;
        if (pParentBlk->_Cou == 0)
        {
            if (pParentBlk->_Prev)
                pParentBlk->_Prev->_Next = pParentBlk->_Next;
            else
                TabFirstPtr = pParentBlk->_Next;

            if (pParentBlk->_Next)
                pParentBlk->_Next->_Prev = pParentBlk->_Prev;

            if (m_pFreeStack)
            {
                RefRun it;
                m_pFreeStack->RunInitNxtRef(&it);
                
                while (uint32_t* pFreeSlotValueRef = m_pFreeStack->RunNxtRefPtr(&it))
                {
                    uint32_t* pCheckedSlot = reinterpret_cast<uint32_t*>(*pFreeSlotValueRef);                    
                    TabBlk* pCheckedParent = *reinterpret_cast<TabBlk**>(&pCheckedSlot[this->EleSize - 1]);
                    
                    if (pCheckedParent == pParentBlk)
                    {
                        m_pFreeStack->RunDelRef(&it);
                    }
                }
            }

            DeleteBlock(pParentBlk);
        }
        else
        {
            m_pFreeStack->Add(reinterpret_cast<uint32_t>(pSlotStart));
        }
    }

    // All of that methods been declared as 'not implemented' by IOI

    uint32_t* STATICREFTAB::Find(uint32_t) const { ZASSERT(false); return nullptr; }
    void STATICREFTAB::Remove(uint32_t) { ZASSERT(false); }
    bool STATICREFTAB::RemoveIfExists(uint32_t) { ZASSERT(false); return false; }
    void STATICREFTAB::RunDelRef(RefRun *) { ZASSERT(false); }
    void STATICREFTAB::RunInitNxtRef(RefRun *) const { ZASSERT(false); }
    void STATICREFTAB::RunInitNxtRef(RefRun *) { ZASSERT(false); }
    void STATICREFTAB::RunInitPrevRef(RefRun *) const { ZASSERT(false); }
    void STATICREFTAB::RunInitPrevRef(RefRun *) { ZASSERT(false); }
    uint32_t STATICREFTAB::RunNxtRef(RefRun *) const { ZASSERT(false); return 0; }
    uint32_t STATICREFTAB::RunNxtRef(RefRun *) { ZASSERT(false); return 0; }
    const uint32_t* STATICREFTAB::RunNxtRefPtr(RefRun *) const { ZASSERT(false); return nullptr; }
    uint32_t* STATICREFTAB::RunNxtRefPtr(RefRun *) { ZASSERT(false); return nullptr; }
    uint32_t STATICREFTAB::RunPrevRef(RefRun *) const { ZASSERT(false); return 0; }
    uint32_t STATICREFTAB::RunPrevRef(RefRun *) { ZASSERT(false); return 0; }
    const uint32_t* STATICREFTAB::RunPrevRefPtr(RefRun *) const { ZASSERT(false); return nullptr; }
    uint32_t* STATICREFTAB::RunPrevRefPtr(RefRun *)  { ZASSERT(false); return nullptr; }
}