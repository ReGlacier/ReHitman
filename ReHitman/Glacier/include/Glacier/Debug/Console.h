#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZCyclicBuffer.h>
#include <Glacier/ZSTL/ZCompletionList.h>
#include <cstdint>


namespace Glacier
{
    class CConsole
    {
    public:
        // vtbl
        ~CConsole();
        void Update();
        void AddLine(const char* pFormat, ...);
        const char* GetLine(int lLine);
        void Command();
        void ExecuteCommand(const char* szCommand);
        void SplitToArgs(const char* pszString, uint32_t* argc, const char*** argv);
        void FlipDown();
        bool IsDown() const;
        void GetHistory(int lDirection);
        void AddHistory(const char* szLine);
        float GetOffset();
        void SetOffset(float fOffset);

        // methods
        CConsole();
        static int OnInput(unsigned int iKey, int iMask, void* pUserData);

        // members
        bool m_bDown;
        bool m_bIsDown;
        float m_fOffset;
        float m_fSpeed;
        int m_lLine;
        int m_lScrollLine;
        bool m_bLastCommmandTab;
        int m_iCurrentCommand;
        char m_szCommandLine[100];
        int m_lCommandLinePos;
        int m_iHistPos;
        ZCompletionList* m_pkCompletions;
        ZCyclicBuffer* m_pkCommands;
        ZCyclicBuffer* m_pkHistory;
    };
    RE_VERIFY_SIZE(CConsole, 0x94);

    extern CConsole* g_pConsole;
}
