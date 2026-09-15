#include <Glacier/ZSTL/ZCompletionList.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        char szEmptyString[] = "";

        int StrCaseCmp(const char* pszLhs, const char* pszRhs)
        {
#ifdef _WIN32
            return _stricmp(pszLhs, pszRhs);
#else
            return StrCaseCmp(pszLhs, pszRhs);
#endif
        }

        int StrNCaseCmp(const char* pszLhs, const char* pszRhs, size_t iCount)
        {
#ifdef _WIN32
            return _strnicmp(pszLhs, pszRhs, iCount);
#else
            return StrNCaseCmp(pszLhs, pszRhs, iCount);
#endif
        }
    }

    ZCompletionList::ZCompletionList(int iBufferSize)
    {
        m_iBufferSize = iBufferSize;
        m_pBuffer = static_cast<char*>(ZUniMemory::Allocate(iBufferSize));

        if (!m_pBuffer)
            m_iBufferSize = 0;

        Clear();
    }

    ZCompletionList::~ZCompletionList()
    {
        if (m_iBufferSize && m_pBuffer)
            ZUniMemory::Free(m_pBuffer);
    }

    void ZCompletionList::Add(const char* pszString)
    {
        const int iLength = static_cast<int>(std::strlen(pszString));
        const int iPrefixLength = static_cast<int>(std::strlen(m_szPrefix));

        if (StrNCaseCmp(m_szPrefix, pszString, iPrefixLength) != 0)
            return;

        if (m_iAllocPos + iLength + 4 * m_iCount + 5 >= m_iBufferSize)
        {
            // Original prints "Console --> completion buffer overflow" through the debug system.
            std::printf("Console --> completion buffer overflow\n");
            return;
        }

        ++m_iCount;
        --m_pszStrings;
        *m_pszStrings = m_pBuffer + m_iAllocPos;
        std::memcpy(*m_pszStrings, pszString, iLength + 1);
        m_iAllocPos += iLength + 1;
    }

    void ZCompletionList::Prefix(char* pszPrefix)
    {
        m_szPrefix = pszPrefix;
    }

    void ZCompletionList::Unique()
    {
        Sort();

        for (int i = 0; i < m_iCount - 1; ++i)
        {
            if (StrCaseCmp(m_pszStrings[i], m_pszStrings[i + 1]) == 0)
                m_pszStrings[i] = nullptr;
        }

        for (int j = m_iCount - 1; j >= 0; --j)
        {
            if (m_pszStrings[j])
                continue;

            int k = j - 1;
            while (k >= 0 && !m_pszStrings[k])
                --k;

            if (k < 0)
            {
                m_iCount = m_iCount - j - 1;
                break;
            }

            Swap(k, j);
        }

        m_pszStrings = reinterpret_cast<char**>(m_pBuffer + m_iBufferSize);
        m_pszStrings -= m_iCount;
    }

    void ZCompletionList::Sort()
    {
        for (int i = m_iCount / 2; i >= 0; --i)
            Heapify(i);

        const int iSavedCount = m_iCount;

        for (int j = iSavedCount - 1; j > 0; --j)
        {
            Swap(0, j);
            --m_iCount;
            Heapify(0);
        }

        m_iCount = iSavedCount;
    }

    void ZCompletionList::Clear()
    {
        m_iAllocPos = 0;
        m_pszStrings = reinterpret_cast<char**>(m_pBuffer + m_iBufferSize);
        m_iCount = 0;
        m_szPrefix = szEmptyString;
    }

    int ZCompletionList::Count()
    {
        return m_iCount;
    }

    char* ZCompletionList::Get(int iIndex)
    {
        return m_pszStrings[iIndex];
    }

    void ZCompletionList::LongestCommonPrefix(char*& pszPrefix, int& iLength)
    {
        if (!m_iCount)
        {
            iLength = 0;
            pszPrefix = nullptr;
            return;
        }

        pszPrefix = Get(0);
        iLength = static_cast<int>(std::strlen(pszPrefix));

        for (int i = 1; i < m_iCount; ++i)
        {
            const char* pszEntry = Get(i);

            int j = 0;
            while (j < iLength && pszEntry[j] && StrNCaseCmp(pszEntry + j, pszPrefix + j, 1) == 0)
                ++j;

            iLength = j;
        }
    }

    bool ZCompletionList::LTH(int iLhs, int iRhs)
    {
        return StrCaseCmp(m_pszStrings[iLhs], m_pszStrings[iRhs]) < 0;
    }

    void ZCompletionList::Swap(int iLhs, int iRhs)
    {
        char* pszTemp = m_pszStrings[iLhs];
        m_pszStrings[iLhs] = m_pszStrings[iRhs];
        m_pszStrings[iRhs] = pszTemp;
    }

    void ZCompletionList::Heapify(int iIndex)
    {
        const int iLeft = 2 * iIndex + 1;
        const int iRight = 2 * iIndex + 2;

        int iSmallest = iIndex;

        if (iLeft < m_iCount && LTH(iIndex, iLeft))
            iSmallest = iLeft;

        if (iRight < m_iCount && LTH(iSmallest, iRight))
            iSmallest = iRight;

        if (iSmallest != iIndex)
        {
            Swap(iIndex, iSmallest);
            Heapify(iSmallest);
        }
    }
}
