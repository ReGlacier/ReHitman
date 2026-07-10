#include <Glacier/ZSTL/LINKREFTAB.h>

namespace Glacier
{
    LINKREFTAB::LINKREFTAB(int RefsPrBlk, int Exsize) : REFTAB(RefsPrBlk, Exsize)
    {
        if ( RefsPrBlk < 1 )
            RefsPrBlk = 1;

        m_lRefsPrBlk = RefsPrBlk;
        EleSize = Exsize + 3;
        EleCount = 0;
        BlkSize = (Exsize + 3) * RefsPrBlk;
        TabFirstPtr = 0;
        TabBlockPtr = 0;
        
        FreeStack = 0;
        First = 0;
        Last = 0;
    }

    LINKREFTAB::~LINKREFTAB()
    {
        if (FreeStack)
        {
            REFTAB::DeleteReftab(FreeStack);
            FreeStack = nullptr;
        }

        REFTAB::Clear();
    }

    uint32_t* LINKREFTAB::Add(uint32_t rRef)
    {
        uint32_t* pRecord = AllocateRecord(rRef);

        PrevNext* pNode = GetPrevNext(pRecord);

        pNode->Prev = Last;

        if (Last)
            Last->Next = pNode;
        else
            First = pNode;

        Last = pNode;

        return pRecord + 1;
    }

    void LINKREFTAB::Clear()
    {
        ClearThis();
        REFTAB::Clear();
    }

    void LINKREFTAB::ClearThis() 
    {
        if (FreeStack)
        {
            RemoveFreeStack();
        }

        FreeStack = nullptr;
        First = nullptr;
        Last = nullptr;
    }

    int LINKREFTAB::Count() 
    {
        if (FreeStack)
        {
            return EleCount - FreeStack->Count();
        }

        return EleCount;
    }

    void LINKREFTAB::DelRefPtr(uint32_t* RefPtr)
    {
        PrevNext* pLinks = GetPrevNext(RefPtr);

        if (!FreeStack)
        {
            CreateFreeStack();
        }

        FreeStack->Add(reinterpret_cast<uint32_t>(pLinks));

        if (pLinks->Prev)
        {
            pLinks->Prev->Next = pLinks->Next;
        }
        else
        {
            First = pLinks->Next;
        }

        if (pLinks->Next)
        {
            pLinks->Next->Prev = pLinks->Prev;
        }
        else
        {
            Last = pLinks->Prev;
        }

        pLinks->Prev = nullptr;
        pLinks->Next = nullptr;
    }

    void LINKREFTAB::RunDelRef(RefRun* pRefRun)
    {
        TabBlk* RunPtr = pRefRun->_RunPtr;
        ZASSERT(RunPtr != nullptr);

        if (!FreeStack)
        {
            CreateFreeStack();
        }

        FreeStack->Add(reinterpret_cast<uint32_t>(pRefRun->_RunPtr));

        if (pRefRun->_RunCou <= 0)
        {
            pRefRun->_RunPtr = pRefRun->_RunPtr->_Next;
        }
        else
        {
            pRefRun->_RunPtr = pRefRun->_RunPtr->_Prev;
        }

        if ( RunPtr->_Prev )
        {
            RunPtr->_Prev->_Next = RunPtr->_Next;
        }
        else
        {
            First = (PrevNext*)RunPtr->_Next;
        }

        TabBlk* Next = RunPtr->_Next;
        if ( Next )
            Next->_Prev = RunPtr->_Prev;
        else
            Last = (PrevNext*)RunPtr->_Prev;
        
        RunPtr->_Next = 0;
        RunPtr->_Prev = 0;
    }

    void LINKREFTAB::RunInitNxtRef(RefRun* pRefRun) const
    {
        pRefRun->_RunCou = 1;
        pRefRun->_RunPtr = nullptr;
    }

    void LINKREFTAB::RunInitNxtRef(RefRun* pRefRun)
    {
        pRefRun->_RunCou = 1;
        pRefRun->_RunPtr = 0;

        REFTAB::MakeDirty();
    }

    void LINKREFTAB::RunInitPrevRef(RefRun* pRefRun) const
    {
        pRefRun->_RunCou = -1;
        pRefRun->_RunPtr = nullptr;
    }

    void LINKREFTAB::RunInitPrevRef(RefRun* pRefRun)
    {
        pRefRun->_RunCou = -1;
        pRefRun->_RunPtr = nullptr;
        
        REFTAB::MakeDirty();
    }

    const uint32_t* LINKREFTAB::RunNxtRefPtr(RefRun* pRefRun) const
    {
        TabBlk* Next = nullptr;

        if (pRefRun->_RunPtr)
        {
            Next = pRefRun->_RunPtr->_Next;
        }
        else
        {
            Next = (TabBlk*)First;
        }

        pRefRun->_RunPtr = Next;

        if (Next)
        {
            return (uint32_t*)Next + 2 - EleSize;
        }

        return nullptr;
    }

