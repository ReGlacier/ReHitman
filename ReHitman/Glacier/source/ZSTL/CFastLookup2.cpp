#include <Glacier/ZSTL/CFastLookup2.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/ZValTree.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>

namespace Glacier
{
    namespace
    {
        LINKREFTAB* LookupListFromTreeValue(uint32_t lValue)
        {
            return reinterpret_cast<LINKREFTAB*>(static_cast<uintptr_t>(lValue));
        }

        int LookupListToTreeValue(LINKREFTAB* pList)
        {
            return static_cast<int>(reinterpret_cast<uintptr_t>(pList));
        }

        SFastLookupLink* AllocateFastLookupLink(LINKREFTAB* pList)
        {
            // LINKREFTAB::Add returns the user pointer after the ref dword.
            return reinterpret_cast<SFastLookupLink*>(pList->Add(0) - 1);
        }
    }

    CFastLookup2::CFastLookup2(int lRefTabSize)
    {
        m_pChkSumTree = ZUniMemory::New<ZValTree>(lRefTabSize);
        m_bStatic = false;
        m_bRuntime = false;
    }

    CFastLookup2::~CFastLookup2()
    {
        Clear();

        if (m_pChkSumTree)
        {
            ZUniMemory::Delete(m_pChkSumTree);
            m_pChkSumTree = nullptr;
        }
    }

    void CFastLookup2::Clear()
    {
        while (SBinTreeNode* pTopNode = m_pChkSumTree->GetTopNode())
        {
            auto* pList = LookupListFromTreeValue(m_pChkSumTree->GetKeyVal(pTopNode->m_lKey));
            ZASSERT(pList != nullptr);

            if (!m_bStatic)
            {
                RefRun run;
                pList->RunInitNxtRef(&run);

                while (uint32_t* pRef = pList->RunNxtRefPtr(&run))
                {
                    ZUniMemory::Free(reinterpret_cast<void*>(*pRef));
                }
            }

            ZUniMemory::Delete(pList);
            m_pChkSumTree->Delete(pTopNode);
        }

        m_bStatic = false;
        m_bRuntime = false;
    }

    int CFastLookup2::CalcChkSum(const char* pLookup, int lLookupLen)
    {
        int lChkSum = lLookupLen;

        for (int i = 0; i != (lLookupLen >> 2); ++i)
        {
            lChkSum += (static_cast<uint8_t>(pLookup[4 * i + 3]) << 24)
                | (static_cast<uint8_t>(pLookup[4 * i + 2]) << 16)
                | (static_cast<uint8_t>(pLookup[4 * i + 1]) << 8)
                | static_cast<uint8_t>(pLookup[4 * i]);
        }

        for (uint32_t i = lLookupLen & 0xFFFFFFFC; i != static_cast<uint32_t>(lLookupLen); ++i)
        {
            lChkSum += pLookup[i];
        }

        return lChkSum;
    }

    void CFastLookup2::SetLowerCase(const char* pLookup, uint32_t lVal)
    {
        MYSTR lookup(pLookup);
        MYSTR lowerLookup(lookup);

        lowerLookup.ToLower();
        Set(lowerLookup, lVal);
    }

    void CFastLookup2::Set(const char* pLookup, uint32_t lVal)
    {
        ZASSERT(!m_bStatic);

        const size_t lLookupLen = std::strlen(pLookup);
        auto* pLookupCopy = static_cast<char*>(ZUniMemory::Allocate(static_cast<int>(lLookupLen + 1)));
        std::strcpy(pLookupCopy, pLookup);

        const int lCopiedLookupLen = static_cast<int>(std::strlen(pLookupCopy));
        SFastLookupLink* pFastLink = PrepareFastLink(pLookupCopy, lCopiedLookupLen);

        pFastLink->m_pLookup = pLookupCopy;
        pFastLink->m_lLookupLen = lCopiedLookupLen;
        pFastLink->m_lValue = lVal;
        m_bRuntime = true;
    }

