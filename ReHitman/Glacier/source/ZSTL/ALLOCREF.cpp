#include <Glacier/ZSTL/ALLOCREF.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/System/ZSysInterfaceWintel.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <vector>

namespace Glacier
{
namespace 
{
    inline bool IsRefsLocked()
    {
        return g_pSysInterface && g_pSysInterface->m_bRefsLocked;
    }
}

    ALLOCREF::ALLOCREF(uint32_t lRefShift, uint32_t lNrActiveRefs)
    {
        m_lRefShift = lRefShift;
        m_lRefAnd = lNrActiveRefs - 1;
        m_pRefStack = REFTAB::MakeReftab(128, 0);
        m_pUsedRefs = (ZREF*)ZUniMemory::Allocate(sizeof(ZREF) * lNrActiveRefs);

        if (lNrActiveRefs)
        {
            for (int i = 0; i < lNrActiveRefs; i++)
            {
                m_pUsedRefs[i] = 1 << m_lRefShift;
            }
        }

        m_lActRefNr = 0;
        m_bCheckLock = true;
    }

    ALLOCREF::~ALLOCREF()
    {
        GetActiveRefs(nullptr);
        ZUniMemory::Free((void*)m_pUsedRefs);
        REFTAB::DeleteReftab(m_pRefStack);
        m_pRefStack = nullptr;
    }
    
    Glacier::ZREF ALLOCREF::NewRef()
    {
        if (m_bCheckLock && IsRefsLocked())
        {
            // Unexpected
            ZASSERT(false);
        }

        if (m_pRefStack && m_pRefStack->Count() > 0)
        {
            RefRun it;
            m_pRefStack->RunInitNxtRef(&it);

            uint32_t freeSlotSignaled = m_pRefStack->RunNxtRef(&it);

            if (it && freeSlotSignaled != 0)
            {
                while (true)
                {
                    m_pRefStack->RunDelRef(&it);

                    uint32_t slotIndex = freeSlotSignaled - 1;
                    uint32_t generationMask = ~m_lRefAnd;

                    uint32_t recycledRef = (generationMask & m_pUsedRefs[slotIndex]) | slotIndex;

                    if ((generationMask & recycledRef) != 0)
                    {
                        return recycledRef;
                    }

                    freeSlotSignaled = m_pRefStack->RunNxtRef(&it);
                    
                    if (!it || freeSlotSignaled == 0)
                    {
                        break;
                    }
                }
            }
        }

        if (m_lActRefNr > m_lRefAnd)
        {
            // Out of pool
            ZASSERT(false);
        }

        uint32_t currentActiveIndex = m_lActRefNr;
        
        Glacier::ZREF newRef = currentActiveIndex | (1 << m_lRefShift);        
        m_lActRefNr = currentActiveIndex + 1;

        return newRef;
    }
    
    uint32_t ALLOCREF::GetActiveRefs(REFTAB* out)
    {
        if (!m_lActRefNr)
            return 0;

        // TODO: Need replace this code to actual glacier... idk
        std::vector<uint8_t> freeSlotsMap(m_lActRefNr, 0);

        if (m_pRefStack)
        {
            RefRun it;
            m_pRefStack->RunInitNxtRef(&it);

            while (true)
            {
                uint32_t refValue = m_pRefStack->RunNxtRef(&it);
                if (!it) 
                    break;

                uint32_t slotIndex = refValue; 

                if (slotIndex == 0 || slotIndex > m_lActRefNr)
                {
                    ZASSERT(false); 
                }

                freeSlotsMap[slotIndex - 1] = 1;
            }
        }

        uint32_t activeRefsCount = 0;

        for (uint32_t i = 0; i < m_lActRefNr; ++i)
        {
            if (freeSlotsMap[i] == 0)
            {
                if (out)
                {
                    uint32_t generationBits = ~m_lRefAnd & m_pUsedRefs[i];
                    uint32_t reconstructedRef = i + generationBits;

                    out->Add(reconstructedRef);
                }
                
                activeRefsCount++;
            }
        }

        return activeRefsCount;
    }

    uint32_t ALLOCREF::GetNrActiveRefs()
    {
        return GetActiveRefs(nullptr);
    }

    bool ALLOCREF::FreeRef(Glacier::ZREF rRef)
    {
        if (m_bCheckLock && IsRefsLocked())
        {
            // Unexpected...
            ZASSERT(false);
        }

        if ((~m_lRefAnd & rRef) == 0)
        {
            return false;
        }

        if (!CheckRefActive(rRef))
        {
            PrintRef(rRef);
            return false;
        }

        auto* pRefSlot = &m_pUsedRefs[(m_lRefAnd & rRef)];
        *pRefSlot = ((*pRefSlot >> m_lRefShift) + 1) << m_lRefShift;
        m_pRefStack->Add((m_lRefAnd & rRef) + 1);

        return true;
    }

    bool ALLOCREF::CheckRefActive(Glacier::ZREF rRef)
    {
        // WHAT THE FUCK IS GOING ON HERE???
        return (rRef & ~m_lRefAnd) == (~m_lRefAnd & m_pUsedRefs[rRef & m_lRefAnd]);
    }

    void ALLOCREF::PrintRef(Glacier::ZREF rRef)
    {
        // From PS2 Debug build
        unsigned int lo = rRef & this->m_lRefAnd;
        unsigned int hi = rRef >> this->m_lRefShift;

        std::printf("REF %x lo=%d hi=%d\n", rRef, lo, hi);
    }
    
    REFTAB* ALLOCREF::GetRefStack()
    {
        return m_pRefStack;
    }

    Glacier::ZREF* ALLOCREF::GetUsedRefs()
    {
        return m_pUsedRefs;
    }

    void ALLOCREF::SetRefStack(REFTAB* pRefTab)
    {
        m_pRefStack->Clear();
        if (!pRefTab) 
            return;
        
        RefRun it;
        pRefTab->RunInitNxtRef(&it);

        for (uint32_t i = pRefTab->RunNxtRef(&it); it; i = pRefTab->RunNxtRef(&it))
        {
            m_pRefStack->Add(i);
        }
    }

    void ALLOCREF::SetUsedRefs(Glacier::ZREF* pUsedRefs, unsigned int lNrUsedRefs)
    {
        ZASSERT(lNrUsedRefs == m_lRefAnd + 1);
        std::memcpy(m_pUsedRefs, pUsedRefs, sizeof(ZREF) * lNrUsedRefs);
    }
}