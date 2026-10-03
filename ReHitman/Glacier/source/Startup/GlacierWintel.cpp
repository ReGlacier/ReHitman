#include <Glacier/Startup/GlacierStartup.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/System/ZSysInterface.h>
#include <Windows.h>
#include <signal.h>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        // PC .data 0x00820680. Only written here; no reversed reader yet.
        HINSTANCE g_hMainExe = nullptr;

        void SignalHandler(int lSignal)
        {
            if (g_pSysInterface->m_pSoundDll)
            {
                g_pSysInterface->m_pSoundDll->CrashFree();
            }

            if (g_pRenderDll)
            {
                g_pRenderDll->CrashFree();
            }
        }

        // PC sub_439550: build "<this><suffix>" into a fresh MYSTR (used for
        // the "@\"" / "\" " redirection pieces around the ini path).
        MYSTR Concat(const MYSTR& lhs, const MYSTR& suffix)
        {
            MYSTR result(lhs);
            result += suffix;
            return result;
        }
    }

    int Glacier_Main(int hInstance, int hPrevInstance, const char* psCmdLine)
    {
        CreateMutexA(nullptr, false, "Hitboy");
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            return 0;
        }

        signal(8, SignalHandler);
        signal(22, SignalHandler);
        signal(4, SignalHandler);
        signal(11, SignalHandler);

        g_hMainExe = reinterpret_cast<HINSTANCE>(hInstance);
        // PC additionally calls an intentionally empty stub here (sub_4715C0).

        // Assemble the start command line. If '@' is already present, the
        // caller supplied an explicit ini redirection; otherwise synthesize
        // the default "@<exe>\" / "main.ini" form.
        MYSTR sStartLine;
        if (strchr(psCmdLine, '@'))
        {
            sStartLine = psCmdLine;
        }
        else
        {
            char szExePath[MAX_PATH];
            GetModuleFileNameA(reinterpret_cast<HMODULE>(hInstance), szExePath, MAX_PATH);

            HANDLE hExe = CreateFileA(szExePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

            if (hExe == INVALID_HANDLE_VALUE)
            {
                // The exe file could not be opened; use the module name without
                // extension as the base name for "main.ini".
                sStartLine = szExePath;

                char* lastSlash = strrchr(static_cast<char*>(sStartLine), '\\');
                if (lastSlash)
                {
                    lastSlash[1] = '\0';
                }
                else
                {
                    sStartLine = "";
                }
                MYSTR iniPath = sStartLine + MYSTR("main.ini");

                HANDLE hIni = CreateFileA(static_cast<const char*>(iniPath), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

                if (hIni != INVALID_HANDLE_VALUE)
                {
                    // "@<path>main.ini" + cmd
                    sStartLine = Concat(MYSTR("@\""), iniPath) + MYSTR("\" ") + MYSTR(psCmdLine);
                    CloseHandle(hIni);
                }
                else
                {
                    HANDLE hMainIni = CreateFileA("main.ini", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (hMainIni != INVALID_HANDLE_VALUE)
                    {
                        sStartLine = MYSTR("@main.ini ") + MYSTR(psCmdLine);
                        CloseHandle(hMainIni);
                    }
                    else
                    {
                        sStartLine = "";
                    }
                }
            }
            else
            {
                CloseHandle(hExe);
                sStartLine = MYSTR("@\"") + MYSTR(szExePath) + MYSTR("\" ") + MYSTR(psCmdLine);
            }
        }

        InitGlacierSystem(hInstance, false, nullptr);

        // Default progress bar rectangle (fractional screen coords).
        g_pSysInterface->SetProgressBarRect(0.1f, 0.221f, 0.8f, 0.004f);

        g_pSysInterface->m_sConfigFile = "Hitman.cfg";
        g_pSysInterface->m_sGameTitle = "Hitman Blood Money";

        g_pSysInterface->RunMain(static_cast<char*>(sStartLine));

        FreeGlacierSystem();
        return 0;
    }

    int Glacier_RunNormal(int hInstance, int hPrevInstance, const char* psCmdLine)
    {
        g_sProjectID = "hitman3";
        return Glacier_Main(hInstance, hPrevInstance, psCmdLine);
    }
}
