#include <Glacier/ZSTL/STRREFTAB.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    bool AlphaiCompare(const char* pStr1, const char* pStr2)
    {
        if (stricmp(pStr1, pStr2) == 0)
            return false;

        const size_t minLen = std::min(strlen(pStr1), strlen(pStr2));

        for (size_t i = 0; i < minLen; ++i)
        {
            char c1 = pStr1[i];
            char c2 = pStr2[i];

            if (c1 >= 'A' && c1 <= 'Z')
                c1 += 'a' - 'A';

            if (c2 >= 'A' && c2 <= 'Z')
                c2 += 'a' - 'A';

            if (c1 > c2)
                return true;

            if (c1 < c2)
                return false;
        }

        return pStr2[minLen] == '\0';
    }

    int QSortAlphaCompare(const void* a, const void* b)
    {
        const auto* lhs = static_cast<char* const*>(a);
        const auto* rhs = static_cast<char* const*>(b);

        return AlphaiCompare(*lhs, *rhs) ? 1 : -1;
    }

    STRREFTAB::STRREFTAB(int pPoolSize, int pUserData) 
        : REFTAB(pPoolSize, pUserData) 
    {
    }

    STRREFTAB::~STRREFTAB()
    {
        ClearThis();
    }

    void STRREFTAB::Clear()
    {
        ClearThis();

        TabBlk* pToDelete = nullptr;
        while (TabFirstPtr)
        {
            pToDelete = TabFirstPtr;
            TabFirstPtr = TabFirstPtr->_Next;
            DeleteBlock(pToDelete);
        }
        
        EleCount = 0;
        TabFirstPtr = nullptr;
        TabBlockPtr = nullptr;
    }

    void STRREFTAB::ClearThis()
    {
        RefRun it;
        RunInitNxtRef(&it);

        for (void* i = (void*)RunNxtRef(&it); it ; i = (void*)RunNxtRef(&it))
        {
            ZUniMemory::Free(i);
        }
    }

    void STRREFTAB::RunDelRef(RefRun* pRefRun)
    {
        char* pString;

        if (pRefRun->_RunDir <= 0)
        {
            pString = reinterpret_cast<char*>(*RunToRefPtr(pRefRun));
        }
        else
        {
            int runCou = pRefRun->_RunCou - EleSize;
            TabBlk* pRunBlk = pRefRun->_RunPtr;

            if (runCou < 0)
            {
                pRunBlk = pRunBlk->_Prev;
                runCou = BlkSize - EleSize;
            }

            auto* pSlot = reinterpret_cast<uint32_t*>(pRunBlk + 1) + runCou;
            pString = reinterpret_cast<char*>(*pSlot);
        }

        ZUniMemory::Free(pString);
        REFTAB::RunDelRef(pRefRun);
    }

    uint32_t* STRREFTAB::FindStr(const char* pString)
    {
        RefRun it;
        RunInitNxtRef(&it);

        auto* pRef = reinterpret_cast<uint32_t*>(RunNxtRefPtr(&it));
        if (!pRef)
            return nullptr;

        while (true)
        {
            const char* pCurrentString = reinterpret_cast<const char*>(*pRef);

            if ((m_bCaseSensitive ? strcmp(pCurrentString, pString) : stricmp(pCurrentString, pString)) == 0)
            {
                return pRef;
            }

            pRef = reinterpret_cast<uint32_t*>(RunNxtRefPtr(&it));
            if (!pRef)
                return nullptr;
        }
    }

    uint32_t* STRREFTAB::AddStr(const char* pString)
    {
        if (Exists(pString))
        {
            return nullptr;
        }

        return AddAlways(pString);
    }

    uint32_t* STRREFTAB::AddAlways(const char* pString)
    {
        auto* pCopy = (char*)ZUniMemory::Allocate(strlen(pString) + 1);
        strcpy(pCopy, pString);
        return REFTAB::Add((uint32_t)pCopy);
    }

    void STRREFTAB::RemoveStr(const char* pString)
    {
        RefRun it;
        RunInitNxtRef(&it);

        char* pCurrent = (char*)RunNxtRef(&it);
        if (it)
        {
            while (!Equals(pCurrent, pString))
            {
                pCurrent = (char*)RunNxtRef(&it);
                if (!it)
                {
                    return;
                }
            }
            RunDelRef(&it);
        }
    }

    bool STRREFTAB::Exists(const char* pString)
    {
        for (const auto* entry : *this)
        {
            if (Equals(pString, entry))
            {
                return true;
            }
        }

        return false;
    }

    void STRREFTAB::Sort()
    {    
        // Generate temp buffer
        const uint32_t iCount = Count();
        char** pBuffer = (char**)(ZUniMemory::Allocate(sizeof(char*) * iCount));
        int iIndex = 0;
        for (const auto* entry : *this)
        {
            pBuffer[iIndex] = const_cast<char*>(entry);
            ++iIndex;
        }

        // Sort temp buffer via custom comparator
        qsort(pBuffer, iIndex, sizeof(char*), QSortAlphaCompare);

        // Clear current blk memory (but not strings! there are copied into pBuffer[...])
        REFTAB::Clear();

        // Insert all entries back
        for (iIndex = 0; iIndex < iCount; ++iIndex)
        {
            AddAlways(pBuffer[iIndex]); // Copy copy to copy copy
            ZUniMemory::Free(pBuffer[iIndex]); // Clear after copy (prev allocated chkmem)
        }

        // Clear pool mem
        ZUniMemory::Free(pBuffer);
    }

    bool STRREFTAB::Equals(const char* pString1, const char* pString2) const
    {
        return (m_bCaseSensitive ? strcmp(pString1, pString2) : stricmp(pString1, pString2)) == 0;
    }
}