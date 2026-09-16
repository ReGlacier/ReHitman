#include <Glacier/Debug/console.h>

#include <Glacier/Debug/ZDebugCommand.h>

#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>


namespace Glacier
{
    CConsole g_Console;
    CConsole* g_pConsole = &g_Console;

    CConsole::CConsole()
    {
        m_bDown = false;
        m_bIsDown = false;
        m_fOffset = 0.0f;
        m_fSpeed = 0.2f;
        m_szCommandLine[0] = '>';
        m_szCommandLine[1] = '\0';
        m_lCommandLinePos = 1;
        m_lLine = 0;
        m_lScrollLine = -1;

        m_pkCompletions = ZUniMemory::New<ZCompletionList>(0x2000);
        m_pkCommands = ZUniMemory::New<ZCyclicBuffer>(10240);
        m_pkHistory = ZUniMemory::New<ZCyclicBuffer>(1024);
    }

    CConsole::~CConsole()
    {
        ZUniMemory::Delete(m_pkCompletions);
        ZUniMemory::Delete(m_pkCommands);
        ZUniMemory::Delete(m_pkHistory);
    }

    bool CConsole::IsDown() const
    {
        return m_bDown;
    }

    float CConsole::GetOffset()
    {
        return m_fOffset;
    }

    void CConsole::SetOffset(float fOffset)
    {
        m_fOffset = fOffset;
    }

    void CConsole::Update()
    {
        if (m_bDown)
        {
            m_fOffset += m_fSpeed;
            m_bIsDown = true;
            if (m_fOffset < 1.0f)
                return;
        }
        else
        {
            m_fOffset -= m_fSpeed;
            if (m_fOffset > 0.0f)
                return;
            m_bIsDown = false;
        }

        m_fOffset = m_bDown ? 1.0f : 0.0f;
    }

    void CConsole::AddLine(const char* pFormat, ...)
    {
        char szText[1024];

        va_list args;
        va_start(args, pFormat);
        int lLen = std::vsnprintf(szText, sizeof(szText), pFormat, args);
        va_end(args);
        if (lLen >= (int)sizeof(szText))
            lLen = (int)sizeof(szText) - 1;

        char* pEnd = szText + lLen;
        while (pEnd > szText && (pEnd[-1] == '\n' || pEnd[-1] == '\r'))
            *--pEnd = '\0';

        for (char* p = strtok(szText, "\n"); p; p = strtok(nullptr, "\n"))
        {
            void* pRecord = m_pkHistory->Alloc(static_cast<int>(std::strlen(p) + 1));
            if (pRecord)
                std::memcpy(pRecord, p, std::strlen(p) + 1);
        }
    }

    void CConsole::GetHistory(int lDirection)
    {
        int lHistPos = m_iHistPos + lDirection;
        m_iHistPos = lHistPos;

        int lFirst = -m_pkCommands->m_iCount;
        if (lHistPos < lFirst)
            m_iHistPos = lFirst;

        if (m_iHistPos < 0)
        {
            char* pRecord = static_cast<char*>(m_pkCommands->Get(m_iHistPos));
            if (pRecord)
            {
                std::strcat(m_szCommandLine, pRecord);
                m_lCommandLinePos = static_cast<int>(std::strlen(m_szCommandLine)) - 1;
            }
        }
        else
        {
            m_iHistPos = 0;
            m_szCommandLine[1] = '\0';
            m_lCommandLinePos = 1;
        }
    }

    void CConsole::AddHistory(const char* szLine)
    {
        int lLen = static_cast<int>(std::strlen(szLine));
        void* pRecord = m_pkCommands->Alloc(lLen);
        if (pRecord)
            std::memcpy(pRecord, szLine, lLen);
        m_iHistPos = 0;
    }

    const char* CConsole::GetLine(int lLine)
    {
        // lLine >= 0 : the active command line (0).
        // lLine  < 0 : history, -1 == most recent, -2 == next older, etc.
        if (lLine >= 0)
            return m_szCommandLine;

        return static_cast<const char*>(m_pkHistory->Get(lLine));
    }