    void CFastLookup2::Reset()
    {
        Clear();
    }

    void CFastLookup2::RemoveLowerCase(const char* pLookup)
    {
        const int lLookupLen = static_cast<int>(std::strlen(pLookup));
        MYSTR lookup(pLookup);
        MYSTR lowerLookup(lookup);

        lowerLookup.ToLower();
        Remove(lowerLookup, lLookupLen);
    }

    uint32_t CFastLookup2::Remove(const char* pLookup, int lLookupLen)
    {
        if (!lLookupLen)
        {
            lLookupLen = static_cast<int>(std::strlen(pLookup));
        }

        const int lChkSum = CalcChkSum(pLookup, lLookupLen);
        SBinTreeNode* pTreeNode = m_pChkSumTree->Search(lChkSum, nullptr);
        if (!pTreeNode)
        {
            return 0;
        }

        auto* pList = LookupListFromTreeValue(static_cast<SValTreeNode*>(pTreeNode)->m_lValue);
        if (!pList)
        {
            return 0;
        }

        SFastLookupLink* pFastLink = FindFastLink(pList, pLookup, lLookupLen);
        if (!pFastLink)
        {
            return 0;
        }

        if (!m_bStatic)
        {
            ZUniMemory::Free(const_cast<char*>(pFastLink->m_pLookup));
        }

        pList->DelRefPtr(reinterpret_cast<uint32_t*>(pFastLink));
        if (pList->Count())
        {
            return pList->Count();
        }

        ZUniMemory::Delete(pList);
        m_pChkSumTree->Delete(pTreeNode);
        return 0;
    }

    SFastLookupLink* CFastLookup2::PrepareFastLink(const char* pLookup, int lLookupLen)
    {
        const int lChkSum = CalcChkSum(pLookup, lLookupLen);
        auto* pList = LookupListFromTreeValue(m_pChkSumTree->GetKeyVal(lChkSum));
        SFastLookupLink* pFastLink = nullptr;

        if (pList)
        {
            pFastLink = FindFastLink(pList, pLookup, lLookupLen);
        }
        else
        {
            pList = ZUniMemory::New<LINKREFTAB>(4, 2);
            m_pChkSumTree->InsertKey(lChkSum, LookupListToTreeValue(pList));
        }

        if (pFastLink)
        {
            if (!m_bStatic)
            {
                ZUniMemory::Free(const_cast<char*>(pFastLink->m_pLookup));
            }

            pFastLink->m_pLookup = nullptr;
            return pFastLink;
        }

        return AllocateFastLookupLink(pList);
    }

    int CFastLookup2::GetLowerCase(const char* pLookup)
    {
        MYSTR lookup(pLookup);
        MYSTR lowerLookup(lookup);

        lowerLookup.ToLower();
        return Get(lowerLookup);
    }

    uint32_t CFastLookup2::Get(const char* pLookup)
    {
        const int lLookupLen = static_cast<int>(std::strlen(pLookup));
        const int lChkSum = CalcChkSum(pLookup, lLookupLen);
        auto* pList = LookupListFromTreeValue(m_pChkSumTree->GetKeyVal(lChkSum));

        if (pList)
        {
            if (SFastLookupLink* pFastLink = FindFastLink(pList, pLookup, lLookupLen))
            {
                return pFastLink->m_lValue;
            }
        }

        return 0;
    }

    SFastLookupLink* CFastLookup2::FindFastLink(LINKREFTAB* pList, const char* pLookup, int lLookupLen)
    {
        if (pList)
        {
            RefRun run;
            pList->RunInitNxtRef(&run);

            while (uint32_t* pRef = pList->RunNxtRefPtr(&run))
            {
                auto* pFastLink = reinterpret_cast<SFastLookupLink*>(pRef);
                if (pFastLink->m_lLookupLen == lLookupLen && !std::memcmp(pFastLink->m_pLookup, pLookup, lLookupLen))
                {
                    return pFastLink;
                }
            }
        }

        return nullptr;
    }
}
