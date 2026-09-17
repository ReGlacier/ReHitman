#include <Windows.h>
#include <Glacier/Startup/GlacierStartup.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    using namespace Glacier;

    if (!IsDebuggerPresent())
    {
        return Glacier_RunNormal(reinterpret_cast<intptr_t>(hInstance), nShowCmd, lpCmdLine);
    }

    g_sProjectID = "hitman3";
    return Glacier_Main(reinterpret_cast<intptr_t>(hInstance), nShowCmd, lpCmdLine);
}