    uint32_t* LINKREFTAB::RunNxtRefPtr(RefRun* pRefRun)
    {
        return const_cast<uint32_t*>(const_cast<const LINKREFTAB*>(this)->RunNxtRefPtr(pRefRun));
    }

    const uint32_t* LINKREFTAB::RunPrevRefPtr(RefRun* pRefRun) const
    {
        TabBlk* Prev;

        if (pRefRun->_RunPtr)
        {
            Prev = pRefRun->_RunPtr->_Prev;
        }
        else
        {
            Prev = (TabBlk*)Last;
        }

        pRefRun->_RunPtr = Prev;

        if (Prev)
        {
            return (uint32_t*)Prev + 2 - EleSize;
        }

        return 0;
    }

    uint32_t* LINKREFTAB::RunPrevRefPtr(RefRun* pRefRun)
    {
        return const_cast<uint32_t*>(const_cast<const LINKREFTAB*>(this)->RunPrevRefPtr(pRefRun));
    }
    
    const uint32_t* LINKREFTAB::RunToRefPtr(RefRun *pRefRun) const
    {
        uint32_t* result = (uint32_t *)pRefRun->_RunPtr;
        if ( pRefRun->_RunPtr )
        {
            result += 2 - EleSize;
        }

        return result;
    }
    
    void LINKREFTAB::RemoveFreeStack()
    {
        if (FreeStack)
        {
            REFTAB::DeleteReftab(FreeStack);
            FreeStack = nullptr;
        }
    }
    
    void LINKREFTAB::CreateFreeStack()
    {
        ZASSERT(FreeStack == nullptr); // Otherwise mem leak (same in original code)
        FreeStack = REFTAB::MakeReftab(8, 0);
    }

    uint32_t* LINKREFTAB::AddStart(uint32_t rRef)
    {
        uint32_t* pRecord = AllocateRecord(rRef);

        PrevNext* pNode = GetPrevNext(pRecord);

        pNode->Next = First;

        if (First)
            First->Prev = pNode;
        else
            Last = pNode;

        First = pNode;

        return pRecord + 1;
    }
    
    uint32_t* LINKREFTAB::AddEnd(uint32_t rRef)
    {
        return Add(rRef);
    }

    uint32_t* LINKREFTAB::InsertBefore(uint32_t *pBefore, uint32_t rRef)
    {
        if (!pBefore)
            return Add(rRef);

        uint32_t* pRecord = AllocateRecord(rRef);

        PrevNext* pNode = GetPrevNext(pRecord);
        PrevNext* pBeforeNode = GetPrevNext(pBefore);

        pNode->Prev = pBeforeNode->Prev;
        pNode->Next = pBeforeNode;

        if (pBeforeNode->Prev)
            pBeforeNode->Prev->Next = pNode;
        else
            First = pNode;

        pBeforeNode->Prev = pNode;

        return pRecord + 1;
    }

    uint32_t* LINKREFTAB::GetPrevRefPtr(uint32_t* pRecord)
    {
        auto* pPrev = GetPrevNext(pRecord)->Prev;
        return pPrev ? GetRecord(pPrev) : nullptr;
    }

    uint32_t* LINKREFTAB::GetNextRefPtr(uint32_t* pRecord)
    {
        auto* pNext = GetPrevNext(pRecord)->Next;
        return pNext ? GetRecord(pNext) : nullptr;
    }

    uint32_t* LINKREFTAB::GetRefPtrNr(int lRefNo)
    {
        RefRun it;

        RunInitNxtRef(&it);

        auto* res = RunNxtRefPtr(&it);
        if (!res)
            return nullptr;

        while (lRefNo--)
        {
            res = RunNxtRefPtr(&it);
            if (!res)
                return nullptr;
        }

        return res;
    }

    uint32_t LINKREFTAB::GetRefNr(int lRefNo)
    {
        RefRun it;
        RunInitNxtRef(&it);

        auto res = RunNxtRef(&it);
        if (!it)
            return 0;

        while (lRefNo--)
        {
            res = RunNxtRef(&it);
            if (!it)
                return 0;
        }

        return res;
    }

    uint32_t* LINKREFTAB::AllocateRecord(uint32_t rRef)
    {
        uint32_t* pRecord;

        if (FreeStack)
        {
            RefRun it;
            FreeStack->RunInitNxtRef(&it);

            auto* pNode = reinterpret_cast<PrevNext*>(FreeStack->RunNxtRef(&it));

            ZASSERT(it);

            pRecord = GetRecord(pNode);

            FreeStack->RunDelRef(&it);

            if (!FreeStack->Count())
            {
                RemoveFreeStack();
                FreeStack = nullptr;
            }
        }
        else
        {
            pRecord = REFTAB::Add(rRef) - 1;
        }

        *pRecord = rRef;

        PrevNext* pNode = GetPrevNext(pRecord);
        pNode->Prev = nullptr;
        pNode->Next = nullptr;

        return pRecord;
    }
}