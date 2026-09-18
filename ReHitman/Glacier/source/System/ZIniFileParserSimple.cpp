#include <Glacier/System/ZIniFileParserSimple.h>
#include <Glacier/System/ZSysInterface.h>

#include <cstring>

#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        constexpr int kMaxOptionLineLength = 0x1FF;
        constexpr int kLineBufferSize = 0x200;

        bool IsSpaceImpl(char c)
        {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n';
        }

        int CompareNoCase(const char* lhs, const char* rhs)
        {
            ZASSERT(lhs != nullptr);
            ZASSERT(rhs != nullptr);

            while (*lhs && *rhs)
            {
                char l = *lhs++;
                char r = *rhs++;

                if (l >= 'A' && l <= 'Z')
                {
                    l = static_cast<char>(l + ('a' - 'A'));
                }

                if (r >= 'A' && r <= 'Z')
                {
                    r = static_cast<char>(r + ('a' - 'A'));
                }

                if (l != r)
                {
                    return static_cast<unsigned char>(l) - static_cast<unsigned char>(r);
                }
            }

            return static_cast<unsigned char>(*lhs) - static_cast<unsigned char>(*rhs);
        }

        void RemoveTrailingSpacesImpl(char* pszText)
        {
            ZASSERT(pszText != nullptr);

            int length = static_cast<int>(std::strlen(pszText));
            if (length <= 0)
            {
                return;
            }

            for (char* current = &pszText[length - 1]; current != pszText; --current)
            {
                if (!IsSpaceImpl(*current))
                {
                    break;
                }

                *current = '\0';
            }
        }

        const char* CopyNameImpl(char* pszOut, const char* pszIn)
        {
            ZASSERT(pszOut != nullptr);
            ZASSERT(pszIn != nullptr);

            while (IsSpaceImpl(*pszIn))
            {
                ++pszIn;
            }

            while (*pszIn && !IsSpaceImpl(*pszIn))
            {
                *pszOut++ = *pszIn++;
            }

            *pszOut = '\0';
            return pszIn;
        }

        const char* CopyArgsImpl(char* pszOut, const char* pszIn)
        {
            ZASSERT(pszOut != nullptr);
            ZASSERT(pszIn != nullptr);

            while (IsSpaceImpl(*pszIn))
            {
                ++pszIn;
            }

            while (*pszIn)
            {
                *pszOut++ = *pszIn++;
            }

            *pszOut = '\0';
            return pszIn;
        }

        void ParseOptionLineImpl(const char* pszLine, char* pszName, char* pszArgs)
        {
            ZASSERT(pszLine != nullptr);
            ZASSERT(pszName != nullptr);
            ZASSERT(pszArgs != nullptr);
            ZASSERT(std::strlen(pszLine) < kMaxOptionLineLength);

            char line[kLineBufferSize]{};
            std::strcpy(line, pszLine);

            char* separator = std::strchr(line, '=');
            if (separator)
            {
                *separator = ' ';
            }

            const char* args = CopyNameImpl(pszName, line);
            CopyArgsImpl(pszArgs, args);
            RemoveTrailingSpacesImpl(pszArgs);
        }
    }

    ZIniFileParserSimple::ZIniFileParserSimple() = default;
    ZIniFileParserSimple::~ZIniFileParserSimple() = default;

    bool ZIniFileParserSimple::ParseIniFile(const char* pszContents)
    {
        ZASSERT(pszContents != nullptr);
        ZASSERT(g_pSysInterface != nullptr);

        const char* current = pszContents;
        char line[kLineBufferSize]{};
        char name[kLineBufferSize]{};
        char args[kLineBufferSize]{};

        while (*current)
        {
            while (*current && (*current == '\r' || *current == '\n'))
            {
                ++current;
            }

            int lineLength = 0;
            while (*current && *current != '\r' && *current != '\n')
            {
                ZASSERT(lineLength < kLineBufferSize - 1);
                line[lineLength++] = *current++;
            }

            line[lineLength] = '\0';
            RemoveTrailingSpacesImpl(line);

            if (line[0] != '/' && line[1] != '/')
            {
                ParseOptionLineImpl(line, name, args);

                if (CompareNoCase(name, "ConsoleCmd") == 0)
                {
                    g_pSysInterface->m_sCmdLine += MYSTR(args);
                    g_pSysInterface->m_sCmdLine += MYSTR("\n");
                }

                g_pSysInterface->SetOption(name, args);
            }
        }

        return true;
    }

    MYSTR ZIniFileParserSimple::ProcessCmdLine(const char* pszCmdLine)
    {
        MYSTR cmdLine(pszCmdLine);
        RemoveTrailingSpacesImpl(static_cast<char*>(cmdLine));

        MYSTR result;
        if (static_cast<const char*>(cmdLine)[0] == '@')
        {
            result = MYSTR(static_cast<const char*>(cmdLine) + 1);
        }
        else
        {
            result = cmdLine;
        }

        if (std::strcmp(static_cast<const char*>(result), "") == 0)
        {
            result = MYSTR("main.ini");
        }

        return result;
    }

#ifdef REHITMAN_TESTS
    namespace ZIniFileParserSimpleTests
    {
        bool IsSpace(char c)
        {
            return IsSpaceImpl(c);
        }

        void RemoveTrailingSpaces(char* pszText)
        {
            RemoveTrailingSpacesImpl(pszText);
        }

        const char* CopyName(char* pszOut, const char* pszIn)
        {
            return CopyNameImpl(pszOut, pszIn);
        }

        const char* CopyArgs(char* pszOut, const char* pszIn)
        {
            return CopyArgsImpl(pszOut, pszIn);
        }

        void ParseOptionLine(const char* pszLine, char* pszName, char* pszArgs)
        {
            ParseOptionLineImpl(pszLine, pszName, pszArgs);
        }
    }
#endif

}
