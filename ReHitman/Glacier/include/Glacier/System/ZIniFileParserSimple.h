#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <cstdint>


namespace Glacier
{
    class ZIniFileParserSimple
    {
    public:
        ZIniFileParserSimple();
        ~ZIniFileParserSimple();

        MYSTR ProcessCmdLine(const char* pszCmdLine);
        bool ParseIniFile(const char* pszContents);
    };

#ifdef REHITMAN_TESTS
    namespace ZIniFileParserSimpleTests
    {
        bool IsSpace(char c);
        void RemoveTrailingSpaces(char* pszText);
        const char* CopyName(char* pszOut, const char* pszIn);
        const char* CopyArgs(char* pszOut, const char* pszIn);
        void ParseOptionLine(const char* pszLine, char* pszName, char* pszArgs);
    }
#endif
}
