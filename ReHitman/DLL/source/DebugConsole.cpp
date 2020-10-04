#include <DLL/DebugConsole.h>
#include <Windows.h>

namespace ReHitman
{
    bool DebugConsole::g_bIsCreated = false;

    void DebugConsole::Create(std::string_view caption)
    {
        if (g_bIsCreated || caption.empty())
            return;

        if (AllocConsole()) {
            freopen("CONOUT$", "w", stdout);
            SetConsoleTitle(caption.data());
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);

            g_bIsCreated = true;
        }
    }
}