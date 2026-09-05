#include <Glacier/Debug/ZDebugCommand.h>

#include <cstring>


namespace Glacier
{
    ZDebugCommand* ZDebugCommand::m_pFirst = nullptr;

    ZDebugCommand::ZDebugCommand(const char* pszName, const char* pszDescription)
    {
        m_pszName = pszName;
        m_pszDescription = pszDescription;
        m_pNext = nullptr;
        m_pszPath = nullptr;
        m_szValue[0] = '\0';

        // Link into the global registry list (append at the tail).
        ZDebugCommand* pCursor = m_pFirst;
        ZDebugCommand* pLast = nullptr;

        while (pCursor)
        {
            pLast = pCursor;
            pCursor = pCursor->m_pNext;
        }

        if (pLast)
            pLast->m_pNext = this;
        else
            m_pFirst = this;
    }

    ZDebugCommand::~ZDebugCommand()
    {
        ZDebugCommand* pPrev = nullptr;

        for (ZDebugCommand* pCursor = m_pFirst; pCursor; pCursor = pCursor->m_pNext)
        {
            if (pCursor == this)
            {
                if (pPrev)
                    pPrev->m_pNext = m_pNext;
                else
                    m_pFirst = m_pNext;

                return;
            }

            pPrev = pCursor;
        }
    }

    ECLASSTYPE ZDebugCommand::GetType()
    {
        return ECLASS_UNKNOWN;
    }

    void ZDebugCommand::GetTabCompletionList(uint32_t iArgC, const char** ppszArgV, ZCompletionList* pList)
    {
    }

    void ZDebugCommand::Inc()
    {
    }

    void ZDebugCommand::Dec()
    {
    }

    void ZDebugCommand::CalcValue()
    {
    }

    char* ZDebugCommand::GetValue()
    {
        return m_szValue;
    }

    ZDebugCommand* ZDebugCommand::First()
    {
        return m_pFirst;
    }

    ZDebugCommand* ZDebugCommand::Next()
    {
        return m_pNext;
    }

    const char* ZDebugCommand::Name()
    {
        return m_pszName;
    }

    const char* ZDebugCommand::Description()
    {
        return m_pszDescription;
    }

    const char* ZDebugCommand::GetPath()
    {
        return m_pszPath;
    }
}