    void CConsole::SplitToArgs(const char* pszString, uint32_t* argc, const char*** argv)
    {
        char buffer[256];
        std::strncpy(buffer, pszString, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        const char* tokens[8];
        uint32_t count = 0;

        for (char* p = buffer; count < 8; )
        {
            while (*p == ' ' || *p == '\t')
                ++p;
            if (*p == '\0')
                break;
            tokens[count++] = p;
            while (*p != '\0' && *p != ' ' && *p != '\t')
                ++p;
            if (*p == ' ' || *p == '\t')
                *p++ = '\0';
        }

        *argc = count;
        *argv = count ? reinterpret_cast<const char**>(tokens) : nullptr;
    }

    void CConsole::Command()
    {
        if (m_szCommandLine[1])
            AddHistory(&m_szCommandLine[1]);

        m_lScrollLine = -1;

        uint32_t argc = 0;
        const char** argv = nullptr;
        SplitToArgs(&m_szCommandLine[1], &argc, &argv);

        if (argc)
        {
            AddLine("%s", m_szCommandLine);

            ZDebugCommand* pCommand = ZDebugCommand::First();
            bool lFound = false;
            while (pCommand)
            {
                if (std::strcmp(pCommand->Name(), argv[0]) == 0)
                {
                    pCommand->Execute(argc, argv);
                    lFound = true;
                    break;
                }
                pCommand = pCommand->Next();
            }

            if (!lFound)
                AddLine("Unknown command -use 'tab' to display list");

            m_szCommandLine[1] = '\0';
            m_lCommandLinePos = 1;
        }
    }

    void CConsole::ExecuteCommand(const char* szCommand)
    {
        std::snprintf(m_szCommandLine, sizeof(m_szCommandLine), ">%s", szCommand);
        Command();
    }

    void CConsole::FlipDown()
    {
        if (m_bDown)
        {
            m_bDown = false;
        }
        else
        {
            m_bDown = true;
            m_lScrollLine = -1;
        }
    }

    int CConsole::OnInput(unsigned int iKey, int iMask, void* pUserData)
    {
        CConsole* pThis = static_cast<CConsole*>(pUserData);
        if ((iMask & 0x200) == 0)
            return 1;

        switch (iKey)
        {
            case 0:   // Up (scroll up history view)
            {
                if (pThis->m_lScrollLine == -1)
                {
                    pThis->m_lScrollLine = pThis->m_pkHistory->m_iCount + pThis->m_pkHistory->m_iFirst - 1;
                    return 1;
                }
                if (pThis->m_lScrollLine <= 0)
                    return 1;
                pThis->m_lScrollLine -= 1;
                return 1;
            }
            case 1:   // Down
            {
                if (pThis->m_lScrollLine == -1)
                    return 1;
                int vNext = pThis->m_lScrollLine + 1;
                pThis->m_lScrollLine = vNext;
                if (vNext >= pThis->m_pkHistory->m_iCount + pThis->m_pkHistory->m_iFirst - 1)
                    pThis->m_lScrollLine = -1;
                return 1;
            }
            case 3:   // Home
                pThis->m_lScrollLine = pThis->m_pkHistory->m_iFirst;
                return 1;
            case 4:   // End
                pThis->m_lScrollLine = -1;
                return 1;
            case 5:   // Prev history
                pThis->m_bLastCommmandTab = false;
                pThis->GetHistory(-1);
                return 1;
            case 6:   // Next history
                pThis->m_bLastCommmandTab = false;
                pThis->GetHistory(1);
                return 1;
            case 9:   // Enter
                pThis->m_bLastCommmandTab = false;
                pThis->Command();
                return 1;
            case 10:  // Esc: close console
                pThis->m_bLastCommmandTab = false;
                pThis->m_bDown = false;
                return 0;
            case 11:  // Left
            case 12:  // Backspace
            {
                int lPos = pThis->m_lCommandLinePos;
                pThis->m_bLastCommmandTab = false;
                if (lPos <= 1)
                    return 1;
                int lNew = lPos - 1;
                pThis->m_lCommandLinePos = lNew;
                pThis->m_szCommandLine[lNew] = '\0';
                return 1;
            }
            case 13:  // Ctrl+A: start of line
                pThis->m_bLastCommmandTab = false;
                pThis->m_lCommandLinePos = 1;
                pThis->m_szCommandLine[1] = '\0';
                return 1;
            case 14:  // Tab: command completion
            {
                pThis->m_bLastCommmandTab = false;
                pThis->m_bDown = false;
                return 1;
            }
            default:
            {
                if (iKey > 31 && iKey < 100)
                {
                    uint32_t lPos = pThis->m_lCommandLinePos;
                    if (lPos < 99)
                    {
                        pThis->m_bLastCommmandTab = false;
                        pThis->m_lScrollLine = -1;
                        pThis->m_szCommandLine[lPos] = static_cast<char>(iKey);
                        pThis->m_lCommandLinePos = lPos + 1;
                        pThis->m_szCommandLine[lPos + 1] = '\0';
                    }
                }
                return 1;
            }
        }
    }
}
